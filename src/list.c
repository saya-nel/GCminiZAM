#include "list.h"

#include <stdio.h>
#include <stdlib.h>

int length (list l){
  int n = 0;
  while (l != Empty){ 
  	++n; 
  	l = l->next; 
  }
  return n;
}

void list_delete(list * l){
  while (*l != Empty){ 
    FreeCar(*l);
  }
}

void print_list(list l){
  printf("[");
  while (l != Empty){
    printf("%llx|",(uint64_t) l->content);
    l = l->next;
  }
  printf("]\n");
}
