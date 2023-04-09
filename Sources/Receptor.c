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

void zonaCritica(struct datosCompartida* d,char clave);

int main(int argc, char *argv[]){

    // Valores recibidos
    char *Modo;
    char *ID;
    int clave;
    int tiempo;

    // Valor del textEmisor
    char text;

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    ID = argv[2];
    clave = atoi(argv[3]);
    tiempo = atoi(argv[4]);

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios, *sem_mutexR;
    sem_llenos = sem_open("/sem_llenos",0);
    sem_vacios = sem_open("/sem_vacios",0);
    sem_mutexR = sem_open("/sem_mutexR", 0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok("tmp", *ID);
    
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
    datos->TxtReceptor=fopen("Data/Receptor.txt","a");
    
    //Modo de uso
    if (strcmp(Modo,"a")==0){
        while (1){
            sleep(tiempo);
            if (datos->endProcess==0){
                datos->contReceptoresVivos++;
                sem_wait(sem_llenos);
                sem_wait(sem_mutexR);
                /////////////////// Zona critica ////////////////////
                zonaCritica(datos, clave);
                /////////////////////////////////////////////////////
                sem_post(sem_mutexR);
                sem_post(sem_vacios);
            }else{
                break;
            }
        }
    }
    char enter;
    if (strcmp(Modo,"m")==0){
        while(1){
            enter = getchar();
             if (enter==13 || enter==10){
                if (datos->endProcess==0){
                    datos->contReceptoresVivos++;
                    sem_wait(sem_llenos);
                    sem_wait(sem_mutexR);
                    
                    /////////////////// Zona critica ////////////////////
                    zonaCritica(datos, clave);
                    /////////////////////////////////////////////////////
                    sem_post(sem_mutexR);
                    sem_post(sem_vacios);
                }else{
                    break;
                }
             }
        }
    }
    return 0;
}


void zonaCritica(struct datosCompartida* datos, char clave) {
    if (datos->indiceReceptor != -1){
        //Memoria circular
        if (datos->numeroEspacio==datos->indiceReceptor){
            datos->indiceReceptor=0;
        }

        // Sacamos el valor del buffer
        char datoBuffer = datos->buffer[datos->indiceReceptor];
        char respuesta = datoBuffer^clave;

        // Write char del puntero del indice del file 
        char text = fputc(respuesta,datos->TxtReceptor);

        //Limpiar el buffer
        datos->buffer[datos->indiceReceptor] = '\0';

        // Obtemos el tiempo
        time_t tiempo_actual = time(NULL);                    // Obtenemos el tiempo actual en segundos
        struct tm *tiempo_local = localtime(&tiempo_actual);  // Convertimos el tiempo en una estructura tm

        // Print elegante
        printf("\n \n");

        printf("| %-15s | %-10s | %-10s | %-5s |\n", "Fecha actual", "Hora actual", "Índice", "Valor ASCII");
        printf("| %02d/%02d/%d      | %02d:%02d:%02d    | %-10d| %-11c |\n",
                tiempo_local->tm_mday, tiempo_local->tm_mon + 1, tiempo_local->tm_year + 1900,
                tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec,
                datos->indiceReceptor, respuesta);

        printf("\n \n");

        //Aumentar los indices
        datos->indiceReceptor++;
        datos->contReceptoresVivos--;
        datos->contReceptoresTotal++;
    }else{
        datos->contReceptoresVivos--;
    }return;
}
