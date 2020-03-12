#include "stop_and_copy.h"

mlvalue *stop_and_copy_alloc(size_t n)
{
  return malloc(8 * n);
}