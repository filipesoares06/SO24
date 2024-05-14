#include "HeaderFile.h"

void backOfficeUserCommands()
{ // Método responsável por imprimir os comandos disponíveis no processo BackOfficeUser.
    printf("Available Operations:\n");
    printf("1) data_stats\n");
    printf("2) reset\n");
    printf("ID_backoffice_user#[data_stats | reset]\n\n");
    fflush(stdout);
}

void monitor_engine_func()
{
    key_t key = ftok("msgfile", 'A');
    int msgq_id = msgget(key, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error while opening Message Queue");
        // writeLogFile("[ME] Error while opening Message Queue");
        exit(1);
    }

    int n_users;
    sem_wait(shmSemaphore);
    n_users = shMemory->n_users;
    sem_post(shmSemaphore);

    message msg;

    while (1)
    {
        for (int i = 0; i < n_users; i++)
        {
            char alert[40];

            sem_wait(shmSemaphore);
            mobileUser *user = &(shMemory->mobileUsers[i]);
            sem_post(shmSemaphore);

            int currentUsage = user->usedData;
            int initialPlafond = user->inicialPlafond;

            if (user->alertAux != 0)
            {
                // msg.user_id = user->user_id;
                msg.mtype = 10 + user->user_id;

#if DEBUG
                printf("[DEBUG] 10 + %d -> %d\n", user->user_id, msg.mtype);
#endif

                switch (user->alertAux)
                {
                case 1:
                    snprintf(alert, sizeof(alert), "USER %d REACHED 80%% of DATA USAGE\n", user->user_id);
                    // writeLogFile(alert);
                    snprintf(msg.msg, 10, "A#80");
                    break;
                case 2:
                    snprintf(alert, sizeof(alert), "USER %d REACHED 90%% of DATA USAGE\n", user->user_id);
                    // writeLogFile(alert);
                    snprintf(msg.msg, 10, "A#90");
                    break;
                case 3:
                    snprintf(alert, sizeof(alert), "USER %d REACHED 100%% of DATA USAGE\n", user->user_id);
                    // writeLogFile(alert);
                    snprintf(msg.msg, 10, "A#100");
                    break;
                }

                if (msgsnd(msgq_id, &msg, 1024, 0) == -1)
                {
                    perror("Error while sending message");
                    // writeLogFile("[ME] Error while sending message");
                    exit(1);
                }

#if DEBUG
                char *text = NULL;
                snprintf(text, 1024, "sent msgq alert for user %d \n", user->user_id);
                // writeLogFile(text);
                free(text);
#endif
            }
        }
    }

    // TODO fazer a cada 30s enviar stats
}

int random_number(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void cleanResources()
{ // Método responsável por terminar o programa e limpar todos os recursos.
    // writeLogFile("SYSTEM SHUTTING DOWN");

    wait(NULL);

    sem_close(mutexSemaphore);
    sem_unlink("");

    sem_close(shmSemaphore);
    sem_unlink("");

    shmdt(shMemory);
    shmctl(shmId, IPC_RMID, NULL);

    for (int i = 0; i < N_AUTH_ENG; i++)
    { // TODO close pipes before unlinks
        close(fd_sender_pipes[i][0]);

        close(fd_sender_pipes[i][1]);
    }

    // close();
    unlink(BACK_PIPE);

    // close();
    unlink(USER_PIPE);

    msgctl(msgget(ftok("msgfile", 'A'), 0666 | IPC_CREAT), IPC_RMID, NULL);

    // TODO fechar log
}

void sigint(int signum)
{ // Método responsável por receber o sinal sigint.
    // writeLogFile("SIGNAL SIGINT RECEIVED");
    // writeLogFile("SIMULATOR WAITING FOR LAST TASKS TO FINISH");

    for (int i = 0; i < 4; i++)
        wait(NULL); // Espera que os processos acabem e, depois, limpa os recursos.

    cleanResources();

    exit(0);
}