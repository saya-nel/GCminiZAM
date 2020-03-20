#include "stop_and_copy.h"

enum Tag_enum
{
  FWD_PTR_T
};

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

  mlvalue *curr = Caml_state->from_space + 1; // +1 pour skipper le premier header
  while (curr < Caml_state->heap_pointer)
  {
    // print_val(Val_ptr(curr));
    // printf("\n");
    printf("tag : %lu | ", Tag(Val_ptr(curr)));
    curr += Size(Val_ptr(curr)) + 1; // +1 pour skipper le header du bloc suivant
  }
  printf("\n");
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
    exit(0);
    return NULL;
  }
}