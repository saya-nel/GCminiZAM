#include "list.h"

int length (list l){
    int n = 0;
    while (l != Empty){ ++n; l = l->next; }
    return n;
}

void list_delete(list * l){
    while (*l){
        Cdr(*l);
    }
}
