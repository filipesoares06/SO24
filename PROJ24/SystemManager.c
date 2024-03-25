#include "HeaderFile.h"

FILE *logFile;
sem_t *mutexSemaphore;

void initializeMutexSemaphore() {
    sem_unlink("MUTEX");

    mutexSemaphore = sem_open("MUTEX", O_CREAT | O_EXCL, 0766, 1);
}

void initializeLogFile() {   //Método responsável por inicializar o ficheiro de log.
    if ((logFile = fopen("files//logFile.txt", "a+")) == NULL) {
        puts("[CONSOLE] Failed to open log file.");

        exit(1);
    }
}

void writeLogFile(char *strMessage) {   //Método responsável por escrever no ficheiro de log e imprimir na consola sincronizadamente..
    char timeS[10];
    time_t time1 = time(NULL);
    
    struct tm *time2 = localtime(&time1);
    strftime(timeS, sizeof(timeS), "%H:%M:%S ", time2);

    sem_wait(mutexSemaphore);   //Semáforo mutex de forma a que o ficheiro de log seja acedido de maneiro exclusiva.
    
    fprintf(logFile, "%s %s", timeS, strMessage);
    printf("%s %s", timeS, strMessage);
    fflush(logFile);   //Garante que as mensagens sejam imediatamente impressas no ficheiro de log e na consola e não no fim da execução ou quando o buffer se encontrar cheio.
    fflush(stdout);

    sem_post(mutexSemaphore);
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

                queuePos = number;
                lineId++;
                
                break;
            
            case (1):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Number of Max Authorization Engines to be lauched is too low.");

                    return -1;
                }

                maxAuthServers = number;
                lineId++;

                break;

            case (2):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Time of Authorization Engine request process is too low.");

                    return -1;
                }

                authProcTime = number;
                lineId++;

                break;

            case (3):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Max time of Video Authorization Services request is too low.");

                    return -1;
                }

                maxVideoWait = number;
                lineId++;

                break;

            case (4):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Max tim of Other Authorization Services request is too low.");

                    return -1;
                }

                maxOthersWait = number;
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

void createProcess(void (*functionProcess) (void*), void *args) {   //Método responsável por criar um novo processo.
    if (fork() == 0) {   //Cria um novo processo. Se a função fork() returnar 0, todo o código neste bloco será executado no processo filho.
        if (args)   //Verifica se a função possui argumentos.
            functionProcess(args);

        else
            functionProcess(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "5g_auth_platform {config-file}");

        return -1;
    }

    initializeLogFile();

    readConfigFile(argv[1]);   //Lê o ficheiro de configurações passado como parâmetro.

    return 0;
}
