#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>            // Necesario para O_CREAT y O_EXCL
#include "datosCompartidos.h" // Estructura
#include <unistd.h>


int main(int argc, char *argv[]) {

    // Valores ingresados
    char *ID;
    int clave;
    int numeroEspacio;

    // Obtener los argumentos y convertir el número a entero
    ID = argv[1];
    clave = atoi(argv[2]);
    numeroEspacio = atoi(argv[3]);

    // Para correr la vara varias veces borramos los semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");
    sem_unlink("/sem_mutexE");
    sem_unlink("/sem_mutexR");


    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios, *sem_mutexE, *sem_mutexR;


    // Crear sem_llenos con nombre "sem_llenos" y un valor inicial de 0
    sem_llenos = sem_open("/sem_llenos", O_CREAT | O_EXCL, 0644, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    // Crear sem_llenos con nombre "sem_mutexR" y un valor inicial de 1
    sem_mutexR = sem_open("/sem_mutexR", O_CREAT | O_EXCL, 0644, 1);
    if (sem_mutexR == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    // Crear sem_llenos con nombre "sem_mutexE" y un valor inicial de 1
    sem_mutexE = sem_open("/sem_mutexE", O_CREAT | O_EXCL, 0644, 1);
    if (sem_mutexE == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Crear sem_vacios con nombre "sem_vacios" y un valor inicial de numeroEspacio
    sem_vacios = sem_open("/sem_vacios", O_CREAT | O_EXCL, 0644, numeroEspacio);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok("tmp", *ID);

    // size de la estructura de datos
    size_t strucTamano = sizeof(struct datosCompartida);

    // size del buffer
    size_t bufferTamano = numeroEspacio * sizeof(char);

    // Crear la memoria compartida
    int shmid = shmget(key, strucTamano+bufferTamano, 0666 | IPC_CREAT);
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

    //ReceptorFile
    FILE* textReceptor;
    textReceptor = fopen("Data/Receptor.txt", "w");
    fclose(textReceptor);

    //LogFile
    FILE* logFile;
    // clear the content of the file
    logFile = fopen("Data/log.txt", "w");
    fclose(logFile);
    // write headers to log file
    logFile = fopen("Data/log.txt", "a");
    if (logFile == NULL) {
        perror("Error al abrir el log .txt");
        exit(1);
    }
    char infoFormato[] = "%s    | %s    | %s    | %s    \n";
    fprintf(logFile, infoFormato, "PID-E/R", "Caracter", "Index", "Date");
    fclose(logFile);

    // Asignar valores a la estructura
    datos->clave = clave;
    datos->numeroEspacio = numeroEspacio;
    datos->contEmisoresTotal=0;
    datos->contReceptoresTotal=0;
    datos->contEmisoresVivos=0;
    datos->contReceptoresVivos=0;
    datos->indiceEmisor=0;
    datos->indiceReceptor=0;
    datos->indiceTxtEmisor=0;
    datos->endProcess=0;

    printf("\n");
    printf("\033[1;36m"); // Cambiar color del texto a cyan brillante
    printf("| %-15s | %-10s | %-10s |\n", "ID", "Clave", "Numero de espacios");
    printf("| %-15s | %-10d | %-18d |\n", ID, clave, numeroEspacio);
    printf("\033[0m"); // Restablecer color del texto a su valor predeterminado
    printf("\n");

     //--- debug
    struct shmid_ds segment_info;
    shmctl(shmid, IPC_STAT, &segment_info);
    printf("Current size of shared memory segment: %ld\n", segment_info.shm_segsz);
    printf("\n");
    printf("No. of current attaches: %ld\n", segment_info.shm_nattch);
    printf("\n");
    printf("Owner: %d. My PID: %d\n", segment_info.shm_cpid, getpid());
    /// -----

    // desasignar del segmento compartido
    if (shmdt(datos) == -1) { 
        perror("Error eliminando asignacion del seg compartido");
        return 1;
    }
    //ipcrm -M 0xffffffff

    return 0;
}
