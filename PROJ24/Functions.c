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