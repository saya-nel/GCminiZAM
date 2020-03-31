#include "list.h"

#include <stdio.h>
#include <stdlib.h>

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

void print_list(list l){
	printf("[");
    while (l){
        printf("%x|",l->content);
        l = l->next;
    }
    printf("]\n");
}
