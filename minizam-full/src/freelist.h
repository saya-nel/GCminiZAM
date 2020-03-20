#include "mlvalues.h"

// chaque freelist se termine par NilFL.
#define NilFL           (0)

// le pointeur vers le bloc libre suivant est stoqué dans le champ 0 du bloc courant
// implique qu'un bloc vide ait bien un champ 0 pour stocker se pointeur
#define NextFL(v)     (Field0(v))

// ajout d'une mlvalue b à la freelist fl (passée par référence)
void cons_fl (mlvalue * bk, mlvalue * fl);

// affiche la freelist fl
void print_fl (mlvalue fl);

// recherche un bloc libre de taille >= sz dans la freelist fl (passée par référence).
// si un tel bloc b existe dans fl, alors b est retiré de la freelist, et b est retourné.
// sinon, renvoie NilFL.
mlvalue first_fit(size_t sz, mlvalue * fl);

