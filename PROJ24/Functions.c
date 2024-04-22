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
    printf("Receiver called\n");
    fflush(stdout);
}

void monitor_engine_func(){
    // TODO usar variavel de estado ou de notificação. sempre que o valor é alterado, ele verifica o plafond / alerta

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
            
            /* TODO reutilizar para o calculo do alerta
            if((currentUsage / initialPlafond) >= 0.8 && (currentUsage / initialPlafond) < 0.9){
                snprintf(alert, sizeof(alert), "USER %d REACHED 80% of DATA USAGE\n", user->user_id);
                writeLogFile(alert);
            }

            if((currentUsage / initialPlafond) >= 0.9 && (currentUsage / initialPlafond) < 1.0){
                snprintf(alert, sizeof(alert), "USER %d REACHED 90% of DATA USAGE\n", user->user_id);
                writeLogFile(alert);
            }

            if(currentUsage == initialPlafond){
                snprintf(alert, sizeof(alert), "USER %d REACHED 100% of DATA USAGE\n", user->user_id);
                writeLogFile(alert);
            }
            */

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

    // TODO clean resources function

    exit(0);
}