#include "HeaderFile.h"

FILE *logFile;
sem_t *mutexSemaphore;

void initializeMutexSemaphore() {   //Método responsável por inicializar um semáforo mutex
    sem_unlink("MUTEX");

    mutexSemaphore = sem_open("MUTEX", O_CREAT | O_EXCL, 0766, 1);
}

void initializeLogFile() {   //Método responsável por inicializar o ficheiro de log.
    if ((logFile = fopen("files//logFile.txt", "a+")) == NULL) {
        printf("[CONSOLE] Failed to open log file.");

        exit(1);
    }
}

void writeLogFile(char *strMessage) {   //Método responsável por escrever no ficheiro de log e imprimir na consola sincronizadamente..
    char timeS[10];
    time_t time1 = time(NULL);

    struct tm *time2 = localtime(&time1);
    strftime(timeS, sizeof(timeS), "%H:%M:%S ", time2);
    
    sem_wait(mutexSemaphore);   //Semáforo mutex de forma a que o ficheiro de log seja acedido de maneira exclusiva.
    
    fprintf(logFile, "%s %s\n", timeS, strMessage);
    printf("%s %s\n", timeS, strMessage);
    fflush(logFile);   //Garante que as mensagens sejam imediatamente impressas no ficheiro de log e na consola e não no fim da execução ou quando o buffer se encontrar cheio.
    fflush(stdout);

    sem_post(mutexSemaphore);   //Liberta o semáforo.
}

int readConfigFile(char *fileName) {   //Método responsável por ler o ficheiro de configurações.
    FILE *configFile;
    char *strLine = NULL;
    size_t len = 0;
    ssize_t readLine;
    int lineId = 0;
    int number;
    
    if ((configFile = fopen(fileName, "r")) == NULL) {
        writeLogFile("[SYSTEM] Failed to open config file.");

        return -1;
    }

    while ((readLine = getline(&strLine, &len, configFile)) != -1) {
        if (lineId > 5) {
            break;
        }
        
        switch (lineId) {
            case (0):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Number of Queue Pos is too low.");

                    return -1;
                }

                shMemory -> queuePos = number;
                lineId++;
                
                break;
            
            case (1):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Number of Max Authorization Engines to be lauched is too low.");

                    return -1;
                }

                shMemory -> maxAuthServers = number;
                lineId++;

                break;

            case (2):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Time of Authorization Engine request process is too low.");

                    return -1;
                }

                shMemory -> authProcTime = number;
                lineId++;

                break;

            case (3):
                number = atoi(strLine);
                
                if (number < 1) {
                    writeLogFile("[SYSTEM] Max time of Video Authorization Services request is too low.");

                    return -1;
                }

                shMemory -> maxVideoWait = number;
                lineId++;
                
                break;

            case (4):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Max time of Other Authorization Services request is too low.");

                    return -1;
                }

                shMemory -> maxOthersWait = number;
                lineId++;

                break;
        }
    }
    
    fclose(configFile);

    if (strLine) {
        free(strLine);
    }

    if (lineId < 5) {
        writeLogFile("[SYSTEM] Config File does not have enought information.");
    
        return -1;
    }

    return 1;
}

int createSharedMemory(int shmSize) {   //Método responsável por criar a memória partilhada.
    int shmIdValue = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | 0666);

    if (shmId < 0) {
        writeLogFile("Error creating Shared Memory");

        exit(1);
    }

    return shmIdValue;
}

sharedMemory* attatchSharedMemory(int shmId) {   //Método responsável por atrivuir uma zona (endereço) de memória partilhada.
    char *shmAddr = shmat(shmId, NULL, 0);

    if (shmAddr == (sharedMemory *) - 1) {
        writeLogFile("Shmat error");

        exit(1);
    }

    return shmAddr;
}

void initializeSharedMemory() {   //Método responsável por inicializar a memória partilhada.
    size_t shmSize = sizeof(shMemory) + sizeof(mobileUser) + 1;   //Ver.

    shmId = createSharedMemory(shmSize);
    shMemory = attatchSharedMemory(shmId);

    shMemory -> queuePos = 0;
    shMemory -> maxAuthServers = 0;
    shMemory -> authProcTime = 0;
    shMemory -> maxVideoWait = 0;
    shMemory -> maxOthersWait = 0;

    writeLogFile("SHARED MEMORY INITIALIZED");
}

void createProcess(void (*functionProcess) (void*), void *args) {   //Método responsável por criar um novo processo.
    if (fork() == 0) {   //Cria um novo processo. Se a função fork() retornar 0, todo o código neste bloco será executado no processo filho.
        if (args)   //Verifica se a função possui argumentos.
            functionProcess(args);

        else
            functionProcess(NULL);
    }
}

void authorizationRequestManager() {   //Método responsável por criar o processo Authorization Request Manager.
    writeLogFile("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    //Criar as threads Receiver e Sender.
}

void monitorEngine() {   //Método responsável por criar o processo Monitor Engine.

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "5g_auth_platform {config-file}");

        return -1;
    }

    initializeMutexSemaphore();
    initializeLogFile();

    initializeSharedMemory();

    readConfigFile(argv[1]);   //Lê o ficheiro de configurações passado como parâmetro.

    writeLogFile("5G_AUTH_PLATFORM SIMULATOR STARTING");

    //createProcess(authorizationRequestManager, NULL);

    return 0;
}
