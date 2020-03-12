#include "mark_and_sweep.h"

mlvalue *mark_and_sweep_alloc(size_t size)
{
  return malloc(8 * size);
}
