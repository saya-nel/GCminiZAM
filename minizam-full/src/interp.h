#ifndef _INTERP_H
#define _INTERP_H

#include "mlvalues.h"

unsigned int sp;

mlvalue caml_interprete(code_t* prog);

#endif /* _INTERP_H */
