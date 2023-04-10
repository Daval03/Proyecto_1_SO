//Struct que vamos a emplear para guardar en la memoria compartida

struct datosCompartida {
    int numeroEspacio; //4
    int indiceEmisor, indiceReceptor, indiceTxtEmisor; //12
    int contEmisoresVivos, contReceptoresVivos; //8
    int contEmisoresTotal, contReceptoresTotal; //8
    int clave; //4
    int endProcess; //4
    char buffer[0];
};
//4+12+8+8+4+4