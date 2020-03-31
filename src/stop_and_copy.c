#include "stop_and_copy.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "domain_state.h"
#include "config.h"
#include "interp.h"

mlvalue *next;

/*
  Return 1 si le tas peut alloué n octets, 0 sinon
*/
int heap_can_alloc(size_t n)
{
  return (Caml_state->heap_pointer + n) < (Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
}

void resize_spaces()
{
  // on traite le redimensionnement des semi spaces si nécéssaire
  int half = (SEMI_SPACE_SIZE / sizeof(mlvalue)) / 2;               // nombre d'éléments a la moitié d'un semi space
  int quarter = half / 2;                                           // nombre d'élements au quart d'un semi space
  int nb_elems = Caml_state->heap_pointer - Caml_state->from_space; // nombre d'élement dans le from_space
  // définition de la nouvelle taille
  long old_size = SEMI_SPACE_SIZE;
  if (Caml_state->from_space + half < Caml_state->heap_pointer) // si remplis à plus de 50%
    SEMI_SPACE_SIZE = SEMI_SPACE_SIZE * 1.5;
  else if (Caml_state->from_space + quarter > Caml_state->heap_pointer) // si remplis à moins de 25%
    SEMI_SPACE_SIZE = SEMI_SPACE_SIZE / 2;

  if (old_size != SEMI_SPACE_SIZE)
  {
    // création du nouveau from_space à la bonne taille
    mlvalue *new_from_space = malloc(SEMI_SPACE_SIZE);
    // copie de l'ancien from_space dans le nouveau
    memcpy(new_from_space, Caml_state->from_space, nb_elems * sizeof(mlvalue));

    // on ajuste accu et env si c'est des pointeus
    if (Is_block(accu))
    {
      int index = &(Field0(accu)) - Caml_state->from_space;
      accu = Val_ptr(new_from_space + index);
    }
    if (Is_block(env))
    {
      int index = &(Field0(env)) - Caml_state->from_space;
      env = Val_ptr(new_from_space + index);
    }

    // on ajuste les pointeurs de la pile vers le nouveau from_space
    for (int i = 0; i < (int)sp; i++) // on parcours les mlvalue dans la pile
    {
      if (Is_block(Caml_state->stack[i])) // le mlvalue est un pointeur vers un elem de l'ancien from_space
      {
        int index = &(Field0(Caml_state->stack[i])) - Caml_state->from_space; // sa position dans from_space
        Caml_state->stack[i] = Val_ptr(new_from_space + index);               // on fais pointer le mlvalue de la pile vers new_from_space
      }
    }

    // on ajuste les pointeurs new_from_space -> from_space à new_from_space -> new_from_space
    for (int i = 1; i < nb_elems;) // on parcours les objets en sautant le premier header
    {
      for (int j = 0; j < (int)Size(Val_ptr(new_from_space + i)); j++) // on parcours les fields de l'objet
      {
        if (Is_block(new_from_space[i + j])) // si le field est un pointeur
        {
          // on regarde la position de l'element vers lequel il pointe dans l'ancien from_space
          int index = &(Field0(Caml_state->from_space[i + j])) - Caml_state->from_space;
          // on corrige le pointeur dans new_from_spae
          new_from_space[i + j] = Val_ptr(new_from_space + index);
        }
      }
      i += Size(Val_ptr(new_from_space + i)) + 1; // on passe à l'objet suivant en sautant son header
    }

    // mlvalue *old = Caml_state->from_space;
    free(Caml_state->from_space);
    Caml_state->from_space = new_from_space;
    Caml_state->heap_pointer = Caml_state->from_space + nb_elems;
    free(Caml_state->to_space);
    Caml_state->to_space = malloc(SEMI_SPACE_SIZE);
  }
}

void move_addr(mlvalue *val)
{
  if (Is_block(*val)) // val pointe vers un block
  {
    if (Tag(*val) == FWD_PTR_T) // si le block est un fwd ptr
      *val = Field0(*val);      // on fais pointer val vers le fwd ptr
    else
    {
      // on copie tout le bloc (header comprit) dans to_space
      mlvalue *old = next; // sauvegarde de l'endroit ou on va copier dans to_space
      memcpy(next, &(Hd_val(*val)), (Size(*val) + 1) * sizeof(mlvalue));

      next += Size(*val) + 1; // prochaine position disponible dans to_space
      // on change son tag en FWD_PTR_T
      Hd_val(*val) = Make_header(Size(*val), Color(*val), FWD_PTR_T);
      // ajoute le forward pointer vers la nouvelle position dans to_space
      Field0(*val) = Val_ptr(old + 1); // on pointe sur le premier champ
      // on pointe vers le nouvel objet dans to_space
      *val = Val_ptr(old + 1); // on pointe sur le premier champ
    }
  }
}

void run_gc()
{

  // pour parcourir la pile
  mlvalue *curr = Caml_state->stack; // premier element de la pile
  next = Caml_state->to_space;       // premiere pos qu'on peut allouer dans to_space

  while (curr < &(Caml_state->stack[sp])) // on parcours toute la pile
  {
    move_addr(curr);
    curr++; // on va sur l'élément suivant dans la pile
  }

  // on traite l'accu
  move_addr(&accu);
  // on traite l'env
  move_addr(&env);

  mlvalue *scan = Caml_state->to_space + 1; // scan le tas, on ce place apres le premier header
  // on parcours le tas jusqu'a la premier position non allouée (next)
  while (scan < next)
  {
    for (int i = 0; i < (int)Size(Val_ptr(scan)); i++) // on parcours l'objet
    {
      move_addr(&scan[i]);
    }
    scan += Size(Val_ptr(scan)) + 1; // on passe a l'objet suivant et saute son header
  }                                  // fin while

  // on a finit, on echange from_space et to_space
  mlvalue *tmp = Caml_state->from_space;
  Caml_state->from_space = Caml_state->to_space;
  Caml_state->to_space = tmp;
  Caml_state->heap_pointer = next;

  // on redimensionne les espaces
  resize_spaces();
}

mlvalue *stop_and_copy_alloc(size_t n)
{
  int nb_mlvalue = n / sizeof(mlvalue);
  if (!heap_can_alloc(nb_mlvalue))
  {
    run_gc();
    if (!heap_can_alloc(nb_mlvalue))
    {
      printf("plus de mémoire\n");
      exit(1);
    }
  }
  mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
  Caml_state->heap_pointer += nb_mlvalue;  // on alloue n octet
  return res;
}