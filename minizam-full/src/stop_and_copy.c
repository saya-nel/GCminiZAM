#include "stop_and_copy.h"

mlvalue *stop_and_copy_alloc(size_t n)
{
  // if ((Caml_state->heap_pointer + n) >= (Caml_state->from_space + (SEMI_SPACE_SIZE / 8)))
  //   printf("Plus de place dans le tas\n");

  mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
  Caml_state->heap_pointer = res + n;      // on alloue n octet
  return res;
}