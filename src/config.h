#ifndef _CONFIG_H
#define _CONFIG_H

// GC à utiliser
#define STOP_AND_COPY
//#define MARK_AND_SWEEP

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

#define ZONE_SIZE 16
#define NB_FREELIST (BIG_OBJECT_MIN_SIZE / ZONE_SIZE)

#define BIG_OBJECT_MIN_SIZE (32 * KB)
#define PAGE_SIZE (64 * KB)

#endif /* _CONFIG_H */
