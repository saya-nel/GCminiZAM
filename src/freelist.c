#include <assert.h>
#include <stdio.h>

#include "freelist.h"
#include "mlvalues.h"

// ne pas insérer deux fois la même valeur
// car créé un cycle
void cons_fl(mlvalue bk, freelist_t *fl){
  NextFL(bk) = *fl;
  *fl = bk;
}

// libération d'une freelist
// NB : utile seulement si les objets ont été alloués individuellement avec malloc
void delete_freelist(freelist_t fl){
  mlvalue * p;
  if (fl){
    p = Ptr_val(fl) - 1;
    delete_freelist(NextFL(fl));
    free(p);
  }
}

void insert_fl(mlvalue bk, freelist_t *fl){
  mlvalue *cur, *p;
  if (!*fl){
    cons_fl(bk, fl);
    return;
  }
  if (Size(bk) <= Size(*fl)){
    cons_fl(bk, fl);
    return;
  }
  cur = fl;
  while (NextFL(cur)){
    p = (mlvalue *)NextFL(cur);
    if (Size(bk) <= Size(p)){
      cons_fl(bk, cur);
      return;
    }
    cur = p;
  }
  cons_fl(bk, cur);
}

void print_fl(freelist_t fl){
  printf("(");
  for (; fl != NilFL; fl = NextFL(fl)){
    printf("[0x%lx]{size = %ld} ;", fl, Size(fl));
  }
  printf(" Nil)\n");
}

mlvalue *first_fit(size_t sz, freelist_t *fl){
  /* rend le premier bloc de taille suffisante dans le free_list, ou Nil */
  mlvalue cur, p;

  if (*fl){
    cur = *fl;
    if (Size(cur) >= sz){
      *fl = NextFL(*fl);
      return (Ptr_val(cur) - 1);
    }
    while (NextFL(cur)){
      if (Size(NextFL(cur)) >= sz){

        p = NextFL(cur); 
        NextFL(cur) = NextFL(NextFL(cur));
        return (Ptr_val(p) - 1);
      }
      cur = NextFL(cur);
    }
  }
  return NilFL;
}

int length_fl(freelist_t fl){
  int n = 0;
  while (fl != NilFL){
    ++n;
    fl = NextFL(fl);
  }
  return n;
}

int compare_size(mlvalue v1, mlvalue v2){
  return v1 <= v2;
  return Size(v2) - Size(v1);
}

int compare_addr(mlvalue v1, mlvalue v2){
  return v2 - v1;
}

void insert_freelist(int compare(mlvalue, mlvalue), mlvalue bk, mlvalue *fl){
  mlvalue *cur, *p;
  if (!*fl){
    cons_fl(bk, fl);
    return;
  }
  if (compare(bk, *fl) <= 0){
    cons_fl(bk, fl);
    return;
  }
  cur = fl;
  while (NextFL(cur)){
    p = (mlvalue *)NextFL(cur);
    if (compare(bk, *p) <= 0){
      cons_fl(bk, cur);
      return;
    }
    cur = p;
  }
  cons_fl(bk, cur);
}
