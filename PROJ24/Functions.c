//Filipe Freire Soares 2020238986
//Francisco Cruz Macedo 2020223771

#include "HeaderFile.h"

void backOfficeUserCommands() {   // Método responsável por imprimir os comandos disponíveis no processo BackOfficeUser.
    printf("Available Operations:\n");
    printf("1) data_stats\n");
    printf("2) reset\n");
    printf("ID_backoffice_user#[data_stats | reset]\n\n");
    fflush(stdout);
}

int random_number(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void cleanup() {
    pid_t currentPID = getpid();

    sem_wait(shmSemaphore);
    int counter = shMemory->pid_counter;
    sem_post(shmSemaphore);
    
    sem_wait(shmSemaphore);
    for (int i = 0; i < counter; i++) {
        if (shMemory->pids[i] == currentPID) {
            sem_post(shmSemaphore);
            cleanupResources(i);  // Cleanup resources for the current process
        }
    }
}

void cleanResources(int i) { 
    // i sempre igual a 0 porque só temos um processo
    while (wait(NULL) > 0);

    // Fechar e remover semáforos
    if(i == 0){
        sem_close(mutexSemaphore);
        sem_unlink("mutexSemaphore");

        sem_close(shmSemaphore);
        sem_unlink("shmSemaphore");

        sem_close(videoQueueSemaphore);
        sem_unlink("videoQueueSemaphore");

        sem_close(otherQueueSemaphore);
        sem_unlink("otherQueueSemaphore");

        sem_close(ae_states_semaphore);
        sem_unlink("ae_states_semaphore");
    }

    
    if (i == 0){
        // Fechar e remover pipes
        for (int i = 0; i < N_AUTH_ENG; i++) {
            close(fd_sender_pipes[i][0]);
            close(fd_sender_pipes[i][1]);
        }

        // Remover named pipes
        unlink(BACK_PIPE);
        unlink(USER_PIPE);

        // Remover fila de mensagens
        msgctl(msgq_id, IPC_RMID, NULL);

        sem_wait(shmSemaphore);
        int qs = shMemory->queuePos;
        sem_post(shmSemaphore);

        // Limpar as filas de video e outros serviços
        if (videoQueue != NULL) {
            for(int i = 0; i < qs; i++){
                free(videoQueue[i]);
                videoQueue[i] = NULL;
            }
            free(videoQueue);
            videoQueue = NULL;
        }

        if (otherQueue != NULL) {
            for(int i = 0; i < qs; i++){
                free(otherQueue[i]);
                otherQueue[i] = NULL;
            }
            free(otherQueue);
            otherQueue = NULL;
        }
    }

    if (i == 0){
        // Desanexar e remover a memória compartilhada
        shmdt(shMemory);
        shmctl(shmId, IPC_RMID, NULL);

        // Fechar o arquivo de log
        //fclose(logFile);
    }

    

    printf("SYSTEM SHUTDOWN COMPLETE\n");
}

void sigint(int signum) {   //Método responsável por receber o sinal sigint.
    // writeLogFile("SIGNAL SIGINT RECEIVED");
    // writeLogFile("SIMULATOR WAITING FOR LAST TASKS TO FINISH");

    for (int i = 0; i < 4; i++)
        wait(NULL);   //Espera que os processos acabem e, depois, limpa os recursos.

    cleanup();

    exit(0);
}
