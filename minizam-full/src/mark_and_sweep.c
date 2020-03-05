#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"

mlvalue* mark_and_sweep_alloc(size_t size) {
    /* printf("--> mark_and_sweep_alloc\n"); */
    return malloc(8 * size);
}
