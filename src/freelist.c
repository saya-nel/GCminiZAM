#include "freelist.h"
#include <assert.h>
#include <stdio.h>
#include "mlvalues.h"

#define Debug(x) //(x)

void cons_fl (mlvalue bk, freelist_t * fl){
  // attention à ne pas faire un cycle
   Debug (printf ("fa\n"));
   Debug (printf ("fi %d\n",Size(bk)));
  NextFL(bk) = *fl;
  *fl = bk;
  Debug (printf ("fu\n"));
}
void insert_fl (mlvalue bk, freelist_t * fl){
  mlvalue *cur,*p;
 //printf ("fi %d\n",Size(bk));
  if (!*fl){
    cons_fl(bk,fl); 
    return;
  }
  Debug (printf ("foo\n"));
  if (Size(bk) <= Size(*fl)){ 
    cons_fl(bk,fl); 
    return; 
  }
   Debug (printf ("bar\n"));
  cur = fl;
  while (NextFL(cur)){
    p = (mlvalue *) NextFL(cur);
    Debug ( printf ("cho %d\n",Size(p)));
    if (Size(bk) <= Size(p)){
      Debug (printf ("ki\n"));
      cons_fl(bk,cur);
      return;
    } 
   Debug (printf ("kou\n"));
    cur = p;
  }
  Debug (printf ("toto\n"));
  cons_fl(bk,cur);
  Debug (printf ("titi\n"));
}

void print_fl (freelist_t fl){
  printf ("(");
  for ( ;fl != NilFL ; fl = NextFL(fl)){
    printf ("[0x%llx]{size = %lld} ;",fl,Size(fl));
  }
  printf (" Nil)\n");
}

mlvalue *first_fit(size_t sz, freelist_t * fl){
  /* rend le premier bloc de taille suffisante dans le free_list, ou Nil */
  mlvalue cur,p;
  if (*fl){
    cur = *fl;
    if (Size(cur) >= sz){
      p = *fl;
      *fl = NextFL(cur);
      return Ptr_val(p)-1;
    }
    while (NextFL(cur)){
      if (Size(NextFL(cur)) >= sz){
      	p = NextFL(cur);
      	NextFL(cur) = NextFL(NextFL(cur));
      	return Ptr_val(p)-1;
      }
      cur = NextFL(cur);
    }
  }
  return NilFL;
}

/*
mlvalue *first_fit(size_t sz, freelist_t * fl){
  mlvalue cur,p;
  if (*fl){
    cur = *fl;
    if (Size(cur) >= sz){
      p = *fl;
      *fl = NextFL(cur);
      return Ptr_val(p)-1;
    }
    while (NextFL(cur)){
      if (Size(NextFL(cur)) >= sz){
        p = cur;
        NextFL(cur) = NextFL(NextFL(cur));
        return Ptr_val(p)-1;
      }
      cur = NextFL(cur);
    }
  }
  return NilFL;
}
*/