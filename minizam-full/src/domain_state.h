#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"

typedef struct _caml_domain_state
{
  /* Stack */
  mlvalue *stack;

  // éléments utilisés pour stop_and_copy
  mlvalue *from_space;
  mlvalue *to_space;
  mlvalue *heap_pointer; // premiere position allouable

  // éléments utilisés pour le mark_and_sweep

} caml_domain_state;

/* The global state */
extern caml_domain_state *Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();

#endif
