#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"
#include "interp.h"

#define Debug(x)// (x)

#define Set_color_hd(hd,color) hd = Make_header(Size_hd(hd),color,Tag_hd(hd));

#define Set_color(v,color) Hd_val(v) = Make_header(Size(v),color,Tag(v));

// listes d'objets alloués
struct cell {
  mlvalue * content;
  struct cell * next;    
};
typedef struct cell * list;

#define Empty (0)

/* list c */
#define Free_cell(c) do {          \
  free(c->content);                \
  free(c);                         \
  } while (0);

/* (mlvalue * x, list * l) */
#define Cons(x,l) do {             \
  list c = malloc(sizeof(list));   \
  assert (c);                      \
  c->content = (mlvalue *) x;      \
  c->next = * ((list *)l);         \
  *l = c;                          \
  } while (0);                     

#define Cdr(l) do {                  \
  list c = *l;                       \
  *l = (*l)->next;                   \
  Free_cell(c);                      \
  } while (0);

#define RemoveCadr(l) do {           \
  list c = (*l)->next;               \
  (*l)->next = (*l)->next->next;     \
  Free_cell(c);                      \
  } while (0);


int length (list l){
  int n = 0;
  while (l != Empty){ ++n; l = l->next; }
  return n;
}

//liste de gros objets alloués
static list big_objects = Empty;

// allouer un gros objet
mlvalue * alloc_big_object(size_t sz){
  mlvalue *p = malloc(sz*8);
  //printf ("---->%d\n",(length (big_objects)));
  Cons(p,&big_objects);
  return p;
}

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

// pour gros objets
void sweep_free(list * lst){
  list cur;
  mlvalue * b;
  if (*lst != Empty){
    b = (*lst)->content;
    if (Color_hd(*b) == WHITE){
      Cdr(lst);
    } else { 
      Set_color_hd(*b,WHITE); 
    }
    cur = *lst;
    while (cur->next != Empty){
      b = cur->next->content;
      if (Color_hd(*b) == WHITE){
        RemoveCadr(&cur);
      } else {
        Set_color_hd(*b,WHITE);
        cur = cur->next;
      }
    }
  }
}

void sweep (){
  Debug( printf ("[debut : %d objets alloués\n",length(big_objects)) );
  sweep_free(&big_objects);
  Debug( printf ("fin : %d objets alloués]\n",length(big_objects)) );
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
  return alloc_big_object(sz); // return malloc (8*sz);
}
