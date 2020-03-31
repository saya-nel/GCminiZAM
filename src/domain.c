#include <stdlib.h>

#include "domain_state.h"
#include "config.h"
#include "mlvalues.h"
#include "stdio.h"

caml_domain_state *Caml_state;

void caml_init_domain()
{
  Caml_state = malloc(sizeof(caml_domain_state));
  Caml_state->stack = malloc(Stack_size);

#ifdef STOP_AND_COPY
  SEMI_SPACE_SIZE = 32 * KB;
  Caml_state->from_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->to_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->heap_pointer = Caml_state->from_space;
#endif
}

void free_domain()
{
  free(Caml_state->stack);
#ifdef STOP_AND_COPY
  free(Caml_state->from_space);
  free(Caml_state->to_space);
  free(Caml_state->heap_pointer);
#endif
  free(Caml_state);
}
