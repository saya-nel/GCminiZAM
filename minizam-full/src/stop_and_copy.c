#include "stop_and_copy.h"

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
  return (Caml_state->heap_pointer + n) <= (Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
}

void move_addr(mlvalue *val)
{
  if (Is_block(*val)) // val pointe vers un block
  {
    if (Tag(*val) == FWD_PTR_T) // si le block est un fwd ptr
    {
      *val = Field0(*val); // on fais pointer val vers le fwd ptr
    }
    else
    {
      printf("tag : %ld, size : %ld\n", Tag(*val), Size(*val));
      // on copie tout le bloc (header comprit) dans to_space
      mlvalue *old = next; // sauvegarde de l'endroit ou on va copier dans to_space
      memcpy(next, &(Hd_val(*val)), (Size(*val) + 1) * sizeof(mlvalue));
      next += Size(*val) + 1; // prochaine position disponible dans to_space

      // on change son tag en FWD_PTR_T
      Hd_val(*val) = Make_header(Size(*val), WHITE, FWD_PTR_T);

      // ajoute le forward pointer vers la nouvelle position dans to_space
      Field0(*val) = Val_ptr(old + 1); // on pointe sur le premier champ

      // on pointe vers le nouvel objet dans to_space
      *val = Val_ptr(old + 1); // on pointe sur le premier champ
    }
  }
}

void run_gc()
{
  // pour parcourir la pile
  mlvalue *curr = Caml_state->stack; // premier element de la pile
  next = Caml_state->to_space;       // premiere pos qu'on peut allouer dans to_space
  // clear_heap(next);

  // on parcours l'accu
  move_addr(&accu);
  move_addr(&env);

  while (curr < &(Caml_state->stack[sp])) // on parcours toute la pile
  {
    move_addr(curr);
    curr++; // on va sur l'élément suivant dans la pile
  }

  mlvalue *scan = Caml_state->to_space + 1; // scan le tas, on ce place apres le premier header
  // on parcours le tas jusqu'a la premier position non allouée (next)

  while (scan < next)
  {
    for (int i = 0; i < Size(Val_ptr(scan)); i++) // on parcours l'objet
    {
      move_addr(&scan[i]);
    }
    scan += Size(Val_ptr(scan)) + 1; // on passe a l'objet suivant et saute son header
  }

  // on a finit, on echange from_space et to_space
  mlvalue *tmp = Caml_state->from_space;
  Caml_state->from_space = Caml_state->to_space;
  Caml_state->to_space = tmp;
  Caml_state->heap_pointer = next;
  clear_heap(Caml_state->to_space);
}

mlvalue *stop_and_copy_alloc(size_t n)
{
  n = n / sizeof(mlvalue);
  if (heap_can_alloc(n))
  {
    // printf("alloc ok\n");
    mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
    Caml_state->heap_pointer += n;           // on alloue n octet
    return res;
  }
  else
  {
    printf("Plus de place dans from_space, lancement GC.\n");
    run_gc();
    return stop_and_copy_alloc(n * sizeof(mlvalue));
  }
}