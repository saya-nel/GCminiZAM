#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"
#include "interp.h"
#include "freelist.h"
#include "list.h"

#define Debug(x) (x)

#define Set_color_hd(hd,color) hd = Make_header(Size_hd(hd),color,Tag_hd(hd));

#define Set_color(v,color) Hd_val(v) = Make_header(Size(v),color,Tag(v));

/* ******************************* */
// nombre de freelist
#define NB_FREELIST 3

// les freelist du programme
static freelist_t freelist[NB_FREELIST] = {NilFL,NilFL,NilFL};

// liste des "petits" objets alloués
static list objects = Empty;

// rend un pointeur sur la freelist cible d'un bloc de taille sz 
freelist_t * select_freelist(size_t sz){ 
  if (sz <= 64){
    return &freelist[0];
  }
  if (sz <= 256){
    return &freelist[1];
  }
  return &freelist[2];
}

// insertion d'un block dans la bonne freelist
// le header du block, doit être block[0] et il doit être bien formé (on consulte la taille)
void recycle (mlvalue * block){
  size_t z = Size_hd(*block);
  freelist_t * fl = select_freelist(z);
  cons_fl(block,fl);
}

// recherche d'un emplacement assez grand dans une freelist, 
// suivant une strategie (eg. first_fit)
mlvalue fl_find (mlvalue strategie (size_t sz,freelist_t * fl), 
                 size_t sz){
  freelist_t * fl = select_freelist(sz);
  return strategie(sz,fl);
}

/* ****************************** */

list pages = Empty;

mlvalue * palloc (size_t sz){
  static mlvalue * ptr = 0; // pointeur vers le dernier bloc alloué
  static size_t rest = 0;  // nombre de mlvalue restance jusqu'à la fin de la page
  mlvalue * b;
  if (!ptr){
    CreatePage:
    b = malloc(PAGE_SIZE);
    Cons(b,pages);
    rest = PAGE_SIZE / sizeof(mlvalue);
    rest -= sz;
    ptr = b + sz;
    return b;
  }
  else if (sz <= rest){
    b = ptr;
    rest -= sz;
    ptr = b + sz;
    return b;
  } else {
    *ptr = Make_header(rest / sizeof(mlvalue),0,0);
    recycle(ptr);
    goto CreatePage;
  }
}

void delete_pages(){
  list_delete(&pages);
  printf("pages correctement libérés.\n");
}
//atexit(delete_pages);


/* allocateur de petits objets */
#define Alloc palloc // ou malloc

/* ****************************** */

mlvalue * alloc(size_t sz,list * objects){
  mlvalue b,*p = 0;
  b = fl_find(first_fit,sz); 
  if (b == NilFL){
    p = Alloc(sz);
    Cons(p,*objects);
    return p;
  }
  return Ptr_val(b);
}

void fl_sweep(list * lst){
  list cur = *lst;
  mlvalue * b;
  if (cur != Empty){
    for ( ;cur->next != Empty ; cur = cur->next){
      b = cur->next->content;
      //printf("sz = %lld, color = %d, tag = %d\n",Size(Val_ptr(b+1)),Color(Val_ptr(b+1)),Tag(Val_ptr(b+1)));
      if (Color_hd(*b) == WHITE){  
        recycle(b); // ajoute b à la freelist 
        cur->next = cur->next->next; // retire b de la liste de gros objets alloués
        assert(cur);
      } else {
	Set_color_hd(*b,WHITE);
      }
    }
  }
}

/* ****************************************************** */

//liste de gros objets alloués
static list big_list = Empty;

// allouer un gros objet
static mlvalue * big_alloc(size_t sz){
  mlvalue *p = malloc(sz);
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
  Debug( printf ("[debut : %d gros objets alloués\n",length(big_list)) );
  Debug( printf ("[debut : %d objets alloués\n",length(objects)) );
  big_sweep();
  fl_sweep(&objects);
  Debug( printf ("fin : %d gros objets alloués]\n",length(big_list)) );
  Debug( printf ("fin : %d objets alloués]\n",length(objects)) );
}

/* ****************************************************** */

mlvalue *mark_and_sweep_alloc(size_t sz){
  static size_t sz_last = 0; // 3 fois la quantité de mémoire utilisée lors du dernier gc
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
  return alloc(sz,&objects);
}
