#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>   //mmap
#include <sys/stat.h>   //mode_t
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>  //ftruncate
#include <errno.h>
#include <fcntl.h>            // Necesario para O_CREAT y O_EXCL
#include "datosCompartidos.h" // Estructura de vars compartidos
#include "elemento.h"         // Abstraccion de char y datos individuales
#include <time.h>

#define MEM_ID "shared_memory"
int main(int argc, char *argv[]) {

    // Valores ingresados
    char *ID;
    int clave;
    int numeroEspacio;

    // Verificar que se hayan ingresado los 3 argumentos
    if (argc != 4) {
        printf("Uso: programa <string1> <numero1> <numero2>\n");
        return 1;
    }

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


    // Crear una clave única para la memoria compartida
    key_t key = ftok(MEM_ID, *ID);
    
    int tamano = sizeof(struct datosCompartida) + (sizeof(struct elemento)*numeroEspacio);

    // Crear la memoria compartida
    int fd = shm_open(MEM_ID, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    // Asignar la estructura a la memoria compartida
    if (ftruncate(fd, tamano) == -1){
        perror("ftruncate");
        exit(1);
    }


    typedef struct arrayCompartido
    {
        elemento *elementos;
        struct datosCompartida datos;
        
    } arrayCompartido;

    // Inicializamos esta memoria compartida
    arrayCompartido *recursosCompartidos = mmap(0, tamano, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Asignar valores a la estructura
    recursosCompartidos->elementos                  = malloc(numeroEspacio*sizeof(elemento));
    //recursosCompartidos->elementos[0].caracter      = 'f';
    recursosCompartidos->datos.contEmisoresTotal    = 0;
    recursosCompartidos->datos.contReceptoresTotal  = 0;
    recursosCompartidos->datos.contEmisoresVivos    = 0;
    recursosCompartidos->datos.contReceptoresVivos  = 0;
    recursosCompartidos->datos.indiceEmisor         = 0;
    recursosCompartidos->datos.indiceReceptor       = 0;
    recursosCompartidos->datos.indiceTxtEmisor      = 0;
    recursosCompartidos->datos.indiceTxtReceptor    = 0;
    recursosCompartidos->datos.clave                = clave;
    recursosCompartidos->datos.sem_llenos           = sem_llenos;
    recursosCompartidos->datos.sem_vacios           = sem_vacios;

    printf("\n");
    printf("ID: %-20s\n", ID);
    printf("Clave:%-20d\n",recursosCompartidos->datos.clave);
    printf("Numero de espacios reservados:%-20d\n",numeroEspacio );
    //printf("Primer dato escrito :%c \n",recursosCompartidos->elementos[0].caracter);
    printf("Direccion de memoria de sem_llenos: %p \n", (void *)recursosCompartidos->datos.sem_llenos);
    printf("Direccion de memoria de sem_vacios: %p \n", (void *)recursosCompartidos->datos.sem_vacios);

    printf("\n");

    return 0;
}

/**
cd /dev/shm/ 
ls
rm shared_memory
rm sem.sem_llenos
rm sem.sem_vacios
*/