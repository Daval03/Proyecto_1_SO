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

typedef struct estadisticas
{
    int transferidos, enMemCompartida;
    int emisoresVivos, emisoresTotales;
    int receptoresVivos, receptoresTotales;
    
} estadisticas;

void extraerEstadisticas(estadisticas *estadisticasFinales, struct datosCompartida* datos);

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

    datos->endProcess = 1;

    int tempReceptores = datos->contReceptoresVivos;
    int tempEmisores = datos->contEmisoresVivos;

    /*----------------------Captar datos para estadisticas----------------------*/
    
    estadisticas *estadisticasFinales;
    struct shmid_ds memoriaTotal;
    estadisticasFinales->emisoresTotales    = 0;
    estadisticasFinales->emisoresVivos      = 0;
    estadisticasFinales->enMemCompartida    = 0;
    estadisticasFinales->receptoresTotales  = 0;
    estadisticasFinales->receptoresVivos    = 0;
    estadisticasFinales->transferidos       = 0;
    extraerEstadisticas(estadisticasFinales, datos);
    //Memoria total utilizada
    shmctl(shmid, IPC_STAT, &memoriaTotal);
    

    
    /*----------------------Terminar los receptores en cola----------------------*/
    int flag = 1;
   
    while (datos->contReceptoresVivos > 0){
        flag = 1;
        sem_wait(sem_vacios);
        datos->indiceReceptor = -1;
        sem_post(sem_llenos);
        //Esperar a los N receptores
        while(flag){
            if(tempReceptores != datos->contReceptoresVivos){
                printf("\n %d \n", tempReceptores);
                tempReceptores = datos->contReceptoresVivos;
                flag=0;
            }
        }
        if(tempReceptores == 0){
            break;
        }
    }

    /*----------------------Terminar los emisores en cola----------------------*/
    flag = 1;
    while (datos->contEmisoresVivos > 0){
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

    // Cerramos semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");
    sem_unlink("/sem_mutexE");
    sem_unlink("/sem_mutexR");

    /*----------------------Imprimimos las estadisticas---------------------*/
    printf("\n");
    printf("\033[0;36m");
    printf("Caracteres transferidos:            %-25d \n", estadisticasFinales->transferidos);
    printf("Caracteres en memoria compartida:   %-25d \n", estadisticasFinales->enMemCompartida);
    printf("Emisores vivos:                     %-25d \n", estadisticasFinales->emisoresVivos);
    printf("Emisores totales:                   %-25d \n", estadisticasFinales->emisoresTotales);
    printf("Receptores vivos:                   %-25d \n", estadisticasFinales->receptoresVivos);
    printf("Receptores totales:                 %-25d \n", estadisticasFinales->receptoresTotales);
    printf("Memoria compartida en bytes:        %-25ld \n", memoriaTotal.shm_segsz);
    printf("\033[0m");
    printf("\n");
    

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

void extraerEstadisticas(estadisticas *estadisticasFinales, struct datosCompartida* datos)
{   FILE* TextoReceptor=fopen("Data/Receptor.txt","r");
    int buffer_len = datos->numeroEspacio;
    char ch;
    //    caracteres transferidos
    while ((ch=fgetc(TextoReceptor)) != EOF){
        estadisticasFinales->transferidos ++;
    }
    fclose(TextoReceptor);
    
    //    cantidad de caracteres en memoria compartida
    for (int i = 0; i < buffer_len; i++)
    { if (datos->buffer[i] != '\0') estadisticasFinales->enMemCompartida++; }

    //    cantidad de emisores vivos y totales
    estadisticasFinales->emisoresVivos = datos->contEmisoresVivos; 
    estadisticasFinales->emisoresTotales = datos->contEmisoresTotal;

    //    cantidad de receptores vivos y totales
    estadisticasFinales->receptoresVivos = datos->contReceptoresVivos; 
    estadisticasFinales->receptoresTotales = datos->contReceptoresTotal;
    
    return;
}

