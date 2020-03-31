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

// libère le pointeur l vers une cellule
// ainsi que le pointeur (void *) contenu dans cette cellule
#define Free_cell(l) do {			\
    free(l->content);				\
    free(l);					\
  } while (0);

// ajout d'un pointeur p (de tout type) dans la liste l
#define Cons(p,l) do {				\
    list Cons = malloc(sizeof(list));		\
    Cons->content = (void *) p;			\
    Cons->next = ((list) l);			\
    l = Cons;					\
  } while (0);

// supprime le premier élément de la liste l, 
// et affecte à l son cdr.
#define FreeCar(l) do {				\
    list FreeCar = l;				\
    l = (l)->next;				\
    Free_cell(FreeCar);				\
  } while (0);

// suprime le deuxième élément d'une liste l
#define FreeCadr(l) do {			\
    list FreeCadr = (l)->next;			\
    (l)->next = (l)->next->next;		\
    Free_cell(FreeCadr);			\
  } while (0);

// calcul la longeur de la liste l
int length (list l);

// libère chaque cellule de la liste l 
// et l'ensemble des pointeurs stockés dans l
void list_delete(list * l);

// libère chaque cellule de la liste l 
// sans liberer les pointeurs stockés dans l
void list_delete_structure(list * l);

// affiche les adresses des pointeurs stockés dans la liste l
void print_list(list l);

#endif /* _LIST_H */
