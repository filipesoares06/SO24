#ifndef HEADER_FILE_H
#define HEADER_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>   //fork()
#include <string.h>
#include <sys/ipc.h>   //Shared memory.
#include <sys/shm.h>

/*int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
int maxAuthServers;
int authProcTime;
int maxVideoWait;
int maxOthersWait;*/

typedef struct mobileUser {   //Estrutura que representa o Mobile User.
    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;
} mobileUser;

typedef struct sharedMemory {
    mobileUser *mobileUsers;

    int n_users;
    int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
    int maxAuthServers;
    int authProcTime;
    int maxVideoWait;
    int maxOthersWait;
} sharedMemory;

int shmId;
sharedMemory* shMemory;

void initializeLogFile();
void writeLogFile(char *strMessage);
int readConfigFile(char *fileName);
void backOfficeUserCommands();
int createSharedMemory(int shmSize);
sharedMemory* attatchSharedMemory(int shmId);
void initializeSharedMemory();

void receiver_func();
void sender_func();

#endif
