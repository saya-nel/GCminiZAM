#ifndef _CONFIG_H
#define _CONFIG_H

// #define MARK_AND_SWEEP
#define STOP_AND_COPY // GC à utiliser

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

/* The size of the stack. */
/* Note that it doesn't hold 8MB mlvalues, but rather 8MB of bytes. */
/* No boundary-checks are done: stack overflow will silently corrupt
   the heap; probably causing something to go wrong somewhere. */
/* TODO: auto-growing stack, or throw stack overflow when needed. */
#define Stack_size (8 * MB)

// config stop and copy
#define SEMI_SPACE_SIZE (1 * KB)


#define BIG_OBJECT_MIN_SIZE (32 * KB)
#define PAGE_SIZE (64 * KB)
#endif
