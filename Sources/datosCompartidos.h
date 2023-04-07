#ifndef datosCompartidos
#define datosCompartidos
#include <semaphore.h>
//Struct que vamos a emplear para guardar en la memoria compartida
struct datosCompartida{
    sem_t *sem_llenos, *sem_vacios;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    int clave;
};

#endif /* datosCompartidos */