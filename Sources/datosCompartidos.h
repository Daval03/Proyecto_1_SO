#ifndef datosCompartidos
#define datosCompartidos

//Struct que vamos a emplear para guardar en la memoria compartida

struct datosCompartida {
    int numeroEspacio;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    char clave;
    int buffer[256];
    //char buffer[256];
    //FILE * fileTxt;
    //char fileTxtReceptor;
};

#endif /* datosCompartidos */