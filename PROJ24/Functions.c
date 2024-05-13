#include "HeaderFile.h"

void backOfficeUserCommands()
{ // Método responsável por imprimir os comandos disponíveis no processo BackOfficeUser.
    printf("Available Operations:\n");
    printf("1) data_stats\n");
    printf("2) reset\n");
    printf("ID_backoffice_user#[data_stats | reset]\n\n");
    fflush(stdout);
}

void authorization_engine(int engine_id)
{
    // le mensagens do sender pelo unnamed pipe
    char aux[1024];
    read(fd_sender_pipes[engine_id][0], &aux, sizeof(aux));

    int user_id;
    int s;
    int req_value;
    int n2;
    int n3;
    // REGISTERING
    if (sscanf(aux, "%d#%d", &user_id, &req_value) == 2)
    {
        mobileUser aux_user;
        aux_user.user_id = user_id;
        aux_user.inicialPlafond = n2;
        // aux_user.numAuthRequests = 0; aux_user.videoInterval = 0; aux_user.musicInterval = 0;
        int n_users;
        sem_wait(shmSemaphore);
        n_users = shMemory->n_users;
        sem_post(shmSemaphore);

        for (int i = 0; i < n_users; i++)
        {
            sem_wait(shmSemaphore);
            if (&(shMemory->mobileUsers[i]) == NULL)
            {
                shMemory->mobileUsers[i] = aux_user;
                sem_post(shmSemaphore);
#if DEBUG
                printf("inserted in shm %s\n", fdBuffer);
#endif
                break;
            }
            sem_post(shmSemaphore);
        }
#if DEBUG
        printf("%s\n", fdBuffer);
#endif
    }

    // UPDATING REQUESTS and ALERTS
    else if (sscanf(aux, "%d#%s#%d", &user_id, &s, &req_value) == 3)
    {
        int n_users;
        bool found = false;

        sem_wait(shmSemaphore);
        n_users = shMemory->n_users;
        sem_post(shmSemaphore);

        for (int i = 0; i < n_users; i++)
        {
            sem_wait(shmSemaphore);
            if (shMemory->mobileUsers[i].user_id == user_id && shMemory->mobileUsers[i].usedData + req_value <= shMemory->mobileUsers[i].inicialPlafond)
            {
                shMemory->mobileUsers[i].usedData += req_value;

                if ((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.8 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 0.9)
                {
                    shMemory->mobileUsers[i].alertAux = 1;
                    char alert[40];
                    snprintf(alert, sizeof(alert), "USER %d REACHED 80%% of DATA USAGE\n", user_id);
                    // writeLogFile(alert);
                }

                else if ((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.9 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 1.0)
                {
                    shMemory->mobileUsers[i].alertAux = 2;
                    char alert[40];
                    snprintf(alert, sizeof(alert), "USER %d REACHED 90%% of DATA USAGE\n", user_id);
                    // writeLogFile(alert);
                }

                else if (shMemory->mobileUsers[i].usedData == shMemory->mobileUsers[i].inicialPlafond)
                {
                    shMemory->mobileUsers[i].alertAux = 3;
                    char alert[40];
                    snprintf(alert, sizeof(alert), "USER %d REACHED 100%% of DATA USAGE\n", user_id);
                    // writeLogFile(alert);
                }
            }
            sem_post(shmSemaphore);

            if (found)
                break;
        }
    }

    // FROM BACK_OFFICE
    else if (sscanf(aux, "%d#[^\n]", &user_id, &s) == 2)
    {
        if (strcmp(s, "data_stats"))
        {
            message msg;
            msg.mtype = 200;

            sem_wait(shmSemaphore);
            snprintf(msg.msg, 1024, "STATS (data|aut reqs)\nVIDEO: %d|%d\nMUSIC: %d|%d\nSOCIAL: %d|%d\n",
                    shMemory->totalVideoData, shMemory->totalVideoAuthReq,
                    shMemory->totalMusicData, shMemory->totalMusicAuthReq,
                    shMemory->totalSocialData, shMemory->totalSocialAuthReq);
            sem_post(shmSemaphore);

            key_t key = ftok("msgfile", 'A');
            int msgq_id = msgget(key, 0666 | IPC_CREAT);

            if (msgq_id == -1)
            {
                perror("[AE] Error while opening Message Queue");
                // writeLogFile("[AE] Error while opening Message Queue");
                exit(1);
            }

            
            if (msgsnd(msgq_id, &msg, 1024, 0) == -1)
            {
                perror("[AE] Error while sending message");
                // writeLogFile("[AE] Error while sending message");
                exit(1);
            }
#if DEBUG
            printf("[AE] Stats sent :)\n");
#endif

            // writeLogFile("[AE] Stats Executed");
        }

        else if (strcmp(s, "reset"))
        {
            sem_wait(shmSemaphore);
            shMemory->totalVideoData = 0;
            shMemory->totalVideoAuthReq = 0;
            shMemory->totalMusicData = 0;
            shMemory->totalMusicAuthReq = 0;
            shMemory->totalSocialData = 0;
            shMemory->totalSocialAuthReq = 0;
            sem_post(shmSemaphore);

            // writeLogFile("[AE] Reset executed");
#if DEBUG
            printf("[AE] Reset done :)\n");
#endif

        }
    }

    // ERROR
    else
    {
        perror("[AE] Error - Failed to parse the string\n");

        exit(1);
    }
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
