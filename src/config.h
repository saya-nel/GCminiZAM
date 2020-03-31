#ifndef _CONFIG_H
#define _CONFIG_H

// GC à utiliser
//#define STOP_AND_COPY
#define MARK_AND_SWEEP


#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

/* The size of the stack. */
/* Note that it doesn't hold 8MB mlvalues, but rather 8MB of bytes. */
/* No boundary-checks are done: stack overflow will silently corrupt
   the heap; probably causing something to go wrong somewhere. */
/* TODO: auto-growing stack, or throw stack overflow when needed. */
#define Stack_size (64 * MB)

// config stop and copy
long SEMI_SPACE_SIZE;

#define FREELIST_ARRAY_RANGE 64
#define NB_FREELIST (BIG_OBJECT_MIN_SIZE / FREELIST_ARRAY_RANGE)


// seul l'allocation de gros objets est fonctionnel sur les jeux de test fournis
#define BIG_OBJECT_MIN_SIZE  0 // (32 * KB)
#define PAGE_SIZE (64 * KB)

#endif /* _CONFIG_H */
