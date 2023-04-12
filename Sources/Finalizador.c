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
    char *ID="buffer1";

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
    int tempEmisores = datos->contEmisoresVivos;
    /*----------------------Captar datos para estadisticas----------------------*/
    
    int estadisticaEmisoresTotal = datos->contEmisoresTotal;
    int estadisticaReceptoresTotal = datos->contReceptoresTotal;
    int estadisticaReceptoresVivos = datos->contReceptoresVivos;
    int estadisticaEmisoresVivos = datos->contEmisoresVivos;
    char *buffer;
    char *bufferActual=datos->buffer;
    FILE *textReceptor;

    //Terminar los receptores en cola
    int flag=1;
    //tempReceptores == datos->contReceptoresVivos
    while (datos->contReceptoresVivos > 0){
        flag=1;
        sem_wait(sem_vacios);
        datos->indiceReceptor = -1;
        sem_post(sem_llenos);
        //Esperar a los N receptores
        while(flag){
            if(tempReceptores != datos->contReceptoresVivos){
                tempReceptores = datos->contReceptoresVivos;
                flag=0;
            }
        }
        if(tempReceptores == 0){
            break;
        }
    }
    //Terminar los emisores en cola
    flag=1;
    while (datos->contEmisoresVivos > 0){
        flag=1;
        sem_wait(sem_llenos);
        datos->indiceEmisor= -1;
        sem_post(sem_vacios);
        //Esperar a los N receptores
        while(flag){
            if(tempEmisores != datos->contEmisoresVivos){

                tempEmisores = datos->contEmisoresVivos;
                flag=0;
            }
        }
        if(tempEmisores==0){
            break;
        }
    }

    //Texto transferido
    textReceptor = fopen("Data/Receptor.txt", "rb");  // Abrir archivo en modo binario
    fseek(textReceptor, 0, SEEK_END);  // Mover el puntero al final del archivo
    long file_size = ftell(textReceptor);   // Obtener la posición actual del puntero, que equivale al tamaño del archivo
    rewind(textReceptor);                   //Nos volvemos al inicio
    buffer = (char *)malloc((file_size + 1) * sizeof(char));  // Reservar memoria para el buffer
    fread(buffer, file_size, 1, textReceptor);  // Leer el contenido del archivo en el buffer
    buffer[file_size] = '\0';         // Agregar el caracter nulo al final del buffer

    //Memoria asignada
    struct shmid_ds segment_info;
    shmctl(shmid, IPC_STAT, &segment_info);

    /*----------------------Imprimimos las estadisticas---------------------*/
    printf("\n");
    printf("\033[0;36m");
    printf("Cantidad de caracteres transferidos:%-26li  \n", file_size);
    printf("Caracteres transferidos al txt:     %-26s  \n", buffer);
    printf("Caracteres en memoria compartida:   %-26s  \n", bufferActual);
    printf("Emisores vivos:                     %-26d  \n", estadisticaEmisoresVivos);
    printf("Emisores totales:                   %-26d  \n", estadisticaEmisoresTotal);
    printf("Receptores vivos:                   %-26d  \n", estadisticaReceptoresVivos);
    printf("Receptores totales:                 %-26d  \n", estadisticaReceptoresTotal);
    printf("Memoria compartida en bytes:        %-26ld  \n", segment_info.shm_segsz);
    printf("\033[0m");
    printf("\n");

    fclose(textReceptor);             // Cerrar el archivo
    free(buffer);                     // Liberar la memoria reservada
    
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