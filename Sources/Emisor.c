#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>   //mmap
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>            // Necesario para O_CREAT y O_EXCL
#include "datosCompartidos.h" // Estructura
#include "elemento.h"
#include <time.h>

#define MEM_ID "shared_memory"

int main(int argc, char *argv[]){
    
    printf("debuggaazo %d ", 0);
        // Valores compartidos
    char *Modo;
    int clave;

    // Verificar que se hayan ingresado los 3 argumentos
    if (argc != 3) {
        printf("Uso: programa <string1> <numero1> <numero2>\n");
        return 1;
    }

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    clave = atoi(argv[2]);
    printf("debuggaazo %d ", 1);
    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("/sem_llenos",0);
    sem_vacios = sem_open("/sem_vacios",10);
    printf("debuggaazo %d ", 2);
    if (sem_llenos == SEM_FAILED || sem_vacios == SEM_FAILED){
        perror("sem_open");
        exit(1);
    }

    printf("debuggaazo %d ", 3);
    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    char *ID="buffer1";

    // Crear una clave única para la memoria compartida
    key_t key = ftok(MEM_ID, *ID);

    // Para calcular cantidad de elementos
    struct stat mem_obj;
    int numeroEspacio;

    // intenta crear la memoria compartida, pero como ya se creo resulta en fd = -1
    int fd = shm_open(MEM_ID, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        fd = shm_open(MEM_ID, O_RDWR, S_IRUSR | S_IWUSR);
        fstat(fd, &mem_obj);
        numeroEspacio = ((mem_obj.st_size) - sizeof(struct datosCompartida))/sizeof(elemento);
    }
    
    // Asignar la estructura a la memoria compartida

    typedef struct arrayCompartido
    {
        elemento *elementos;
        struct datosCompartida datos;
        
    } arrayCompartido;

    printf("debuggaazo %d ", 4);
    arrayCompartido *recursosCompartidos = mmap(0, sizeof(arrayCompartido), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    recursosCompartidos->datos.contEmisoresVivos++;

    if (sem_wait(sem_vacios) == -1){
        perror("sem_wait");
        exit(1);
    }

    /* validar que el indice de emisor no exceda buffer
    if (datos->indiceEmisor >= datos->numeroEspacio){
        fprintf(stderr, "Error, indice de emisor (%d) excede el tamano del buffer (%d) \n",
        datos->indiceEmisor, datos->numeroEspacio);
        exit(1);
    }*/

    // escribir en buffer compartido
    int currentIndex = recursosCompartidos->datos.indiceEmisor;
    printf("indice %d ", currentIndex);
    recursosCompartidos->elementos[currentIndex].caracter = 'a';
    recursosCompartidos->datos.indiceEmisor ++;


    //update contadores
    recursosCompartidos->datos.contEmisoresVivos --;
    recursosCompartidos->datos.contEmisoresTotal ++;
    if (sem_post(sem_llenos) == -1){
        perror("sem_post");
        exit(1);
    }

    time_t tiempo_actual = time(NULL);                    // Obtenemos el tiempo actual en segundos
    struct tm *tiempo_local = localtime(&tiempo_actual);  // Convertimos el tiempo en una estructura tm
    printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);

    return 0;
}
