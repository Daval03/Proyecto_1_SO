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
    // Valores recibidos
    char *Modo;
    char *ID;
    int clave;

    // Valor del textEmisor
    char text;

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    ID = argv[2];
    clave = atoi(argv[3]);

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("/sem_llenos",0);
    sem_vacios = sem_open("/sem_vacios",0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok("Data/shmid.txt", *ID);
    printf("key: %-20d\n", key);

    size_t tamaño = sizeof(struct datosCompartida);
    
    // Copiamos la memoria compartida
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
    //Memoria circular
    if (datos->numeroEspacio==datos->indiceEmisor){
        datos->indiceEmisor=0;
    }
    
    //Instanciar los txt
    datos->TxtEmisor=fopen("Data/Emisor.txt","r");

    datos->contEmisoresVivos++;
    sem_post(sem_llenos);
    sem_wait(sem_vacios);
    //Mutex

    /////////////////// Zona critica ////////////////////


    // Movemos el puntero del file, al indice deseado. Aqui lo movemos a datos->indiceEmisor
    fseek(datos->TxtEmisor, datos->indiceTxtEmisor, SEEK_SET);
    
    // Get char del puntero del indice del file 
    text = fgetc(datos->TxtEmisor);
    char respuesta= text^clave;
        
    datos->buffer[datos->indiceEmisor] = respuesta;

    datos->indiceEmisor++;
    datos->indiceTxtEmisor++;
    datos->contEmisoresVivos--;
    datos->contEmisoresTotal++;

    time_t tiempo_actual = time(NULL);                    // Obtenemos el tiempo actual en segundos
    struct tm *tiempo_local = localtime(&tiempo_actual);  // Convertimos el tiempo en una estructura tm
    printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);
    

    return 0;
}
