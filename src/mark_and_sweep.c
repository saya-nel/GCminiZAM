#include <stdio.h>

#include "mark_and_sweep.h"
#include "interp.h"
#include "list.h"


//#define DEBUG

/* MARK =================================================== */

#define NOT_MARKED(x) (x && Is_block(x) && (Color(x) != BLACK))
#define MARK(v,color) Hd_val(v) = Make_header(Size(v),color,Tag(v));

// Nombre de mots accessibles, calculé à chaque marquage.
static size_t reachable_words = 0 ;

// marquage naïf, parcours récursivement les blocs accessibles à partir d'une racine
// risque de provoquer un débordement de pile.
void recurvive_mark(mlvalue racine){
  size_t i;
  if (NOT_MARKED(racine)){
    MARK(racine,BLACK);
    reachable_words += Size(racine) + 1;
    for (i = 0; i < Size(racine);i++){
      recurvive_mark (Field(racine,i));
    }
  }
}

// marquage itératif (avec pile explicite)

// MANAGE_SIZE réalloue le pointeur stk avec une taille de n, si nécessaire.
// ne déplace pas le pointeur si n est plus petit que la taille du bloc pointé.
// si stk est null, alors il est alloué avec une taille de (1024 * sizeof(mlvalue));
#define MANAGE_SIZE(stk,n) do {				\
    static size_t MANAGE_SIZE = 1024;                   \
    if (!stk){                                          \
      stk = malloc(MANAGE_SIZE * sizeof(mlvalue));      \
    } else if (MANAGE_SIZE < n){                        \
      MANAGE_SIZE = n;                                  \
      stk = realloc(stk,MANAGE_SIZE * sizeof(mlvalue)); \
    }                                                   \
  } while (0);                                          \

void iterative_mark(mlvalue racine){
  size_t i;
  long long int t; // t doit être un entier signé.
  mlvalue x;
  static mlvalue * stk = NULL;
  MANAGE_SIZE(stk,reachable_words);

  if (NOT_MARKED(racine)){
    MARK(racine,BLACK);
    reachable_words += Size(racine) + 1;
    t = 0;
    stk[0] = racine;
    while (t >= 0){
      x = stk[t];
      --t;
      for (i = 0; i < Size(x);i++){
	if (NOT_MARKED(Field(x,i))){
	  MARK(Field(x,i),BLACK);
	  reachable_words += Size(Field(x,i)) + 1;
	  t++;
	  stk[t] = Field(x,i);
	}
      }
    }
  }
}

// les racines sont la piles, accu, env
// à quoi s'ajoute un pointeur éventuel CamlLocal
// (CamlLocal est une variable temporaire utilisée lors de l'allocation, voir mlvalues.h)
void mark(){ 
  reachable_words = 0;
  size_t i;
  for (i = 0; i < sp; i++){
    iterative_mark(Caml_state->stack[i]); 
  }
  iterative_mark(accu);
  iterative_mark(env);
  if (CamlLocal){
    iterative_mark(Val_ptr(CamlLocal+1));
  }
}

/* Allocation =================================================== */

// SelectFreelist(i,sz) place dans i l'indice de la freelist associé à la taille sz.
// les freelist 0 à 14 sont réservées aux blocs de 1 à 15 mots.
// les freelist suivantes sont organisées en puissance de 2.
#define SelectFreelist(i,sz) do {       \
    if (sz <= 15){                      \
      i = sz - 1; break;                \
    }                                   \
    int SelectFreelist = sz;            \
    i = (15-4);                         \
    while (SelectFreelist >>= 1) ++i;		\
  } while (0);

// Insertion d'un block dans la bonne freelist
// Le header du block, doit être block[0]
// et il doit être bien formé (sa taille sera consultée)
void recycle (mlvalue * block){
  mlvalue v = Val_ptr(block+1);
  size_t sz = Size(v);
  int i;
  SelectFreelist(i,sz);
  freelist_t fl = Caml_state->freelist_array[i];
  cons_fl(v,&fl);
  //insert_freelist(compare_addr,v,&fl);
  Caml_state->freelist_array[i] = fl;
}
// recherche d'un emplacement assez grand dans une freelist, 
// suivant une strategie (eg. first_fit)
mlvalue *fl_find (mlvalue * strategie (size_t sz,freelist_t * fl), 
                  size_t sz){
  int i;
  SelectFreelist(i,sz);
  freelist_t fl = Caml_state->freelist_array[i];
  mlvalue * b = strategie(sz,&fl);
  Caml_state->freelist_array[i] = fl;
  return b;
}

/* allocateur de petits objets */

// Allocation d'un pointeur sur sz octets dans un espace paginé.
// Si il ne reste pas suffisement de place dans la page courrante pour stocker sz octets,
// alors le bloc restant est ajouté à la bonne freelist, une nouvelle page est créée,
// puis un pointeur sur les sz premiers octets de la nouvelle page est alloué et retourné.
mlvalue * paged_alloc (size_t sz){
  static mlvalue * ptr = 0; // pointeur vers le dernier bloc alloué
  static size_t rest = 0;  // nombre de mlvalue restance jusqu'à la fin de la page
  mlvalue * b = NULL;
  if (!ptr){
  CreatePage:
    b = malloc(PAGE_SIZE);
    Cons(b,Caml_state->pages);
    rest = PAGE_SIZE;
    rest -= sz;
    ptr = b + (sz / sizeof(mlvalue));
    return b;
  }
  else if (sz <= rest){
    b = ptr;
    rest -= sz;
    ptr = b + (sz / sizeof(mlvalue));
    return b;
  } else if (rest / sizeof(mlvalue) >= 2){
    *ptr = Make_header((rest / sizeof(mlvalue)) - 1,0,0);
    recycle(ptr);
    goto CreatePage;
  } else {
    goto CreatePage;
  }
}

// allocation d'un "petits objets" de sz octets
// l'objet est extrait de la bonne freelist, si non vide.
// sinon, il est alloué dans l'espace paginé.
mlvalue * small_alloc(size_t sz){
  mlvalue *b;

  b = fl_find(first_fit,(sz) / sizeof(mlvalue));
  if (b == NilFL){
    b = paged_alloc(sz);
    Cons(b,Caml_state->objects);
    return b;
  } 
  return b;
}

/* allocation de gros objets */

static mlvalue * big_alloc(size_t sz){
  mlvalue *p = malloc(sz);
  Cons(p,Caml_state->big_objects);
  return p;
}

/* SWEEP =================================================== */

#define Big_objs (Caml_state->big_objects)

/* phase de sweep pour les gros objets */
static void big_sweep(){
  list cur;
  mlvalue * b;
  if (PAIR(Big_objs)){
    b = CAR(Big_objs);
    if (Color_hd(*b) == WHITE){ 
      // si le bloc n'est pas marqué, on le libère avec free et on passe à la suite
      FreeCar(Big_objs);
      if (!PAIR(Big_objs)){ 
        return; 
      }
    } else { 
      MARK(Val_ptr(b+1),WHITE); 
    }
    cur = Big_objs;
    while (PAIR(CDR(cur))){
      b = CADR(cur);
      if (Color(Val_ptr(b+1)) == WHITE){
        FreeCadr(cur);
      } else {
        MARK(Val_ptr(b+1),WHITE);
        cur = CDR(cur);
      }
    }
  }
}

#define Objs (Caml_state->objects)

/* phase de sweep pour les petits objets */
static void small_sweep(){
  list cur, c;
  mlvalue * b;
  if (Objs != Empty){
    b = CAR(Objs);
    if (Color_hd(*b) == WHITE){ 
      // si le bloc n'est pas marqué, le libère, on l'ajoute à une freelist, 
      // puis on passe à la suite
      c = Objs;
      Objs = CDR(Objs);
      recycle(b);
      free(c);
      if (!PAIR(Objs)){ 
        return;
      }
    } else { 
      MARK(Val_ptr(b+1),WHITE);
    }
    cur = Objs;
    while (PAIR(CDR(cur))){
      b = CADR(cur);
      if (Color_hd(*b) == WHITE){
        c = CDR(cur);
        RPLACD(cur,CDDR(cur));
        recycle(b);
        free(c);  
      } else {
        MARK(Val_ptr(b+1),WHITE);
        cur = CDR(cur);
      }
    }
  }
}

#ifdef DEBUG
void stat(){  
  printf ("\t%d large objects allocated\n",length(Big_objs));
  printf ("\t%d small objects allocated\n",length(Objs));
}
#endif

void sweep (){
  big_sweep();
  small_sweep();
}

/* MARK&SWEEP =================================================== */

void mark_and_sweep(){
#ifdef DEBUG 
  printf("before gc :\n"); stat();
#endif
  
  mark();

#ifdef DEBUG 
  printf("reachable words : %zu\n",reachable_words);
#endif

  sweep();

#ifdef DEBUG 
  printf("after gc :\n"); stat(); printf("====================\n");
#endif
}

#define MaxW (MAX_HEAP_SIZE / sizeof(mlvalue))

/* allocation avec appel au gc 
 * dès que l'occupation mémoire à augmenter de 50 %
 * par rapport au précédent GC, 
 * ou si le nombre de mots aloués depuis le précedent gc excede MaxW */ 

mlvalue *mark_and_sweep_alloc(size_t sz){
  static size_t cur_sz = 0;
  if (cur_sz > (reachable_words * 3) / 2 || cur_sz >= MaxW){
    
    mark_and_sweep();

    if (reachable_words >= MaxW){
      fprintf(stderr, "out of memory\n");
      exit(EXIT_FAILURE);
    }
    cur_sz = 0;
  }
  cur_sz+=sz;
  if (sz >= BIG_OBJECT_MIN_SIZE){
    return big_alloc(sz);
  }
  return small_alloc(sz);
}
