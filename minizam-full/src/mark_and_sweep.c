#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"
#include "stdio.h"

mlvalue *mark_and_sweep_alloc(size_t size)
{
  printf("Mark and sweep alloc\n");
  return malloc(8 * size);
}
