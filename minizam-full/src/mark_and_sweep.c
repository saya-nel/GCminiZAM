#include "mark_and_sweep.h"
#include "mlvalues.h"
#include <assert.h>
#include <stdio.h>

fl_t cell_alloc(mlvalue * v){
	fl_t p = malloc(sizeof(fl_t));
	Bk(p) = v;
	Next(p) = 0;
	return p;
}

// maintenir une free list par adresse croissante, pour faciliter 
// la fusion des blocs. [cf. runtime de Caml-light]
void insert_fl(mlvalue * b, fl_t * fl){
	fl_t cur, p;
	p = cell_alloc(b);
	
	if (!fl) { 
		fl = malloc(sizeof(fl_t));
		*fl = p; 
	} else {
		cur = *fl;
		if (b < Bk(cur)){
			Next(p) = cur;
			*fl = p;
		} else {
			while (Next(cur) && (b > Bk(Next(cur)))){
				cur = Next(cur);
			}
			Next(p) = Next(cur);
			Next(cur) = p;
		}
	}

}

void print_fl (fl_t fl){
	while (fl){
		print_val(Bk(fl)[0]);
		printf ("(addr: %lld)|",Bk(fl)[0]);
		fl = Next(fl);
	}
	printf ("\n");
}

static mlvalue *alloc_in_fl(size_t sz, fl_t * pfl){
	mlvalue * v = malloc(8*sz);
	insert_fl(v,pfl);
	return v;
}


static fl_t * fl_big = NULL; 
static fl_t * fl_others = NULL; 

mlvalue *mark_and_sweep_alloc(size_t sz){
  if (sz > 0x8000){
    return alloc_in_fl(sz,fl_big);
  } 
  return alloc_in_fl(sz,fl_others);
}
