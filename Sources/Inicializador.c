#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>            // Necesario para O_CREAT y O_EXCL
#include "datosCompartidos.h" // Estructura

int main(int argc, char *argv[]) {

    // Valores ingresados
    char *ID;
    int clave;
    int numeroEspacio;

    // Obtener los argumentos y convertir el número a entero
    ID = argv[1];
    clave = atoi(argv[2]);
    numeroEspacio = atoi(argv[3]);

    // Para correr la vara varias veces borramos los semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    
    // Crear sem_llenos con nombre "sem_llenos" y un valor inicial de 0
    sem_llenos = sem_open("/sem_llenos", O_CREAT | O_EXCL, 0644, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Crear sem_vacios con nombre "sem_vacios" y un valor inicial de numeroEspacio
    sem_vacios = sem_open("/sem_vacios", O_CREAT | O_EXCL, 0644, numeroEspacio);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;
    
    // Crear una clave única para la memoria compartida
    key_t key = ftok("Data/shmid.txt", *ID);

    size_t tamaño = sizeof(struct datosCompartida);

    // Crear la memoria compartida
    int shmid = shmget(key, tamaño, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }   
    // Asignar la estructura a la memoria compartida
    datos = shmat(shmid, NULL, 0);
    if (datos == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // Asignar valores a la estructura
    //datos->buffer = malloc(numeroEspacio * sizeof(int));
    datos->clave = clave;
    datos->numeroEspacio = numeroEspacio;
    datos->contEmisoresTotal=0;
    datos->contReceptoresTotal=0;
    datos->contEmisoresVivos=0;
    datos->contReceptoresVivos=0;
    datos->indiceEmisor=0;
    datos->indiceReceptor=0;
    datos->indiceTxtEmisor=0;
    datos->TxtReceptor=fopen("Data/Receptor.txt","w");

    printf("\n");
    printf("| %-15s | %-10s | %-10s |\n", "ID", "Clave", "Numero de espacios");
    printf("| %-15s | %-10d | %-18d |\n", ID, clave, numeroEspacio);
    printf("\n");
    return 0;
}
