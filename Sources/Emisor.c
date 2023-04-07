#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>   //mmap
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <errno.h>
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
    
    char *ID="buffer1";

    // Crear una clave única para la memoria compartida
    key_t key = ftok(MEM_ID, *ID);
   
    // intenta crear la memoria compartida, pero como ya se creo resulta en fd = -1
    int fd = shm_open(MEM_ID, O_RDWR, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }


    // Para calcular tamano de mem shared
    struct stat mem_obj;
    if (fstat(fd, &mem_obj)){
        perror("fstat");
        exit(1);
    }

    int tamano = mem_obj.st_size;

    
    // Asignar la estructura a la memoria compartida

    typedef struct arrayCompartido
    {
        elemento *elementos;
        struct datosCompartida datos;
        
    } arrayCompartido;

    printf("debuggaazo %d ", 4);
    arrayCompartido *recursosCompartidos = mmap(0, sizeof(arrayCompartido), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(recursosCompartidos == MAP_FAILED){
        perror("mmap: no se mapeado la memoria correctamente");
        exit(1);
    }
    recursosCompartidos->datos.contEmisoresVivos++;

    sem_t *sem_llenos, *sem_vacios;
    
    // Crear sem_llenos con nombre "sem_llenos" y un valor inicial de 0
    sem_llenos = sem_open("/sem_llenos", 0);
    sem_vacios = sem_open("/sem_vacios", 0);
    recursosCompartidos->datos.sem_vacios = sem_vacios;
    recursosCompartidos->datos.sem_llenos = sem_llenos;

    // espera por espacio disponible para escribir
    sem_wait(recursosCompartidos->datos.sem_vacios);


    // escribir en buffer compartido
    for (int i = 0; i < 3; i++){
        recursosCompartidos->elementos[i].caracter = 'a';
    }

    //update contadores
    recursosCompartidos->datos.contEmisoresVivos --;
    recursosCompartidos->datos.contEmisoresTotal ++;
    recursosCompartidos->datos.indiceEmisor += 3;
    sem_post(recursosCompartidos->datos.sem_llenos);


    time_t tiempo_actual = time(NULL);                    // Obtenemos el tiempo actual en segundos
    struct tm *tiempo_local = localtime(&tiempo_actual);  // Convertimos el tiempo en una estructura tm
    printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);
    printf("Direccion de memoria de sem_llenos: %p \n", (void *)recursosCompartidos->datos.sem_llenos);
    printf("Direccion de memoria de sem_vacios: %p \n", (void *)recursosCompartidos->datos.sem_vacios);
    return 0;
}
