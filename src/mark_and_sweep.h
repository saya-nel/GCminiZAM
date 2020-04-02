#ifndef _MARK_AND_SWEEP_H
#define _MARK_AND_SWEEP_H


#include <stdlib.h>
#include "mlvalues.h"
#include "config.h"
#include "freelist.h"

#include "domain_state.h"

void recurvive_mark(mlvalue racine);
void iterative_mark(mlvalue racine);
void mark();

mlvalue *mark_and_sweep_alloc(size_t n);

#endif /* _MARK_AND_SWEEP_H */
