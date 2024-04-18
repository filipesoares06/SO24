#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;

    pid_t mobileUserId;   //Id do MobileUser, que corresponderá ao PID (Identificador do processo) do processo.

    if (argc != 6) {   //Verifica se os parâmetros estão corretos.
        fprintf(stderr, "mobile_user {plafond inicial} {número de pedidos de autorização} {intervalo VIDEO} {intervalo MUSIC} {intervalo SOCIAL} {dados a reservar}");

        return -1;
    }

    inicialPlafond = atoi(argv[1]);
    numAuthRequests = atoi(argv[2]);
    videoInterval = atoi(argv[3]);
    musicInterval = atoi(argv[4]);
    socialInterval = atoi(argv[5]);
    reservedData = atoi(argv[6]);
    
    mobileUserId = getpid();   //Obtem o identificador do processo.

    //Criar named pipe.
    int fd = mkfifo(USER_PIPE, O_WRONLY);
    if(fd == -1){
        perror("Error while opening user_pipe");
        writeLogFile("[MU] Error while opening user_pipe");
        exit(1);
    }

    char registerMessageStr[100];

    snprintf(registerMessageStr, sizeof(registerMessageStr), "%d#%d", mobileUserId, inicialPlafond);
    // writeLogFile(registerMessageStr);

    //Enviar informação no named pipe.
    write(fd, registerMessageStr, strlen(registerMessageStr)+1);

    struct timespec last_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    char* services[3] = {"VIDEO", "SOCIAL", "MUSIC"};

    //Gerar mensagens e posteriormente enviar para o named pipe.
    int request_counter = 0;
    while (request_counter < numAuthRequests) {  // TODO será que deviamos ir buscar o numero de authrequests maximo à shMemory? e podemos atualizar esse valor com --?
        char authOrderStr[100];

        // TODO ler da message queue se recebeu alerta de 100%, se sim, termina

        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // Calculate time elapsed since last message in milliseconds
        long elapsed_ms = (current_time.tv_sec - last_time.tv_sec) * 1000 + (current_time.tv_nsec - last_time.tv_nsec) / 1000000;

        // Update the service type based on elapsed time
        const char *service;
        if (elapsed_ms >= musicInterval) {
            service = services[2]; // MUSIC
            last_time = current_time;
        } else if (elapsed_ms >= socialInterval) {
            service = services[1]; // SOCIAL
            last_time = current_time;
        } else if (elapsed_ms >= videoInterval) {
            service = services[0]; // VIDEO
            last_time = current_time;
        } else {
            // If not enough time has passed, continue to the next iteration
            continue;
        }

        snprintf(authOrderStr, sizeof(authOrderStr), "%d#%s#%d", mobileUserId, service,reservedData);
        write(fd, registerMessageStr, strlen(registerMessageStr)+1);

        print("%s", authOrderStr);

        usleep(1000); // sleeps for 1 ms

        // TODO close pipe
    }


    return 0;
}
