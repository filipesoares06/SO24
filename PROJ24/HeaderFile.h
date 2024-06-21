//Filipe Freire Soares 2020238986
//Francisco Cruz Macedo 2020223771

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
//#include <mqueue.h>
#include <sys/select.h>
#include <limits.h>

#define USER_PIPE "/tmp/userpipe"
#define BACK_PIPE "/tmp/backpipe"

#define MAX_PROCESSES 3
#define MAX_ENGINES 6

sem_t *mutexSemaphore;
sem_t *shmSemaphore;
sem_t *videoQueueSemaphore;
sem_t *otherQueueSemaphore;

sem_t *ae_states_semaphore; // counter semaphore for ae engines, initialized at the same time of the shared memory number of auth engines

sem_t *video_counter_sem;
sem_t *other_counter_sem;

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
    int alertAux;
} mobileUser;

typedef struct sharedMemory {
    mobileUser *mobileUsers;
    int ae_state[MAX_ENGINES]; // 0 = FREE; 1 = OCCUPIED


    int pid_counter;
    pid_t pids[MAX_PROCESSES];


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

FILE *logFile;

int shmId;
sharedMemory* shMemory;

pthread_t receiverThread;
pthread_t senderThread;

int msgq_id;
key_t key;

char **videoQueue;   //Queue para video streaming services.
char **otherQueue;   //Queue para other services.

int fd_sender_pipes[N_AUTH_ENG][2];

bool auth_eng_state[N_AUTH_ENG];   //0 = FREE; 1 = OCCUPIED;

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
void addToQueue(char *authOrder);
void authorizationRequestManagerFunction();
void authorizationEngine(int engineId);
void monitorEngineFunction();

char *getFromQueue(char* queue[100], sem_t *queue_sem);
void cleanResources();
void cleanup();
void sigint(int signum);

int random_number(int min, int max);

#endif