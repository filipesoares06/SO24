#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;
    int usedData;
    int alert;

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
    usedData = 0;
    alert = 0; // TODO onde estão a ser usados?
    
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

    message r_msg;

    //Gerar mensagens e posteriormente enviar para o named pipe.
    int request_counter = 0;
    while (request_counter < numAuthRequests) {
        char authOrderStr[100];

        // TODO ler da message queue se recebeu alerta de 100%, se sim, termina
        if(msgrcv(msgq_id, &r_msg, 10 + mobileUserId, 0) == -1){
            perror("Error while receiving message");
            writeLogFile("[MU] Error while receiving message");
            exit(1);
        }

        if(strmcp(r_msg.msg, "A#100") == 0){
            // TODO close and exit

            #if DEBUG   
                printf("[MU %d] Exited due to data limit\n", mobileUserId);
                writeLogFile("[MU %d] Exited due to data limit\n", mobileUserId);
            #endif
        }


        #if DEBUG
            printf("%s\n", r_msg.msg);
        #endif


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
        // TODO semaforo para o pipe ?? 
    }


    return 0;
}
