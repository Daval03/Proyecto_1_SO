#ifndef datosCompartidos
#define datosCompartidos

//Struct que vamos a emplear para guardar en la memoria compartida
struct datosCompartida{
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    int clave;
};

#endif /* datosCompartidos */