//Filipe Freire Soares 2020238986
//Francisco Cruz Macedo 2020223771

#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;

    if (argc != 7) {   //Verifica se os parâmetros estão corretos.
        fprintf(stderr, "mobile_user {plafond inicial} {número de pedidos de autorização} {intervalo VIDEO} {intervalo MUSIC} {intervalo SOCIAL} {dados a reservar}");

        return -1;
    }

    inicialPlafond = atoi(argv[1]);
    numAuthRequests = atoi(argv[2]);
    videoInterval = atoi(argv[3]);
    musicInterval = atoi(argv[4]);
    socialInterval = atoi(argv[5]);
    reservedData = atoi(argv[6]);
    
    pid_t mobileUserId;   //Id do MobileUser, que corresponderá ao PID (Identificador do processo) do processo.
    mobileUserId = getpid();   //Obtem o identificador do processo.

    int fd = open(USER_PIPE, O_WRONLY);   //É aberto o named pipe USER_PIPE para escrita. Named pipe é criado no Authorization Request Manager.
    if(fd == -1) {
        perror("Error while opening USER_PIPE");

        exit(1);
    }

    char registerMessageStr[100];
    snprintf(registerMessageStr, sizeof(registerMessageStr), "%d#%d", mobileUserId, inicialPlafond);

    int fdWrite = write(fd, registerMessageStr, strlen(registerMessageStr) + 1);   //Enviar informação no named pipe.
    if (fdWrite == -1) {
        perror("Error while writing to USER_PIPE");

        exit(1);
    }
    
    struct timespec lastTime, currentTime;
    clock_gettime(CLOCK_MONOTONIC, &lastTime);
    char* availableServices[3] = {"VIDEO", "SOCIAL", "MUSIC"};   //Array que contem todos os serviços disponíveis.

    int requestCounter = 0;   //Gerar mensagens e posteriormente enviar para o named pipe.

    while (requestCounter < numAuthRequests) {   //Ciclo que itera até ao número máximo de pedidos de autorização ser alcançado.
        char authOrderStr[100];

        bool verifyMusicService = false;
        bool verifyVideoService = false;
        bool verifySocialService = false;

        clock_gettime(CLOCK_MONOTONIC, &currentTime);

        long elapsedTime = (currentTime.tv_sec - lastTime.tv_sec) / (CLOCKS_PER_SEC / 1000000);    //Calcula o tempo decorrido desde a última mensagem enviada em ms.
        
        const char *service;   //Determina de que serviço se trata de acordo com o tempo decorrido.
        if (elapsedTime % musicInterval == 0) {   //MUSIC service.
            service = availableServices[2];
            
            snprintf(authOrderStr, sizeof(authOrderStr), "%d#%s#%d", mobileUserId, service, reservedData);
            
            fdWrite = write(fd, authOrderStr, strlen(authOrderStr) + 1);
            if (fdWrite == -1) {
                perror("Error while writing to named pipe USER_PIPE");

                exit(1);
            }

            printf("%s\n", authOrderStr);
            
            lastTime = currentTime;
            usleep(10000);
        }
        
        if (elapsedTime % socialInterval == 0) {   //SOCIAL service.
            service = availableServices[1];

            snprintf(authOrderStr, sizeof(authOrderStr), "%d#%s#%d", mobileUserId, service,reservedData);
            
            fdWrite = write(fd, authOrderStr, strlen(authOrderStr) + 1);
            if (fdWrite == -1) {
                perror("Error while writing to named pipe USER_PIPE");

                exit(1);
            }

            printf("%s\n", authOrderStr);

            lastTime = currentTime;
            usleep(10000);
        } 
        
        if (elapsedTime % videoInterval == 0) {   //VIDEO service.
            service = availableServices[0];

            snprintf(authOrderStr, sizeof(authOrderStr), "%d#%s#%d", mobileUserId, service,reservedData);
            
            fdWrite = write(fd, authOrderStr, strlen(authOrderStr) + 1);
            if (fdWrite == -1) {
                perror("Error while writing to named pipe USER_PIPE");

                exit(1);
            }

            printf("%s\n", authOrderStr);

            lastTime = currentTime;
            usleep(10000);
        } 
        
        else {   //Avança para a próxima iteração caso não tenha decorrido tempo suficiente.
            continue;
        }

        requestCounter++;

        //usleep(5000000);   //Dorme durante 5s (A VM simplesmente não aguenta sem este sleep).
    }

    close(fd);

    return 0;
}
