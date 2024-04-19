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

    for(int i = 0; i < shMemory->n_users; i++){
        char* alert[40];
        mobileUser* user = &(shMemory->mobileUsers[i]);
        int currentUsage = user->usedData;
        int initialPlafond = user->inicialPlafond;

        // TODO enviar pela message queue
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

    }

    sleep(30);
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