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
  Caml_state->from_space = malloc(SEMI_SPACE_SIZE * 2);
  Caml_state->to_space = Caml_state->from_space + SEMI_SPACE_SIZE / sizeof(mlvalue);
  Caml_state->heap_pointer = Caml_state->from_space;
#endif

#ifdef MARK_AND_SWEEP
  // init mark and sweep ici
#endif
}
