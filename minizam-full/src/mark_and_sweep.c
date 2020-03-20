#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"

#include "interp.h"

#include "freelist.h"


#define Debug(x) //(x)

// listes d'objets alloués (pour amorcer les freelist)
struct cell {
  mlvalue * content;
  struct cell * next;    
};
typedef struct cell * list;

#define Empty (0)

void cons (mlvalue * x, list * l){
  list c = malloc(sizeof (list));
  assert (c);
  c->content = x;
  c->next = *l;
  *l = c;
}

int length (list l){
  int n = 0;
  while (l != Empty){
    ++n;
    l = l->next; 
  }
  return n;
}

//liste de gros objets alloués
static list big_objects = Empty;

// freelist de gros objets
static mlvalue fl_big_objects = NilFL;

// allouer un gros objet
mlvalue * alloc_big_object(size_t sz){
  mlvalue b,*p = NilFL;
  b = first_fit(sz,&fl_big_objects); 
  if (b == NilFL){
    p = malloc(8*sz);
    cons (p,&big_objects);
    return p;
  }
  return Ptr_val(b);
}

static void mark_aux(mlvalue racine){
  size_t i;
  if (Is_block(racine)){   
    if (Color(racine) == WHITE){ // si pas vu
      Hd_val(racine) = Make_header(Size(racine),GRAY,Tag(racine)); // color[racine] <- vu
      for (i = 0; i < Size(racine);i++){
	mark_aux (Field(racine,i));          
      }
    }
  }
}
void mark(){ 
  // racines dans la pile
  // TODO : racine dans les registres
  size_t i;
  for (i = 0; i < sp; i++){
    mark_aux(Caml_state->stack[i]);
  }
}

// pour gros objets
void sweep(list * lst){
  list cur = *lst;
  mlvalue * b;
  if (cur != Empty){
    for ( ;cur->next != Empty ; cur = cur->next){
      b = cur->next->content;
      //printf("sz = %lld, color = %d, tag = %d\n",Size(Val_ptr(b+1)),Color(Val_ptr(b+1)),Tag(Val_ptr(b+1)));
      if (Color(Val_ptr(b+1)) == GRAY){  
	cons_fl(b,&fl_big_objects); // ajoute b à la freelist 
	cur->next = cur->next->next; // retire b de la liste de gros objets alloués
	assert(cur);
      }
    }
  }
}

mlvalue *mark_and_sweep_alloc(size_t sz){
  static size_t sz_last_gcX3 = 0; // 3 fois la quantité de mémoire utilisé lors du dernier gc
  static size_t cur_sz = 0;

  if (2 * cur_sz > sz_last_gcX3){  // si augmentation de 50 %
    /* effectivement la stratégie demdandée augmente 
       les performances en pratique */
    sz_last_gcX3 = cur_sz * 3;
        
    Debug( printf ("debut : %d objets alloués\n",length(big_objects)) );
        
    //mark();
    sweep(&big_objects);

    Debug( printf ("fin : %d objets alloués\n",length(big_objects)) );
  }
  cur_sz+=sz;
  return alloc_big_object(sz); //malloc(sz*8);
}
