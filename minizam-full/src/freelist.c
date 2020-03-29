#include "freelist.h"
#include <assert.h>
#include <stdio.h>

void cons_fl (mlvalue *bk, freelist_t * fl){
  // attention à ne pas faire un cycle
  NextFL(bk) = *fl;
  *fl = *bk;
}

void print_fl (freelist_t fl){
  printf ("(");
  for ( ;fl != NilFL ; fl = NextFL(fl)){
    printf ("[0x%llx]{size = %lld} ;",fl,Size(fl));
  }
  printf (" Nil)\n");
}

mlvalue first_fit(size_t sz, freelist_t * fl){
  /* rend le premier bloc de taille suffisante dans le free_list, ou Nil */
  mlvalue cur,p;
  if (*fl != NilFL){
    cur = *fl;
    if (Size(cur) >= sz){
      *fl = NextFL(cur);
      return cur;
    }
    while (NextFL(cur)){
      if (Size(NextFL(cur)) >= sz){
	p = NextFL(cur);
	NextFL(cur) = NextFL(NextFL(cur));
	return p;
      }
    }
  }
  return NilFL;
}
