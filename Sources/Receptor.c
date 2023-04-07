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

int main(int argc, char *argv[]){
        // Valores compartidos
    char *Modo;
    int clave;

    // Verificar que se hayan ingresado los 2 argumentos
    if (argc != 2) {
        printf("Uso: programa <string1> <numero1>\n");
        return 1;
    }

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    clave = atoi(argv[2]);

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("sem_llenos",0);
    sem_vacios = sem_open("sem_vacios",0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;
    
    char *ID="buffer1";

    // Crear una clave única para la memoria compartida
    key_t key = ftok("Data/shmID", *ID);

    int tamano = sizeof(struct datosCompartida);
    
    // Copiamos la memoria compartida
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

    datos->contReceptoresVivos++;
    sem_post(sem_vacios);
    sem_wait(sem_llenos);
    
    /////////////////// Zona critica ////////////////////

    printf("\n");
    printf("%d",datos->contReceptoresVivos);

    //datos->indiceReceptor++;
    //datos->contReceptoresVivos--;
    //datos->contReceptoresTotal++;

    time_t tiempo_actual = time(NULL);                    // Obtenemos el tiempo actual en segundos
    struct tm *tiempo_local = localtime(&tiempo_actual);  // Convertimos el tiempo en una estructura tm
    printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);

    return 0;
}
