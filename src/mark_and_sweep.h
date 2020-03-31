#ifndef _MARK_AND_SWEEP_H
#define _MARK_AND_SWEEP_H


#include <stdlib.h>
#include "mlvalues.h"
#include "config.h"
#include "freelist.h"

#include "domain_state.h"

#define NewFreeListArray(freelist_array) do {                   \
    freelist_array = malloc (NB_FREELIST * sizeof(freelist_t)); \
    for (size_t __i = 0; __i < NB_FREELIST; __i++){             \
      freelist_array[__i] = NilFL;                              \
    }                                                           \
  } while (0);

// rend un pointeur sur la freelist cible d'un bloc de taille sz 
#define SelectFreelist(sz) (&Caml_state->freelist_array[(sz/FREELIST_ARRAY_RANGE)])


mlvalue *mark_and_sweep_alloc(size_t n);

#endif /* _MARK_AND_SWEEP_H */
