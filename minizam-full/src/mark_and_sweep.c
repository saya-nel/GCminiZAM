#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"
#include "interp.h"
#include "freelist.h"

#define Debug(x) //(x)

#define Set_color_hd(hd,color) hd = Make_header(Size_hd(hd),color,Tag_hd(hd));

#define Set_color(v,color) Hd_val(v) = Make_header(Size(v),color,Tag(v));



// listes d'objets alloués
struct cell {
  void * content;
  struct cell * next;    
};
typedef struct cell * list;

#define Empty (0)
#define Next(c) ((c)->next)
/* list c */
#define Free_cell(c) do {			\
    free(c->content);				\
    free(c);					\
  } while (0);

#define Cons(x,l) do {				\
    list Cons = malloc(sizeof(list));		\
    Cons->content = (void *) x;			\
    Cons->next = ((list) l);			\
    l = Cons;					\
  } while (0);                     

#define Cdr(l) do {				\
    list Cdr = l;				\
    l = (l)->next;				\
    Free_cell(Cdr);				\
  } while (0);

#define RemoveCadr(l) do {			\
    list RemoveCadr = (l)->next;		\
    (l)->next = (l)->next->next;		\
    Free_cell(RemoveCadr);			\
  } while (0);


int length (list l){
  int n = 0;
  while (l != Empty){ ++n; l = l->next; }
  return n;
}

/* ******************************* */

//liste de gros objets alloués
static list big_list = Empty;

// allouer un gros objet
static mlvalue * big_alloc(size_t sz){
  mlvalue *p = malloc(sz*8);
  Cons(p,big_list);
  return p;
}

// pour gros objets
static void big_sweep(){
  list cur;
  mlvalue * b;
  if (big_list != Empty){
    b = (big_list)->content;
    if (Color_hd(*b) == WHITE){ 
      Cdr(big_list);
      if (big_list == Empty){
        return;
      }
    } else { 
      Set_color_hd(*b,WHITE); 
    }
    cur = big_list;
    while (cur->next != Empty){
      b = cur->next->content;
      if (Color_hd(*b) == WHITE){
        RemoveCadr(cur);
      } else {
        Set_color_hd(*b,WHITE);
        cur = cur->next;
      }
    }
  }
}


/* ****************************** */

static list obj0 = Empty;
static mlvalue fl0 = NilFL;

static list obj1 = Empty;
static mlvalue fl1 = NilFL;

mlvalue * alloc(size_t sz,list * objects, mlvalue * fl){
  mlvalue b,*p = 0;
  b = first_fit(sz,fl); 
  if (b == NilFL){
    p = malloc(8*sz);
    Cons(p,*objects);
    return p;
  }
  return Ptr_val(b);
}

void fl_sweep(list * lst,mlvalue * fl){
  list cur = *lst;
  mlvalue * b;
  if (cur != Empty){
    for ( ;cur->next != Empty ; cur = cur->next){
      b = cur->next->content;
      //printf("sz = %lld, color = %d, tag = %d\n",Size(Val_ptr(b+1)),Color(Val_ptr(b+1)),Tag(Val_ptr(b+1)));
      if (Color_hd(*b) == WHITE){  
        cons_fl(b,fl); // ajoute b à la freelist 
        cur->next = cur->next->next; // retire b de la liste de gros objets alloués
        assert(cur);
      } else {
	Set_color_hd(*b,WHITE);
      }
    }
  }
}

/* ****************************************************** */

static void mark_aux(mlvalue racine){
  size_t i;
  if (Is_block(racine)){ 
    //printf("sz = %lld, color = %d, tag = %d\n",Size(racine),Color(racine),Tag(racine));  
    if (Color(racine) != BLACK){ // si pas vu
      Set_color(racine,BLACK); // color[racine] <- vu
      for (i = 0; i < Size(racine);i++){
        mark_aux (Field(racine,i));
      }
    }
  }
}

void mark(){ 
  size_t i;
  for (i = 0; i < sp; i++){ 
    mark_aux(Caml_state->stack[i]); 
  }
  mark_aux(accu);
  mark_aux(env);
}

void sweep (){
  Debug( printf ("[debut : %d big objets alloués\n",length(big_list)) );
  Debug( printf ("[debut : %d objets alloués\n",length(obj0)) );
  Debug( printf ("[debut : %d objets alloués\n",length(obj1)) );
  big_sweep();
  fl_sweep(&obj0,&fl0);
  fl_sweep(&obj1,&fl1);
  Debug( printf ("fin : %d big objets alloués]\n",length(big_list)) );
  Debug( printf ("fin : %d objets alloués]\n",length(obj0)) );
  Debug( printf ("fin : %d objets alloués]\n",length(obj1)) );
}

mlvalue *mark_and_sweep_alloc(size_t sz){
  static size_t sz_last = 0; // 3 fois la quantité de mémoire utilisé lors du dernier gc
  static size_t cur_sz = 0;
  if (2 * cur_sz > sz_last * 3){  // si augmentation de 50 %
    // effectivement la stratégie demdandée augmente 
    //  les performances en pratique 
    sz_last = cur_sz;
    mark();
    sweep();
  }
  cur_sz+=sz;
  if (sz >= BIG_OBJECT_MIN_SIZE){
    return big_alloc(sz);
  }
  if  (sz >16){
    return alloc(sz,&obj0,&fl0);
  }
  return alloc(sz,&obj1,&fl1);
}
