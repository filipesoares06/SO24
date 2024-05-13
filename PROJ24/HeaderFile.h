#ifndef HEADER_FILE_H
#define HEADER_FILE_H
#define N_AUTH_ENG 6
#define DEBUG 0

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
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <sys/select.h>
#include <limits.h>

#define USER_PIPE "/tmp/userpipe"
#define BACK_PIPE "/tmp/backpipe"

sem_t *mutexSemaphore;
sem_t *shmSemaphore;
sem_t *videoQueueSemaphore;
sem_t *otherQueueSemaphore;

typedef struct message {   //Estrutura que representa uma mensagem a enviar para o named pipe USER_PIPE com os pedidos de autorização.
    long mtype; // Alertas: 10 + user_id ; Stats: 200
    char msg[1024];
} message;

typedef struct mobileUser {   //Estrutura que representa o Mobile User.
    int user_id;
    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;
    int usedData;
    int alertAux; // TODO wehn changing used data, update alter level [0: <80; 1: 80-90, 2: 90-100; 3: 100]
} mobileUser;

typedef struct sharedMemory {
    mobileUser *mobileUsers;

    int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
    int maxAuthServers;
    int authProcTime;
    int maxVideoWait;
    int maxOthersWait;
    int n_users;
    
    int totalVideoData;   //Variáveis que representam os valores das estatísticas.
    int totalMusicData;
    int totalSocialData;
    int totalVideoAuthReq;
    int totalMusicAuthReq;
    int totalSocialAuthReq;
} sharedMemory;

int shmId;
sharedMemory* shMemory;

pthread_t receiverThread;
pthread_t senderThread;

int msgq_id;
key_t key;

int fd_sender_pipes[N_AUTH_ENG][2]; // TODO ha forma de trocar o NAUTHENG pelo numero de auth engines inserido?

char (*videoQueue)[100];   //Queue para video streaming services.
char (*otherQueue)[100];   //Queue para other services.

bool auth_eng_state[N_AUTH_ENG];

void initializeLogFile();
void writeLogFile(char *strMessage);
int readConfigFile(char *fileName);
void backOfficeUserCommands();
int createSharedMemory(int shmSize);
sharedMemory* attatchSharedMemory(int shmId);
void initializeSharedMemory();
void* receiverFunction();
void* senderFunction();
void initThreads();
void authorizationRequestManagerFunction();
void addVideoQueue(char *fdBuffer);
void addOtherQueue(char *fdBuffer);
void cleanResources();

void monitor_engine_func();
int random_number(int min, int max);

void sigint(int signum);

#endif