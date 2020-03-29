#include "stop_and_copy.h"

#ifndef DEBUG
#define DEBUG
#endif

mlvalue *next;

/*
  Nettoie le tas passer en paramètre
*/
void clear_heap(mlvalue *too_clear)
{
  for (int i = 0; i < (int)(SEMI_SPACE_SIZE / sizeof(mlvalue)); i++)
    too_clear[i] = 0;
}

/*
  Return 1 si le tas peut alloué n octets, 0 sinon
*/
int heap_can_alloc(size_t n)
{
  return (Caml_state->heap_pointer + n) < (Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
}

short ptr_to_from_space()
{
  mlvalue *from_space_start = Caml_state->from_space;
  mlvalue *from_space_end = Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue));

  // on verifie que aucun element de la stack ne pointe vers from_space
  mlvalue *curr = Caml_state->stack;
  while (curr < Caml_state->stack)
  {
    if (Is_block(*curr))
      if (Ptr_val(*curr) >= from_space_start && Ptr_val(*curr) < from_space_end)
      {
        printf("aie pile : curr : %ld, from_space_start : %ld, from_space_end : %ld\n", Ptr_val(*curr), from_space_start, from_space_end);
        return 1;
      }
    curr++;
  }
  // parcours to_space
  curr = Caml_state->to_space + 1;
  while (curr < next && curr < Caml_state->to_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)))
  {
    for (int i = 0; i < Size(Val_ptr(curr)); i++)
    {
      if (Is_block(curr[i]))
        if (Ptr_val(curr[i]) >= from_space_start && Ptr_val(curr[i]) < from_space_end)
        {
          printf("aie tas : curr : %ld, from_space_start : %ld, from_space_end : %ld\n", Ptr_val(curr[i]), from_space_start, from_space_end);
          return 1;
        }
    }
    curr += Size(Val_ptr(curr)) + 1;
  }

  return 0;
}

void print_stack()
{
  mlvalue *curr = Caml_state->stack;
  printf("\n========= STACK ==============\n");
  while (curr < &Caml_state->stack[sp])
  {
    if (Is_block(*curr))
      printf("b %ld | ", *curr);
    else
      printf("v %ld | ", *curr);
    curr++;
  }
  printf("\n======== FIN STACK ===============\n\n");
}

print_from_space()
{
  mlvalue *curr = Caml_state->from_space;
  printf("\n========= FROM_SPACE ==============\n");
  while (curr < Caml_state->heap_pointer)
  {
    if (Is_block(*curr))
      printf("b %ld | ", *curr);
    else
      printf("v %ld | ", *curr);
    curr++;
  }
  printf("\n========= FIN FROM_SPACE ==============\n\n");
}

print_to_space()
{
  mlvalue *curr = Caml_state->to_space;
  printf("\n========= TO_SPACE ==============\n");
  while (curr < next)
  {
    if (Is_block(*curr))
      printf("b %ld | ", *curr);
    else
      printf("v %ld | ", *curr);
    curr++;
  }
  printf("\n========= FIN TO_SPACE ==============\n\n");
}

void move_addr(mlvalue *val, char *zone)
{
  if (Is_block(*val)) // val pointe vers un block
  {

#ifdef DEBUG
    printf("parcours %s. pointeur vers addr : %ld, tag : %ld, size %ld\n", zone, *val, Tag(*val), Size(*val));
#endif

    if (Tag(*val) == FWD_PTR_T) // si le block est un fwd ptr
      *val = Field0(*val);      // on fais pointer val vers le fwd ptr
    else
    {
      // on copie tout le bloc (header comprit) dans to_space
      mlvalue *old = next; // sauvegarde de l'endroit ou on va copier dans to_space
      memcpy(next, &(Hd_val(*val)), (Size(*val) + 1) * sizeof(mlvalue));

#ifdef DEBUG
      printf("copie dans to_space :\n");
      for (int i = 0; i < (Size(*val) + 1); i++)
      {
        printf("%ld | ", (&(Hd_val(*val)))[i]);
      }
      printf("\n");
#endif

      next += Size(*val) + 1; // prochaine position disponible dans to_space
      // on change son tag en FWD_PTR_T
      Hd_val(*val) = Make_header(Size(*val), Color(*val), FWD_PTR_T);
      // ajoute le forward pointer vers la nouvelle position dans to_space
      Field0(*val) = Val_ptr(old + 1); // on pointe sur le premier champ
      // on pointe vers le nouvel objet dans to_space
      *val = Val_ptr(old + 1); // on pointe sur le premier champ
    }
  }
#ifdef DEBUG
  else
    printf("parcours %s : not block\n", zone);
#endif
}

void run_gc()
{

  // pour parcourir la pile
  mlvalue *curr = Caml_state->stack; // premier element de la pile
  next = Caml_state->to_space;       // premiere pos qu'on peut allouer dans to_space

#ifdef DEBUG
  printf("\ndebut parcours stack\nstack initiale : (b = pointeur , v = valeur)\n");
  print_stack();
#endif

  while (curr < &(Caml_state->stack[sp])) // on parcours toute la pile
  {
    move_addr(curr, "stack");
    curr++; // on va sur l'élément suivant dans la pile
#ifdef DEBUG
    print_stack();
#endif
  }

#ifdef DEBUG
  printf("fin parcours stack\n\n");
#endif

  // on parcours l'accu
  move_addr(&accu, "accu");
  // on parcours l'env
  move_addr(&env, "env");

#ifdef DEBUG
  printf("après parcours, accu : %ld, env : %ld\n\n", accu, env);
  printf("debut parcours to_space\nto_space initial : b = pointeur , v = valeur (valeur ou header)\n");
  print_to_space();
#endif

  mlvalue *scan = Caml_state->to_space + 1; // scan le tas, on ce place apres le premier header
  // on parcours le tas jusqu'a la premier position non allouée (next)
  while (scan < next)
  {
#ifdef DEBUG
    printf("to_space tag : %ld, size : %ld\n", Tag(Val_ptr(scan)), Size(Val_ptr(scan)));
#endif

    for (int i = 0; i < Size(Val_ptr(scan)); i++) // on parcours l'objet
    {
#ifdef DEBUG
      printf("to_space infos : addr elem courant : %lu, addr dernier elem : %lu, addr fin to_space %lu\n", scan, next, Caml_state->to_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
#endif

      move_addr(&scan[i], "suivi pointeur depuis to_space");

#ifdef DEBUG
      print_to_space();
#endif
    }

    scan += Size(Val_ptr(scan)) + 1; // on passe a l'objet suivant et saute son header

  } // fin while

#ifdef DEBUG
  printf("VERIF si il reste des pointeur stack -> from_space ou to_space -> from_space, 0 non, 1 oui. : %d\n", ptr_to_from_space());
#endif

  // on a finit, on echange from_space et to_space
  mlvalue *tmp = Caml_state->from_space;
  Caml_state->from_space = Caml_state->to_space;
  Caml_state->to_space = tmp;
  Caml_state->heap_pointer = next;
  clear_heap(Caml_state->to_space);
}

mlvalue *stop_and_copy_alloc(size_t n)
{
  int nb_mlvalue = n / sizeof(mlvalue);
  if (!heap_can_alloc(nb_mlvalue))
  {
#ifdef DEBUG
    printf("Plus de place dans from_space, lancement GC.\n");
    printf("from_space : debut : %ld , fin : %ld / to_space : debut : %ld, fin : %ld\n", Caml_state->from_space, Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)), Caml_state->to_space, Caml_state->to_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
#endif

    run_gc();

#ifdef DEBUG
    printf("fin gc\n");
#endif
  }
  mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
  Caml_state->heap_pointer += nb_mlvalue;  // on alloue n octet
  return res;
}