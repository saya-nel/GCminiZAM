#include <stdlib.h>

#include "domain_state.h"
#include "config.h"
#include "mlvalues.h"
#include "stdio.h"

#include "mark_and_sweep.h" // pour NewFreeListArray
#include "list.h"

caml_domain_state *Caml_state;

void caml_init_domain(){
  Caml_state = malloc(sizeof(caml_domain_state));
  Caml_state->stack = malloc(Stack_size);

#ifdef STOP_AND_COPY
  SEMI_SPACE_SIZE = 32 * KB;
  Caml_state->from_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->to_space = malloc(SEMI_SPACE_SIZE);
  Caml_state->heap_pointer = Caml_state->from_space;
#endif

#ifdef MARK_AND_SWEEP
  NewFreeListArray(Caml_state->freelist_array);
  Caml_state->pages = Empty;
  Caml_state->objects = Empty;
  Caml_state->big_objects = Empty;
#endif
}

void free_domain(){
  free(Caml_state->stack);
#ifdef STOP_AND_COPY
  free(Caml_state->from_space);
  free(Caml_state->to_space);
#endif
#ifdef MARK_AND_SWEEP
  free(Caml_state->freelist_array);
  list_delete(&Caml_state->pages);
  list_delete_structure(&Caml_state->objects);
  list_delete(&Caml_state->big_objects);
#endif
  free(Caml_state);
}
