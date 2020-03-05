#define MARK_AND_SWEEP

#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"


#ifdef STOP_AND_COPY
#include "stop_and_copy.h"
#endif
#ifdef MARK_AND_SWEEP
#include "mark_and_sweep.h"
#endif
mlvalue* caml_alloc(size_t size) {
  return mark_and_sweep_alloc(size);
}
