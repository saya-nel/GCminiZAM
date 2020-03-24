#include "stop_and_copy.h"

/*
  Nettoie le tas passer en paramètre
*/
void clear_heap(mlvalue *too_clear)
{
  for (int i = 0; i < (int)(SEMI_SPACE_SIZE / sizeof(mlvalue)); i++)
    too_clear[i] = 0;
}

/*
  Return 1 si le tas peut alloué n octets, 0 sinon
*/
int heap_can_alloc(size_t n)
{
  return (Caml_state->heap_pointer + n) <= (Caml_state->from_space + (SEMI_SPACE_SIZE / sizeof(mlvalue)));
}

void run_gc()
{
  // pour parcourir la pile
  mlvalue *curr = Caml_state->stack;    // premier element de la pile
  mlvalue *next = Caml_state->to_space; // premiere pos qu'on peut allouer dans to_space
  clear_heap(next);

  while (curr < &Caml_state->stack[sp]) // on parcours toute la pile
  {
    if (Is_block(*curr)) // si on est sur un pointeur dans la pile
    {
      // on regarde si le tag est FWD_PTR_T
      if (Tag(*curr) == FWD_PTR_T)
      {
        // on pointe la pile vers la valeur du forwarding pointer
        *curr = Field0(*curr);
      }
      else
      {
        // on créer une copie dans to_space du bloc courant dans from_space (header compris)
        mlvalue *old = next; // sauvegarde de l'endroit ou on a copier dans to_space
        memcpy(next, &(Hd_val(*curr)), (Size(*curr) + 1) * sizeof(mlvalue));
        next += Size(*curr) + 1; // prochaine position disponible dans to_space
        // on change son tag dans from_space en FWD_PTR_T
        Hd_val(*curr) = Make_header(Size(*curr), WHITE, FWD_PTR_T);
        // ajoute le forward pointer dans from_space vers la nouvelle position dans to_space
        Field0(*curr) = Val_ptr(old + 1); // on pointe sur le premier bloc
        // on pointe (la pile) vers le nouvel objet dans to_space
        *curr = Val_ptr(old + 1); // on pointe sur le premier bloc
      }
    }
    curr++; // on va sur l'élément suivant dans la pile
  }

  // on parcours maintenant to_space jusqu'à next, et fait pareil
  curr = Caml_state->to_space + 1; // on ce place apres le premier header de to_space
  while (curr < next)              // on parcours tout to_space
  {
    mlvalue *last_of_block = curr + Size(Val_ptr(curr)); // pos du header apres le bloc courant

    while (curr < last_of_block) // tans qu'on est sur le bloc actuel
    {
      if (Is_block(*curr)) // si c'est un pointeur
      {
        printf("isblock : %ld\n", *curr);
        // on regarde si le tag de la valeure pointée est FWD_PTR_T
        if (Tag(*curr) == FWD_PTR_T)
        {
          printf("is fwd\n");
          // on pointe curr vers la valeur du forwarding pointer
          *curr = Field0(*curr);
        }
        else
        {
          printf("is not fwd\n");
          // on deplace l'objet dans to_space
          printf("tag : %ld, size : %ld\n", Tag(*curr), Size(*curr));
          memcpy(next, &(Hd_val(*curr)), (Size(*curr) + 1) * sizeof(mlvalue));
          mlvalue *old = next;     // sauvegarde de l'endroit ou on a copier dans to_space
          next += Size(*curr) + 1; // prochaine position disponible dans to_space
          // on change son tag dans from_space en FWD_PTR_T
          Hd_val(*curr) = Make_header(Size(*curr), WHITE, FWD_PTR_T);
          // ajoute le forward pointer dans from_space vers la nouvelle position dans to_space
          Field0(*curr) = Val_ptr(old + 1);
          // on pointe to_space vers le nouvel objet dans to_space
          *curr = Val_ptr(old + 1);
        }
      }
      curr++;
    }
    curr = last_of_block + 1; // on saute le header d'après
  }

  // on a finit, on echange from_space et to_space
  mlvalue *tmp = Caml_state->from_space;
  Caml_state->from_space = Caml_state->to_space;
  Caml_state->to_space = tmp;
  Caml_state->heap_pointer = next;
  clear_heap(Caml_state->to_space);
}

mlvalue *stop_and_copy_alloc(size_t n)
{
  n = n / sizeof(mlvalue);
  if (heap_can_alloc(n))
  {
    mlvalue *res = Caml_state->heap_pointer; // premier bloc vide
    Caml_state->heap_pointer += n;           // on alloue n octet
    return res;
  }
  else
  {
    printf("Plus de place dans from_space, lancement GC.\n");
    run_gc();
    return stop_and_copy_alloc(n * sizeof(mlvalue));
  }
}