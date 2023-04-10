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
#include <unistd.h>

int main(){

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;
    char *ID="buffe1";

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios, *sem_mutexR, *sem_mutexE;

    sem_llenos = sem_open("/sem_llenos",0);
    sem_vacios = sem_open("/sem_vacios",0);
    sem_mutexR = sem_open("/sem_mutexR", 0);
    sem_mutexE = sem_open("/sem_mutexE", 0);

    // Crear una clave única para la memoria compartida
    key_t key = ftok("tmp", *ID);

    int tamaño = sizeof(struct datosCompartida);
    
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

    datos->endProcess = 1;
    int tempReceptores = datos->contReceptoresVivos;
    int casoInicial=1;

    printf("\n %d \n", tempReceptores);
    
    //Terminar los receptores en cola
    for (int i=0; i < datos->contReceptoresVivos;i++){
        sem_wait(sem_vacios);
        datos->indiceReceptor = -1;
        sem_post(sem_llenos);
        sleep(5);
    }

    //Terminar los emisores en cola
    for (int i=0; i < datos->contEmisoresVivos;i++){
        sem_wait(sem_llenos);
        datos->indiceEmisor= -1;
        sem_post(sem_vacios);
        sleep(5);
    }

    // Cerramos semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");
    sem_unlink("/sem_mutexE");
    sem_unlink("/sem_mutexR");

    //Liberamos el espacio de memoria
    if (shmdt(datos) == -1) {  // desasignar del segmento compartido
        perror("Error eliminando asignacion del seg compartido");
        return 1;
    }
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {    //remover bloque shm
        perror("Error removiendo el bloque de mem compartida");
        exit(1);
    }

    return 0;
}