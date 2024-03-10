#ifndef HEADER_FILE_H
#define HEADER_FILE_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

int queuePos;   //Variáveis que representam os valores do ficheiro de configurações.
int maxAuthServers;
int authProcTime;
int maxVideoWait;
int maxOthersWait;

typedef struct mobileUser {   //Estrutura que representa o Mobile User.
    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;
} mobileUser;

void initializeLogFile();
void writeLogFile(char *strMessage);
int readConfigFile(char *fileName);
bool validateMobileUser(char *inputMobileUser[]);

#endif
