#ifndef _FREELIST_H
#define _FREELIST_H

#include "mlvalues.h"

// chaque freelist se termine par NilFL.
#define NilFL           (0)

#define First(v) ((mlvalue) v)

// le pointeur vers le bloc libre suivant est stoqué dans le champ 0 du bloc courant
// implique qu'un bloc vide ait bien un champ 0 pour stocker se pointeur
#define NextFL(v)     (Field0(v))

typedef mlvalue freelist_t;



// ajout d'une mlvalue b à la freelist fl (passée par référence)
void cons_fl (mlvalue bk,freelist_t * fl);

void insert_fl (mlvalue bk, freelist_t * fl);

// affiche la freelist fl
void print_fl (freelist_t fl);

// recherche un bloc libre de taille >= sz dans la freelist fl (passée par référence).
// si un tel bloc b existe dans fl, alors b est retiré de la freelist, et b est retourné.
// sinon, renvoie NilFL.
mlvalue * first_fit(size_t sz,freelist_t * fl);

// retourne un entier inferieur à 0 si Size(v1) < Size(v2)
int compare_size(mlvalue v1, mlvalue v2);

// retourne un entier inferieur à 0 si l'addresse de v1 et inferieur à l'adresse de v2
int compare_addr(mlvalue v1, mlvalue v2);

// insert v dans la freelist pointée par fl, triée suivant la fonction de comparaison spécifiée 
void insert_freelist (int compare (mlvalue,mlvalue), mlvalue v, mlvalue * fl);

#endif /* _FREELIST_H */