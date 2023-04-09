//Struct que vamos a emplear para guardar en la memoria compartida

struct datosCompartida {
    int numeroEspacio;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    int clave;
    int endProcess;
    char buffer[5];
    FILE *TxtEmisor;
    FILE *TxtReceptor;
};