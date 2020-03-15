#include <stdlib.h>
#include "mlvalues.h"

struct cell {
    mlvalue * b;
    struct cell * next; // 0 terminated
};
typedef struct cell * fl_t;

#define Bk(x) ((x)->b)
#define Next(x) ((x)->next)

void insert_fl(mlvalue * v, fl_t * fl);
void print_fl (fl_t fl);

mlvalue *mark_and_sweep_alloc(size_t n);
