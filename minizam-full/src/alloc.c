#define STOP_AND_COPY

#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"

// On inclu un des deux GC

#ifdef STOP_AND_COPY
#include "stop_and_copy.h"
#endif
#ifdef MARK_AND_SWEEP
#include "mark_and_sweep.h"
#endif

mlvalue *caml_alloc(size_t size)
{
// on appel la fonction d'allocation du bon GC
#ifdef STOP_AND_COPY
  return stop_and_copy_alloc(size);
#endif
#ifdef MARK_AND_SWEEP
  return mark_and_sweep_alloc(size);
#endif
}
