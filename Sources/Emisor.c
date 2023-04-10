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

void zonaCritica(struct datosCompartida* d,char clave, FILE*TxtEmisor,  FILE* logFile);
char* getFechaHora();

int main(int argc, char *argv[]){
    // Valores recibidos
    char *Modo;
    char *ID;
    int clave;
    int tiempo;

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    ID = argv[2];
    clave = atoi(argv[3]);
    tiempo = atoi(argv[4]);

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios, *sem_mutexE;
    sem_llenos = sem_open("/sem_llenos",0);
    sem_vacios = sem_open("/sem_vacios",0);
    sem_mutexE = sem_open("/sem_mutexE", 0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    //key_t key = ftok("Data/shmid.txt", *ID);
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

    //Instanciar los txt
    FILE *TxtEmisor=fopen("Data/Emisor.txt", "r");
    
    // Direccion del archivo que contiene informacion de las operaciones
    //"a+": Si el archivo no existe, lo construye
    FILE* logFile;
    logFile = fopen("Data/log.txt", "a");
    if (logFile == NULL) {
        perror("Error al abrir el log .txt");
        exit(1);
    }

    //Modo de uso
    if (strcmp(Modo,"a")==0){
        while (1){
            sleep(tiempo);
            if (datos->endProcess==0){
                datos->contEmisoresVivos++;
                sem_wait(sem_vacios);
                sem_wait(sem_mutexE);
                /////////////////// Zona critica ////////////////////
                zonaCritica(datos, clave, TxtEmisor, logFile);
                /////////////////////////////////////////////////////
                sem_post(sem_mutexE);
                sem_post(sem_llenos);
                datos->contEmisoresVivos--;
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
                    datos->contEmisoresVivos++;

                    sem_wait(sem_vacios);
                    sem_wait(sem_mutexE);
                    /////////////////// Zona critica ////////////////////
                    zonaCritica(datos, clave, TxtEmisor, logFile);
                    /////////////////////////////////////////////////////
                    sem_post(sem_mutexE);
                    sem_post(sem_llenos);
                    
                    datos->contEmisoresVivos--;
                }else{
                    break;
                }
            }
        }
    }
    fclose(logFile);
    fclose(TxtEmisor);
    return 0;
}

void zonaCritica(struct datosCompartida* datos, char clave, FILE*TxtEmisor, FILE* logFile) {
    if (datos->indiceEmisor != -1){
        //Memoria circular
        if (datos->numeroEspacio==datos->indiceEmisor){
            datos->indiceEmisor=0;
        }
        // Movemos el puntero del file, al indice deseado. Aqui lo movemos a datos->indiceEmisor
        fseek(TxtEmisor, datos->indiceTxtEmisor, SEEK_SET);

        // Get char del puntero del indice del file 
        char text = fgetc(TxtEmisor);
        //XOR
        char respuesta = text^clave;

        datos->buffer[datos->indiceEmisor] = respuesta;

        char* fechaActual = getFechaHora(); // call the function to get the string
        
        // escribir la info en el log file, se escribe una linea al final del archivo
        char infoFormato[] = "%d-%c    | %c           |  %d       | %s \n";
        //Ponemos la info en el Logfile
        fprintf(logFile, infoFormato, getpid(),'E', text, datos->indiceEmisor, fechaActual);

        // Print elegante
        printf("\n");
        
        printf("\033[1;31m"); // Cambiar color del texto a rojo brillante
        printf("| %-21s | %-11s | %-5s |\n", "Fecha-Hora", "Índice", "Valor ASCII");
        printf("| %-15s | %-10d | %-11c |\n", fechaActual, datos->indiceEmisor, text);
        printf("\033[0m"); // Restablecer color del texto a su valor predeterminado
        
        printf("\n");

        fflush(logFile);
        free(fechaActual);

        //Aumentar los indices
        datos->indiceEmisor++;
        datos->indiceTxtEmisor++;
        datos->contEmisoresTotal++;
    }return;
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
    char* result = (char*)malloc(8*sizeof(int));
    sprintf(result, "%04d/%02d/%02d : %d:%02d:%02d", year, mes, dia, hora, mins, secs);
    return result;
}