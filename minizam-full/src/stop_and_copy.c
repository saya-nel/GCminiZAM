#include <stdlib.h>

#include "mlvalues.h"

/*
typedef struct {
	size_t capacity, size;
	enum {FROM_SPACE,TO_SPACE} status;
	mlvalue* semiSpace0;
	mlvalue* semiSpace1;
} heap_t;

heap_t heap;
*/
#include<stdio.h>
mlvalue * stop_and_copy(size_t n){
	printf("---> alloc stop and copu\n");
	return malloc(8 * n);
  /*mlvalue* semiSpace;
  switch (Caml_state.heap.status){
    case FROM_SPACE: semiSpace = Caml_state.heap.semiSpace0; break;
    case FROM_SPACE: semiSpace = Caml_state.heap.semiSpace1; break;
  }
  if (Caml_state.heap.size < Caml_state.heap.capacity){
  	return semiSpace[Caml_state.heap.size++];
  } else {
  	// error
  }*/
}

/*
mlvalue * make_heap (size){
  return malloc(size * (sizeof mlvalues));
}


*/
