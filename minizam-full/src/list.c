#include "list.h"

int length (list l){
    int n = 0;
    while (l != Empty){ ++n; l = l->next; }
    return n;
}
