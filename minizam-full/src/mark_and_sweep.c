#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "domain_state.h"
#include "config.h"
#include "interp.h"
#include "freelist.h"
#include "list.h"

#define Debug(x) //(x)

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
  mlvalue v = Val_ptr(block+1);
  size_t z = Size(v);
  //for (int i = 0;i < z;i++) { Field(v,i) = 0; }
  freelist_t * fl = select_freelist(z);
  cons_fl(v,&fl);
  //insert_fl(v,fl);
  
  //print_fl(fl);
}

// recherche d'un emplacement assez grand dans une freelist, 
// suivant une strategie (eg. first_fit)
mlvalue *fl_find (mlvalue * strategie (size_t sz,freelist_t * fl), 
                 size_t sz){
  freelist_t * fl = select_freelist(sz);
  return strategie(sz-1,fl);
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
    //*ptr = Make_header(rest / sizeof(mlvalue),0,0);
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

mlvalue * small_alloc(size_t sz){
  mlvalue *b;
  b = fl_find(first_fit,sz / sizeof(mlvalue));  // pourquoi +1, -1 ?
  //if (b) printf ("~~~~> %d %d\n",sz / sizeof(mlvalue), Size_hd(*b));
  if (b == NilFL){
    b = Alloc(sz);
    Cons(b,objects);
    //print_list(objects);
    //p[0] = Make_header(0,WHITE,0);
    return b;
  }
  return b;
}

// pour gros objets
static void small_sweep(){
  list cur, c;
  mlvalue * b;
  if (objects != Empty){
    b = objects->content;
    if (Color_hd(*b) == WHITE){ 
      c = objects;
      objects = objects->next;
      recycle(b);
      free(c);
      if (!objects){ return; }
    } else { 
      Set_color_hd(*b,WHITE); 
    }
    cur = objects;
    while (cur->next != Empty){
      b = cur->next->content;
      if (Color_hd(*b) == WHITE){
        c = cur->next;
        cur->next = cur->next->next;
        recycle(b);
        free(c);  
      } else {
        Set_color_hd(*b,WHITE);
        cur = cur->next;
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
    b = big_list->content;
    if (Color_hd(*b) == WHITE){ 
      Cdr(big_list);
      //big_list = big_list->next;
      if (!big_list){ return; }
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
  mark_aux(Val_ptr(CamlLocal+1));
}

void sweep (){
  Debug( printf ("[debut : %d gros objets alloués\n",length(big_list)) );
  Debug( printf ("[debut : %d objets alloués\n",length(objects)) );
  big_sweep();
  small_sweep();
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
  return small_alloc(sz);
}
