#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>            // Necesario para O_CREAT y O_EXCL
#include "datosCompartidos.h" // Estructura
#include <time.h>

int main(){

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;
    char *ID="buffer1";

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("sem_llenos",0);
    sem_vacios = sem_open("sem_vacios",0);

    // Crear una clave única para la memoria compartida
    key_t key = ftok("Data/shmID", *ID);

    int tamano = sizeof(struct datosCompartida);
    
    // Crear la memoria compartida
    int shmid = shmget(key, tamano, 0666 | IPC_CREAT);
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
    int tempEmisores=datos->contEmisoresVivos;
    int tempReceptores=datos->contReceptoresVivos;
    
    // Cerramos los procesos en espera
    for (int i=0;i < datos->contEmisoresVivos; i++){
        datos->indiceEmisor=1;
        datos->indiceReceptor=1;
        //Leemos algo
        sem_post(sem_vacios);
        sem_wait(sem_llenos);
    }
    for (int i=0;i < datos->contReceptoresVivos; i++){
        datos->indiceEmisor=1;
        datos->indiceReceptor=1;
        //Escribimos algo
        sem_post(sem_vacios);
        sem_wait(sem_llenos);
    }


    // Cerramos semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");

    //Liberamos el espacio de memoria
    shmdt(datos);
    return 0;
}
