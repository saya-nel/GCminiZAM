#include "freelist.h"
#include <assert.h>
#include <stdio.h>
#include "mlvalues.h"

#define Debug(x) //(x)

// ne pas insérer deux fois la même valeur
// car créé un cycle
void cons_fl(mlvalue bk, freelist_t *fl)
{
  NextFL(bk) = *fl;
  *fl = bk;
}

void insert_fl(mlvalue bk, freelist_t *fl)
{
  mlvalue *cur, *p;
  if (!*fl)
  {
    cons_fl(bk, fl);
    return;
  }
  if (Size(bk) <= Size(*fl))
  {
    cons_fl(bk, fl);
    return;
  }
  cur = fl;
  while (NextFL(cur))
  {
    p = (mlvalue *)NextFL(cur);
    if (Size(bk) <= Size(p))
    {
      cons_fl(bk, cur);
      return;
    }
    cur = p;
  }
  cons_fl(bk, cur);
}

void print_fl(freelist_t fl)
{
  printf("(");
  for (; fl != NilFL; fl = NextFL(fl))
  {
    printf("[0x%lx]{size = %ld} ;", fl, Size(fl));
  }
  printf(" Nil)\n");
}

mlvalue *first_fit(size_t sz, freelist_t *fl)
{
  /* rend le premier bloc de taille suffisante dans le free_list, ou Nil */
  mlvalue cur, p;
  if (*fl)
  {
    cur = *fl;
    if (Size(cur) >= sz)
    {
      p = *fl;
      *fl = NextFL(cur);
      return Ptr_val(p) - 1;
    }
    while (NextFL(cur))
    {
      if (Size(NextFL(cur)) >= sz)
      {
        p = NextFL(cur);
        NextFL(cur) = NextFL(NextFL(cur));
        return Ptr_val(p) - 1;
      }
      cur = NextFL(cur);
    }
  }
  return NilFL;
}

int compare_size(mlvalue v1, mlvalue v2)
{
  return v1 <= v2;
  return Size(v2) - Size(v1);
}

int compare_addr(mlvalue v1, mlvalue v2)
{
  return v2 - v1;
}

void insert_freelist(int compare(mlvalue, mlvalue), mlvalue bk, mlvalue *fl)
{
  mlvalue *cur, *p;
  if (!*fl)
  {
    cons_fl(bk, fl);
    return;
  }
  if (compare(bk, *fl) <= 0)
  {
    cons_fl(bk, fl);
    return;
  }
  cur = fl;
  while (NextFL(cur))
  {
    p = (mlvalue *)NextFL(cur);
    if (compare(bk, *p) <= 0)
    {
      cons_fl(bk, cur);
      return;
    }
    cur = p;
  }
  cons_fl(bk, cur);
}
