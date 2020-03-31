#ifndef _LIST_H
#define _LIST_H

/* liste de pointeurs */

struct cell {
    void * content;
    struct cell * next;
};
typedef struct cell * list;

#define Empty (0)

#define Next(c) ((c)->next)

/* void(list c) */
#define Free_cell(c) do {            \
free(c->content);                    \
free(c);                             \
} while (0);

#define Cons(x,l) do {               \
list Cons = malloc(sizeof(list));    \
Cons->content = (void *) x;          \
Cons->next = ((list) l);             \
l = Cons;                            \
} while (0);

#define Cdr(l) do {                  \
list Cdr = l;                        \
l = (l)->next;                       \
Free_cell(Cdr);                      \
} while (0);

#define RemoveCadr(l) do {            \
list RemoveCadr = (l)->next;          \
(l)->next = (l)->next->next;          \
Free_cell(RemoveCadr);                \
} while (0);


int length (list l);

void list_delete(list * l);

void print_list(list l);
#endif /* _LIST_H */
