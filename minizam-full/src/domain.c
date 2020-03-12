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
  Caml_state->from_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->to_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->heap_pointer = Caml_state->from_space;
#endif

#ifdef MARK_AND_SWEEP
  // init mark and sweep ici
#endif
}
