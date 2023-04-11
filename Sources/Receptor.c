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

void zonaCritica(struct datosCompartida* d,char clave, FILE* TxtReceptor, FILE* logFile);
char* getFechaHora();

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
    
    size_t tamano = sizeof(struct datosCompartida);
    
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
    FILE* TxtReceptor=fopen("Data/Receptor.txt","a");

    // Direccion del archivo que contiene informacion de las operaciones
    //"a+": Si el archivo no existe, lo construye
    FILE* logFile;
    logFile = fopen("Data/log.txt", "a");
    if (logFile == NULL) {
        perror("Error al abrir el log .txt");
        exit(1);
    }
    datos->contReceptoresTotal++;
    //Modo de uso
    if (strcmp(Modo,"a")==0){
        while (1){
            if (datos->endProcess==0){
                datos->contReceptoresVivos++;
                
                sleep(tiempo);
                sem_wait(sem_llenos);
                sem_wait(sem_mutexR);
                /////////////////// Zona critica ////////////////////
                zonaCritica(datos, clave,TxtReceptor, logFile);
                /////////////////////////////////////////////////////
                sem_post(sem_mutexR);
                sem_post(sem_vacios);
                datos->contReceptoresVivos--;
            }else{
                break;
            }
        }
    }
    char enter;
    if (strcmp(Modo,"m")==0){
        while(1){
            printf("Presione enter para continuar...\n");
            enter = getchar();
            if (enter==13 || enter==10){
                if (datos->endProcess==0){
                    datos->contReceptoresVivos++;
                    sem_wait(sem_llenos);
                    sem_wait(sem_mutexR);
                    /////////////////// Zona critica ////////////////////
                    zonaCritica(datos, clave, TxtReceptor, logFile);
                    /////////////////////////////////////////////////////
                    sem_post(sem_mutexR);
                    sem_post(sem_vacios);
                    datos->contReceptoresVivos--;
                }else{
                    break;
                }
             }
        }
    }
    fclose(TxtReceptor);
    fclose(logFile);
    return 0;
}


void zonaCritica(struct datosCompartida* datos, char clave, FILE* TxtReceptor, FILE* logFile) {
    if (datos->indiceReceptor != -1){
        //Memoria circular
        if (datos->numeroEspacio==datos->indiceReceptor){
            datos->indiceReceptor=0;
        }

        // Sacamos el valor del buffer
        char datoBuffer = datos->buffer[datos->indiceReceptor];
        char respuesta = datoBuffer^clave;

        // Write char del puntero del indice del file 
        char text = fputc(respuesta,TxtReceptor);

        //Limpiar el buffer
        datos->buffer[datos->indiceReceptor] = '\0';

        char* fechaActual = getFechaHora(); // call the function to get the string
        
        // escribir la info en el log file, se escribe una linea al final del archivo
        char infoFormato[] = "%d-%c     | %c           |  %d       | %s \n";

        fprintf(logFile, infoFormato, getpid(),'R', text, datos->indiceReceptor, fechaActual);

        // Print elegante
        printf("\n");

        printf("\033[1;34m"); // Cambiar color del texto a azul brillante
        printf("| %-21s | %-11s | %-5s |\n", "Fecha-Hora", "Índice", "Valor ASCII");
        printf("| %-15s | %-10d | %-11c |\n", fechaActual, datos->indiceReceptor, text);
        printf("\033[0m"); // Restablecer color del texto a su valor predeterminado

        printf("\n");  

        fflush(logFile);
        fflush(TxtReceptor);
        free(fechaActual);

        //Aumentar los indices
        datos->indiceReceptor++;        
    }

    return;
}

char* getFechaHora(){
    time_t tiempo_actual = time(NULL);
    struct tm *tiempo_local = localtime(&tiempo_actual);
    int year = tiempo_local->tm_year + 1900;
    int mes = tiempo_local->tm_mon + 1;
    int dia = tiempo_local->tm_mday;
    int hora = tiempo_local->tm_hour;
    int mins = tiempo_local->tm_min;
    int secs = tiempo_local->tm_sec;
    char* result = (char*)malloc(20*sizeof(int));
    sprintf(result, "%04d/%02d/%02d : %d:%02d:%02d", year, mes, dia, hora, mins, secs);
    return result;
}