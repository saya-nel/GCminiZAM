#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int length(list l)
{
  int n = 0;
  while (l != Empty)
  {
    ++n;
    l = l->next;
  }
  return n;
}

void list_delete(list *l)
{
  while (*l != Empty)
  {
    FreeCar(*l);
  }
}

void list_delete_structure(list *l)
{
  list c;
  while (*l != Empty)
  {
    c = *l;
    *l = (*l)->next;
    free(c);
  }
}

void print_list(list l)
{
  printf("[");
  while (l != Empty)
  {
    printf("%lx|", (uint64_t)l->content);
    l = l->next;
  }
  printf("]\n");
}
