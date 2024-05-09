#include "HeaderFile.h"

void backOfficeUserCommands() {   //Método responsável por imprimir os comandos disponíveis no processo BackOfficeUser.
    printf("Available Operations:\n");
    printf("1) data_stats\n");
    printf("2) reset\n");
    printf("ID_backoffice_user#[data_stats | reset]\n\n");
    fflush(stdout);
}

// temp functions maybe
void receiver_func(){
    printf("Receiver called\n");
    fflush(stdout);
}

void sender_func(){
    printf("Sender called\n");
    fflush(stdout);
}

void authorization_engine(int engine_id){
    // le mensagens do sender pelo unnamed pipe
    char aux[1024];
    read(fd_sender_pipes[engine_id][0], &aux, sizeof(aux));

    int user_id; int req_value;
    if (sscanf(aux, "%d#%d", &user_id, &req_value) != 2) {
        writeLogFile("[AE] Error - Failed to parse the string");
        perror("[AE] Error - Failed to parse the string\n");
        return 1;
    }

    int n_users; bool found = false;

    sem_wait(shmSemaphore);
    n_users = shMemory->n_users;
    sem_post(shmSemaphore);

    
    for(int i = 0; i < n_users; i++){
        sem_wait(shmSemaphore);      
        // TODO que valor usar aqui?? initialPlafond ou reservedData?  
        if(shMemory->mobileUsers[i].user_id == user_id && shMemory->mobileUsers[i].usedData + req_value <= shMemory->mobileUsers[i].inicialPlafond){
            shMemory->mobileUsers[i].usedData += req_value;
    
            if((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.8 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 0.9){
                shMemory->mobileUsers[i].alert = 1;
                char* alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 80% of DATA USAGE\n", user_id);
                writeLogFile(alert);
            }

            else if((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.9 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 1.0){
                shMemory->mobileUsers[i].alert = 2;
                char* alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 90% of DATA USAGE\n", user_id);
                writeLogFile(alert);
            }

            else if(shMemory->mobileUsers[i].usedData == shMemory->mobileUsers[i].inicialPlafond){
                shMemory->mobileUsers[i].alert = 3;
                char* alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 100% of DATA USAGE\n", user_id);
                writeLogFile(alert);
            }
        }
        sem_post(shmSemaphore);
        
        if(found)
            break; 
    }

    // --- 
    char back_msg[40];
    int fd = open(BACK_PIPE, O_RDONLY);
    if (fd == -1){
        perror("Error while opening user_pipe");
        writeLogFile("[AE] Error while opening back_pipe");
        exit(1);
    }
    
    if(read(fd, back_msg, sizeof(back_msg)) == -1){
        perror("Error reading from named pipe");
        writeLogFile("[AE] Error reading from named pipe");
        exit(1);
    }

    close(fd);
    int integer_part;
    char string_part[20];

    if (sscanf(back_msg, "%d#%[^\n]", &integer_part, string_part) != 2) {
        printf("Error parsing message\n");
        exit(1);
    }

    if(strcmp(string_part, "data_stats")){
        message msg;
        msg.mtype = 200;
        
        sem_wait(shmSemaphore);
        snprintf(msg.msg, 1024, "STATS (data|aut reqs)\nVIDEO: %d|%d\nMUSIC: %d|%d\nSOCIAL: %d|%d\n", 
        shMemory->total_video_data, shMemory->total_video_authreq, 
        shMemory->total_music_data, shMemory->total_music_authreq, 
        shMemory->total_social_data, shMemory->total_social_authreq);
        sem_post(shmSemaphore);

        key_t key = ftok("msgfile", 'A');
        int msgq_id = msgget(key, 0666 | IPC_CREAT);

        if(msgq_id == -1){
            perror("Error while opening Message Queue");
            writeLogFile("[AE] Error while opening Message Queue");
            exit(1);
        }

        if(msgsnd(msgq_id, &msg, 1024, 0) == -1){
            perror("Error while sending message");
            writeLogFile("[AE] Error while sending message");
            exit(1);
        }

        writeLogFile("[AE] Stats Executed");
    }

    else if(strcmp(string_part, "reset")){
        sem_wait(shmSemaphore);
        shMemory->total_video_data = 0;
        shMemory->total_video_authreq = 0;
        shMemory->total_music_data = 0;
        shMemory->total_music_authreq = 0; 
        shMemory->total_social_data = 0;
        shMemory->total_social_authreq = 0;
        sem_post(shmSemaphore);

        writeLogFile("[AE] Reset executed");
    }
}

void monitor_engine_func(){
    key_t key = ftok("msgfile", 'A');
    int msgq_id = msgget(key, 0666 | IPC_CREAT);

    if(msgq_id == -1){
        perror("Error while opening Message Queue");
        writeLogFile("[ME] Error while opening Message Queue");
        exit(1);
    }

    int n_users;
    sem_wait(shmSemaphore);
    n_users = shMemory->n_users;
    sem_post(shmSemaphore);
    

    message msg;

    while(1){
        for(int i = 0; i < n_users; i++){
            char* alert[40];

            sem_wait(shmSemaphore);
            mobileUser* user = &(shMemory->mobileUsers[i]);
            sem_post(shmSemaphore);

            int currentUsage = user->usedData;
            int initialPlafond = user->inicialPlafond;


            if(user->alert != 0){
                // msg.user_id = user->user_id;
                msg.mtype = 10 + user->user_id;

                #if DEBUG
                    printf("[DEBUG] 10 + %d -> %d\n", user->user_id, msg.mtype);
                #endif

                switch(user->alert){
                    case 1:
                        snprintf(alert, sizeof(alert), "USER %d REACHED 80% of DATA USAGE\n", user->user_id);
                        writeLogFile(alert);
                        snprintf(msg.msg, 10, "A#80");
                        break;
                    case 2:
                        snprintf(alert, sizeof(alert), "USER %d REACHED 90% of DATA USAGE\n", user->user_id);
                        writeLogFile(alert);
                        snprintf(msg.msg, 10, "A#90");
                        break;
                    case 3:
                        snprintf(alert, sizeof(alert), "USER %d REACHED 100% of DATA USAGE\n", user->user_id);
                        writeLogFile(alert);
                        snprintf(msg.msg, 10, "A#100");
                        break;
                }

                if(msgsnd(msgq_id, &msg, 1024, 0) == -1){
                    perror("Error while sending message");
                    writeLogFile("[ME] Error while sending message");
                    exit(1);
                }

                #if DEBUG
                    char* text = NULL;
                    snprintf(text, 1024, "sent msgq alert for user %d \n", user->user_id);
                    writeLogFile(text);
                    free(text);
                #endif
            }
            
            

        }
    }

    // TODO fazer a cada 30s enviar stats
}

void authorization_request_manager_func(){
    writeLogFile("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    pthread_t receiver_id, sender_id;

    pthread_create(&receiver_id, NULL, receiver_func, NULL);
    pthread_create(&sender_id, NULL, sender_func, NULL);

    int fd = mkfifo(USER_PIPE, O_RDONLY);
    if(fd == -1){
        perror("Error while opening user_pipe");
        writeLogFile("[ARM] Error while opening user_pipe");
        exit(1);
    }

    close(fd);
    pthread_join(receiver_id, NULL);
    pthread_join(sender_id, NULL);
}

int random_number(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void sigint(int signum)
{
    write_log("SIGNAL SIGINT RECEIVED");
    write_log("SIMULATOR WAITING FOR LAST TASKS TO FINISH");

    for (int i = 0; i < 4; i++)
        wait(NULL); // espera que os processos acabem e, depois, limpa os recursos.

    clean_resources();

    exit(0);
}

void clean_resources(){
    writeLogFile("SYSTEM SHUTTING DOWN");

    wait(NULL);

    sem_close(mutexSemaphore);
    sem_unlink("");

    sem_close(shmSemaphore);
    sem_unlink("");

    shmdt(shMemory);
    shmctl(shmId, IPC_RMID, NULL);

    //TODO close pipes before unlinks
    for(int i = 0; i < N_AUTH_ENG; i++){
        close(fd_sender_pipes[i][0]);
        close(fd_sender_pipes[i][1]);
    }

    // close();
    unlink(BACK_PIPE);

    // close();
    unlink(USER_PIPE);

    msgctl(msgget(ftok("msgfile", 'A'), 0666 | IPC_CREAT), IPC_RMID, NULL);

    //TODO fechar log
}