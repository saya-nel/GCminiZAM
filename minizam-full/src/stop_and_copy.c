#include "stop_and_copy.h"

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

void search_alive_values_on_stack()
{

  // mlvalue *curr = Caml_state->from_space + 1; // +1 pour skipper le premier header
  // while (curr < Caml_state->heap_pointer)
  // {
  //   // print_val(Val_ptr(curr));
  //   // printf("\n");
  //   printf("tag : %lu, size : %lu | ", Tag(Val_ptr(curr)), Size(Val_ptr(curr)));
  //   curr += Size(Val_ptr(curr)) + 1; // +1 pour skipper le header du bloc suivant
  // }
  // printf("\n");

  // pour parcourir la pile
  mlvalue *curr = Caml_state->stack; // premier element de la pile

  mlvalue *next = Caml_state->to_space; // premiere pos qu'on peut allouer dans to_space

  while (curr < &Caml_state->stack[sp]) // on parcours toute la pile
  {
    if (Is_block(*curr)) // si on est sur un pointeur dans la pile
    {
      // on regarde si le tag est FWD_PTR_T
      if (Tag(*curr) == FWD_PTR_T)
      {
        // on pointe vers la valeur du forwarding pointer
        *curr = Field0(*curr);
      }
      else
      {
        // on créer une copie dans to_space du bloc courant dans to space (header compris)
        memcpy(next, &(Ptr_val(*curr)[-1]), (Size(*curr) + 1) * sizeof(mlvalue));
        mlvalue *old = next;     // sauvegarde de l'endroit ou on a copier dans to_space
        next += Size(*curr) + 1; // prochaine position disponible dans to_space
        // on change son tag dans from_space en FWD_PTR_T
        Hd_val(*curr) = Make_header(Size(*curr), WHITE, FWD_PTR_T);
        // ajoute le forward pointer dans from_space vers la nouvelle position dans to_space
        Field0(*curr) = Val_ptr(old);
        // on pointe (la pile) vers le nouvel objet dans to_space
        *curr = Val_ptr(old);
        // printf("old : %d, val_ptr(hold) %lu\n", old, Val_ptr(old));
      }
    }
    curr += 1; // on va sur l'élément suivant dans la pile
  }
  // on a finit, on echange from_space et to_space
  mlvalue *tmp = Caml_state->from_space;
  Caml_state->from_space = Caml_state->to_space;
  Caml_state->to_space = tmp;
  Caml_state->heap_pointer = next;
}

mlvalue *stop_and_copy_alloc(size_t n)
{
  n = n / sizeof(mlvalue);
  if (heap_can_alloc(n))
  {
    mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
    Caml_state->heap_pointer += n;           // on alloue n octet
    return res;
  }
  else
  {
    printf("Plus de place dans from_space, lancement GC.\n");
    search_alive_values_on_stack();
    return stop_and_copy_alloc(n * sizeof(mlvalue));
  }
}