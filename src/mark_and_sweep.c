#include "mark_and_sweep.h"
#include <assert.h>
#include <stdio.h>

#include "interp.h"

#include "list.h"

#define Debug(x) //(x)

#define Set_color_hd(hd,color) hd = Make_header(Size_hd(hd),color,Tag_hd(hd));

#define Set_color(v,color) Hd_val(v) = Make_header(Size(v),color,Tag(v));

/* ******************************* */


// insertion d'un block dans la bonne freelist
// le header du block, doit être block[0] et il doit être bien formé (on consulte la taille)
void recycle (mlvalue * block){
  mlvalue v = Val_ptr(block+1);
  size_t z = Size(v);
  //for (int i = 0;i < z;i++) { Field(v,i) = 0; }
  freelist_t * fl = SelectFreelist(z);
  insert_fl(v,fl);
  //print_fl(fl);
}

// recherche d'un emplacement assez grand dans une freelist, 
// suivant une strategie (eg. first_fit)
mlvalue *fl_find (mlvalue * strategie (size_t sz,freelist_t * fl), 
                  size_t sz){
  freelist_t * fl = SelectFreelist(sz);
  return strategie(sz-1,fl);
}

/* ****************************** */

mlvalue * paged_alloc (size_t sz){
  static mlvalue * ptr = 0; // pointeur vers le dernier bloc alloué
  static size_t rest = 0;  // nombre de mlvalue restance jusqu'à la fin de la page
  mlvalue * b;
  if (!ptr){
    CreatePage:
    b = malloc(PAGE_SIZE);
    Cons(b,Caml_state->pages);
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
  list_delete(&Caml_state->pages);
  printf("pages correctement libérés.\n");
}

//atexit(delete_pages);

/* allocateur de petits objets */
#define Paged_alloc paged_alloc

/* ****************************** */

mlvalue * small_alloc(size_t sz){
  mlvalue *b;
  b = fl_find(first_fit,sz / sizeof(mlvalue));  // -1 ?
  if (b == NilFL){
    b = Paged_alloc(sz);
    Cons(b,Caml_state->objects);
    return b;
  }
  return b;
}

// pour gros objets
static void small_sweep(){
  list cur, c;
  mlvalue * b;
  if (Caml_state->objects != Empty){
    b = Caml_state->objects->content;
    if (Color_hd(*b) == WHITE){ 
      c = Caml_state->objects;
      Caml_state->objects = Caml_state->objects->next;
      recycle(b);
      free(c);
      if (!Caml_state->objects){ return; }
    } else { 
      Set_color_hd(*b,WHITE); 
    }
    cur = Caml_state->objects;
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

// allouer un gros objet
static mlvalue * big_alloc(size_t sz){
  mlvalue *p = malloc(sz);
  Cons(p,Caml_state->big_objects);
  return p;
}

// pour gros objets
static void big_sweep(){
  list cur;
  mlvalue * b;
  if (Caml_state->big_objects != Empty){
    b = Caml_state->big_objects->content;
    if (Color_hd(*b) == WHITE){ 
      FreeCar(Caml_state->big_objects);
      if (!Caml_state->big_objects){ return; }
    } else { 
      Set_color_hd(*b,WHITE); 
    }
    cur = Caml_state->big_objects;
    while (cur->next != Empty){
      b = cur->next->content;
      if (Color_hd(*b) == WHITE){
        FreeCadr(cur);
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
  Debug( printf ("[debut : %d gros objets alloués\n",length(Caml_state->big_objects)) );
  Debug( printf ("[debut : %d objets alloués\n",length(Caml_state->objects)) );
  big_sweep();
  small_sweep();
  Debug( printf ("fin : %d gros objets alloués]\n",length(Caml_state->big_objects)) );
  Debug( printf ("fin : %d objets alloués]\n",length(Caml_state->objects)) );
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
