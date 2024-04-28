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
//TODO uncomment this #include <mqueue.h> // #include <fcntl.h> no mac 

#define USER_PIPE "/tmp/userpipe"
#define BACK_PIPE "/tmp/backpipe"

sem_t *mutexSemaphore;
sem_t *shmSemaphore;



/*int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
int maxAuthServers;
int authProcTime;
int maxVideoWait;
int maxOthersWait;*/

typedef struct message {
    long mtype; // Alertas: 10 + user_id ; Stats: 200
    
    // int user_id;
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
    int alert; // TODO wehn changing used data, update alter level [0: <80; 1: 80-90, 2: 90-100; 3: 100]
} mobileUser;

typedef struct sharedMemory {
    mobileUser *mobileUsers;

    int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
    int maxAuthServers;
    int authProcTime;
    int maxVideoWait;
    int maxOthersWait;
    int n_users;
    
    // FOR THE STATS
    int total_video_data;
    int total_music_data;
    int total_social_data;
    int total_video_authreq;
    int total_music_authreq;
    int total_social_authreq;
} sharedMemory;

int shmId;
sharedMemory* shMemory;

int msgq_id;
key_t key;

pthread_t sender_thread;
pthread_t receiver_thread;

int fd_sender_pipes[N_AUTH_ENG][2]; // TODO ha forma de trocar o NAUTHENG pelo numero de auth engines inserido?

void initializeLogFile();
void writeLogFile(char *strMessage);
int readConfigFile(char *fileName);
void backOfficeUserCommands();
int createSharedMemory(int shmSize);
sharedMemory* attatchSharedMemory(int shmId);
void initializeSharedMemory();

void receiver_func();
void sender_func();
void authorization_request_manager_func();
void monitor_engine_func();
int random_number(int min, int max);

void sigint(int signum);

#endif
