#include "HeaderFile.h"

FILE *logFile;

int queueFrontVideo = 0;
int queueBackVideo = 0;
            
int queueFrontOther = 0;
int queueBackOther = 0;

void initializeMutexSemaphore() {   //Método responsável por inicializar um semáforo mutex
    sem_unlink("MUTEX");
    sem_unlink("SHM_SEM");
    sem_unlink("VQ_SEM");
    sem_unlink("OSQ_SEM");
    sem_unlink("AES_SEM");

    mutexSemaphore = sem_open("MUTEX", O_CREAT | O_EXCL, 0766, 1);
    if (mutexSemaphore == SEM_FAILED) {
        writeLogFile("[SM] MUTEX Semaphore Creation Failed\n");
        exit(1);
    }

    shmSemaphore = sem_open("SHM_SEM", O_CREAT | O_EXCL, 0766, 1);
    if (shmSemaphore == SEM_FAILED) {
        writeLogFile("[SM] Shared Memory Semaphore Creation Failed\n");
        exit(1);
    }

    videoQueueSemaphore = sem_open("VQ_SEM", O_CREAT | O_EXCL, 0766, 1);
    if (videoQueueSemaphore == SEM_FAILED) {
        writeLogFile("[SM] VQ_SEM Creation Failed\n");
        exit(1);
    }

    otherQueueSemaphore = sem_open("OSQ_SEM", O_CREAT | O_EXCL, 0766, 1);
    if (otherQueueSemaphore == SEM_FAILED) {
        writeLogFile("[SM] OSQ_SEM Creation Failed\n");
        exit(1);
    }

    ae_states_semaphore = sem_open("AES_SEM", O_CREAT | O_EXCL, 0766, 1);
    if (ae_states_semaphore == SEM_FAILED) {
        writeLogFile("[SM] AES_SEM Creation Failed\n");
        exit(1);
    }
}

void initializeLogFile() {   //Método responsável por inicializar o ficheiro de log.
    if ((logFile = fopen("files//logFile.txt", "a+")) == NULL) {
        perror("[CONSOLE] Failed to open log file.");

        exit(1);
    }
}

void writeLogFile(char *strMessage) {   //Método responsável por escrever no ficheiro de log e imprimir na consola sincronizadamente..
    char timeS[10];
    time_t time1 = time(NULL);

    struct tm *time2 = localtime(&time1);
    strftime(timeS, sizeof(timeS), "%H:%M:%S ", time2);
    
    sem_wait(mutexSemaphore);   //Semáforo mutex de forma a que o ficheiro de log seja acedido de maneira exclusiva.
    
    fprintf(logFile, "%s %s\n", timeS, strMessage);
    printf("%s %s\n", timeS, strMessage);
    fflush(logFile);   //Garante que as mensagens sejam imediatamente impressas no ficheiro de log e na consola e não no fim da execução ou quando o buffer se encontrar cheio.
    fflush(stdout);

    sem_post(mutexSemaphore);   //Liberta o semáforo.
}

int readConfigFile(char *fileName) {   //Método responsável por ler o ficheiro de configurações.
    FILE *configFile;
    char *strLine = NULL;
    size_t len = 0;
    ssize_t readLine;
    int lineId = 0;
    int number;
    
    if ((configFile = fopen(fileName, "r")) == NULL) {
        writeLogFile("[SYSTEM] Failed to open config file.");

        return -1;
    }

    while ((readLine = getline(&strLine, &len, configFile)) != -1) {
        if (lineId > 6) {
            break;
        }
        
        switch (lineId) {
            case (0):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Number of Mobile Users is too low.");

                    return -1;
                }

                initializeSharedMemory(number);
                lineId++;
                
                break;

            case (1):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Number of Queue Pos is too low.");

                    return -1;
                }

                shMemory -> queuePos = number;
                lineId++;
                
                break;
            
            case (2):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Number of Max Authorization Engines to be lauched is too low.");

                    return -1;
                }

                shMemory -> maxAuthServers = number;
                lineId++;

                break;

            case (3):
                number = atoi(strLine);

                if (number < 0) {
                    writeLogFile("[SYSTEM] Time of Authorization Engine request process is too low.");

                    return -1;
                }

                shMemory -> authProcTime = number;
                lineId++;

                break;

            case (4):
                number = atoi(strLine);
                
                if (number < 1) {
                    writeLogFile("[SYSTEM] Max time of Video Authorization Services request is too low.");

                    return -1;
                }

                shMemory -> maxVideoWait = number;
                lineId++;
                
                break;

            case (5):
                number = atoi(strLine);

                if (number < 1) {
                    writeLogFile("[SYSTEM] Max time of Other Authorization Services request is too low.");

                    return -1;
                }

                shMemory -> maxOthersWait = number;
                lineId++;

                break;
        }
    }
    
    fclose(configFile);

    if (strLine) {
        free(strLine);
    }

    if (lineId < 5) {
        writeLogFile("[SYSTEM] Config File does not have enought information.");
    
        return -1;
    }

    return 1;
}

int createSharedMemory(int shmSize) {   //Método responsável por criar a memória partilhada.
    int shmIdValue = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | 0666);

    if (shmId < 0) {
        writeLogFile("Error creating Shared Memory");

        exit(1);
    }

    return shmIdValue;
}

sharedMemory* attatchSharedMemory(int shmId) {   //Método responsável por atrivuir uma zona (endereço) de memória partilhada.
    char *shmAddr = shmat(shmId, NULL, 0);

    if (shmAddr == (sharedMemory *) - 1) {
        writeLogFile("Shmat error");

        exit(1);
    }

    return shmAddr;
}

void initializeSharedMemory(int n_users) {   //Método responsável por inicializar a memória partilhada.
    size_t shmSize = sizeof(shMemory) + sizeof(mobileUser) * n_users + 1;

    shmId = createSharedMemory(shmSize);
    shMemory = attatchSharedMemory(shmId);

    shMemory -> queuePos = 0;
    shMemory -> maxAuthServers = 0;
    shMemory -> authProcTime = 0;
    shMemory -> maxVideoWait = 0;
    shMemory -> maxOthersWait = 0;
    shMemory -> n_users = n_users;

    shMemory -> totalVideoData = 0;
    shMemory -> totalMusicData = 0;
    shMemory -> totalSocialData = 0;
    shMemory -> totalVideoAuthReq = 0;
    shMemory -> totalMusicAuthReq = 0;
    shMemory -> totalSocialAuthReq = 0;

    writeLogFile("SHARED MEMORY INITIALIZED");
}

void initializeMessageQueue() {   //Método responsável por inicializar a message queue.
    key_t key = ftok("msgfile", 'A');
    int msgq_id = msgget(key, 0666 | IPC_CREAT);
}

void addVideoQueue(char *fdBuffer) {   //Método responsável por adicionar à video streaming queue.
    int aux_qp;
    sem_wait(shmSemaphore);
    aux_qp = shMemory->queuePos;
    sem_post(shmSemaphore);
    
    if ((queueBackVideo + 1) % (aux_qp) == queueFrontVideo) {
        writeLogFile("MESSAGE NOT ADDED: VIDEO STREAMING QUEUE MAX SIZE WAS REACHED");
    } 
    
    else {
        sem_wait(videoQueueSemaphore);   //Semáforo para aceder à video streaming queue.
        strncpy(videoQueue[queueBackVideo], fdBuffer, sizeof(videoQueue[queueBackVideo]));
        sem_post(videoQueueSemaphore);
#if DEBUG
        printf("%s - %s\n", videoQueue[queueBackVideo], fdBuffer);   //Retirar. Apenas verifica se foi adicionar corretamente à queue.
#endif
        queueBackVideo = (queueBackVideo + 1) % (aux_qp);
    }

    
}

void addOtherQueue(char *fdBuffer) {   //Método responsável por adicionar à other services queue.
    int aux_qp;
    sem_wait(shmSemaphore);
    aux_qp = shMemory->queuePos;
    sem_post(shmSemaphore);

    if ((queueBackOther + 1) % aux_qp == queueFrontOther) {
        writeLogFile("MESSAGE NOT ADDED: OTHER SERVICES QUEUE MAX SIZE WAS REACHED");
    } 
    
    else {
        sem_wait(otherQueueSemaphore); //Semáforo para aceder à other services queue.
        strncpy(otherQueue[queueBackOther], fdBuffer, sizeof(otherQueue[queueBackOther]));
        sem_post(otherQueueSemaphore);
#if DEBUG
        printf("%s - %s\n", otherQueue[queueBackOther], fdBuffer);   //Retirar. Apenas verifica se foi adicionar corretamente à queue.
#endif
        queueBackOther = (queueBackOther + 1) % aux_qp;
    }

    
}

char *getFromQueue(char* queue[100], sem_t *queue_sem) {   //Método responsável por retirar da queue.
    sem_wait(queue_sem);
    if (queue[0] == NULL) {
        sem_post(queue_sem);
        return NULL; // Queue is empty
    }

    char *first = queue[0];

    // Reordering
    for (int i = 0; queue[i] != NULL; i++) {
        queue[i] = queue[i + 1];
    }

    sem_post(queue_sem);
    return first;
}

void* receiverFunction() {   //Método responsável por implementar a thread receiver.
    writeLogFile("THREAD RECEIVER CREATED");
    fflush(stdout);
    
    int fdUserPipe = open(USER_PIPE, O_RDONLY | O_NONBLOCK);   //É aberto o named pipe USER_PIPE para leitura. Named pipe é criado no Authorization Request Manager.

    if(fdUserPipe == -1){
        perror("Error while opening BACK_PIPE");
                        
        exit(1);
    }

    int fdBackPipe = open(BACK_PIPE, O_RDONLY | O_NONBLOCK);   //É aberto o named pipe BACK_PIPE para leitura. Named pipe é criado no Authorization Request Manager.

    if(fdBackPipe == -1){
        perror("Error while opening BACK_PIPE");
                        
        exit(1);
    }
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fdUserPipe, &read_fds);
    FD_SET(fdBackPipe, &read_fds);

    int fdMax = (fdUserPipe > fdBackPipe) ? fdUserPipe : fdBackPipe;

    while (1) {
        int activity = select(fdMax + 1, &read_fds, NULL, NULL, NULL);   // Use select() to wait for activity on pipes

        if (activity == -1) {
            perror("Error selecting pipe");

            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(fdUserPipe, &read_fds)) {   //Name pipe USER_PIPE.
            char fdBuffer[PIPE_BUF];

            ssize_t bytesRead = read(fdUserPipe, fdBuffer, sizeof(fdBuffer));
            if (bytesRead == -1) {
                perror("Error reading from USER_PIPE");

                exit(EXIT_FAILURE);
            } 
            
            else if (bytesRead == 0) {   // TODO isto pode dar merda
                close(fdUserPipe);

                fdUserPipe = -1;
            } 
            
            else {
                int n1, n2;
                char serviceToken[128];
                
                if(sscanf(fdBuffer, "%d#%d", &n1, &n2) == 2) {   //Mensagem de registo.
                    // envia mensagem para a otherqueue
                    addOtherQueue(fdBuffer);

                    #if DEBUG
                    printf("registration %s\n", fdBuffer);
                    #endif
                    
                }

                else if(sscanf(fdBuffer, "%d#%s#%d", &n1, &serviceToken, &n2) == 2) {
                    char *serviceType = strtok(serviceToken, "#");
                    int dataService = atoi(strtok(NULL, "#"));

                    if (strcmp(serviceType, "VIDEO") == 0){   //Envia para a video streaming queue.
                        addVideoQueue(fdBuffer);
                        
                        #if DEBUG
                        printf("%s\n", fdBuffer);
                        #endif
                        //fflush(stdout);
                    }

                    else {   //Envia para a other services queue.
                        addOtherQueue(fdBuffer);
                    }
                }

                else {
                    perror("[RT] Wrong user request format");
                }
            }
        }

        if (fdBackPipe != -1 && FD_ISSET(fdBackPipe, &read_fds)) {   //Named pipe BACK_PIPE.
            char fdBuffer[PIPE_BUF];

            ssize_t bytesRead = read(fdBackPipe, fdBuffer, sizeof(fdBuffer));
            if (bytesRead == -1) {
                perror("Error reading from BACK_PIPE");

                exit(EXIT_FAILURE);
            } 
            
            else if (bytesRead == 0) { 
                close(fdBackPipe);

                fdBackPipe = -1;
            } 
            
            else {   //Envia para a other services queue.
                addOtherQueue(fdBuffer);

                printf("%s\n", fdBuffer);
            }
        }
    }

    close(fdUserPipe);
    close(fdBackPipe);

    pthread_exit(NULL);
}

void* senderFunction() {   //Método responsável por implementar a thread sender.
    writeLogFile("THREAD SENDER CREATED");
    fflush(stdout);

    sem_wait(shmSemaphore);
    int numAuthEngines = shMemory -> maxAuthServers;   //Meter semáforo.
    sem_post(shmSemaphore);

    char *queueMessage;

    while (1) {
        queueMessage = getFromQueue(videoQueue, videoQueueSemaphore); // video queue priority
        bool aeFlag = false; // TRUE = FOUND AUTH ENGINE
        
        if (queueMessage != NULL) {
            // Check for available auth engine
            sem_wait(ae_states_semaphore);
            for (int i = 0; i < numAuthEngines; i++) {
                
                if(auth_eng_state[i]){
                    write(fd_sender_pipes[i][1], queueMessage, sizeof(queueMessage));
                }
                if(aeFlag){
                    usleep(100); // small break
                    break;
                }     
            }
            sem_post(ae_states_semaphore);
        }

        // other services queue
        else {
            queueMessage = getFromQueue(otherQueue, otherQueueSemaphore);
            if(queueMessage != NULL){
                // Check for available auth engine
                sem_wait(ae_states_semaphore);
                for (int i = 0; i < numAuthEngines; i++) {
                    if(auth_eng_state[i]){
                        write(fd_sender_pipes[i][1], queueMessage, sizeof(queueMessage));
                    }
                    
                    if(aeFlag)
                        break;
                }
                sem_post(ae_states_semaphore);
            }
            else{
                usleep(100); // small break
                continue; // next iteration 
            }
        }
        usleep(100); // small break
    }
    // pthread_exit(NULL);
}

void initThreads() {   //Método responsável por inicializar as thread receiver e sender.
    pthread_create(&receiverThread, NULL, receiverFunction, NULL);
    pthread_create(&senderThread, NULL, senderFunction, NULL);
}

void authorizationRequestManagerFunction() {   //Método responsável por implementar o authorization request manager.
    writeLogFile("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    if (access(USER_PIPE, F_OK) != -1) {   //Verifica se o named pipe USER_PIPE já existe.
        writeLogFile("NAMED PIPE USER_PIPE IS READY");
    }

    else {
        if (mkfifo(USER_PIPE, 0666) == -1) {   //É criado o named pipe USER_PIPE.
            perror("Error while creating USER_PIPE");

            exit(1);
        }

        writeLogFile("NAMED PIPE USER_PIPE IS READY!");
    }
    
    if (access(BACK_PIPE, F_OK) != -1) {   //Verifica se o named pipe BACK_PIPE já existe.
        writeLogFile("NAMED PIPE BACK_PIPE IS READY");
    }

    else {
        if (mkfifo(BACK_PIPE, 0666) == -1) {   //É criado o named pipe BACK_PIPE.
            perror("Error while creating BACK_PIPE");

            exit(1);
        }

        writeLogFile("NAMED PIPE BACK_PIPE IS READY");
    }

    int queueSize = shMemory -> queuePos;
    videoQueue = malloc(sizeof(char[queueSize][100]));   //Inicializa a video streaming queue.
    otherQueue = malloc(sizeof(char[queueSize][100]));   //Incializa a other services queue.

    pthread_t receiver_id, sender_id;

    initThreads();
    pthread_join(receiver_id, NULL);
    pthread_join(sender_id, NULL);
}

void createProcess(void (*functionProcess) (void*), void *args) {   //Método responsável por criar um novo processo.
    if (fork() == 0) {   //Cria um novo processo. Se a função fork() retornar 0, todo o código neste bloco será executado no processo filho.
        if (args)   //Verifica se a função possui argumentos.
            functionProcess(args);

        else
            functionProcess(NULL);
    }

    wait(NULL);
}

void authorizationRequestManager() {   //Método responsável por criar o processo Authorization Request Manager.
    createProcess(authorizationRequestManagerFunction, NULL);
}

void monitorEngine() {   //Método responsável por criar o processo Monitor Engine.
    createProcess(monitor_engine_func, NULL);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    if (argc != 2) {
        fprintf(stderr, "5g_auth_platform {config-file}");

        return -1;
    }

    initializeMutexSemaphore();

    initializeLogFile();   //Inicializa o logFile.

    writeLogFile("5G_AUTH_PLATFORM SIMULATOR STARTING");

    readConfigFile(argv[1]);   //Lê o ficheiro de configurações passado como parâmetro.

    writeLogFile("PROCESS SYSTEM MANAGER CREATED");

    sem_init(&videoQueueSemaphore, 0, 1);   //Inicializa o semáforo para a video streaming queue.
    sem_init(&otherQueueSemaphore, 0, 1);   //Inicializa o semáforo para a other services queue.

    createProcess(authorizationRequestManager, NULL);

    /* TODO é preciso fazer isto aqui?
        pthread_join(receiver_thread, NULL);
        pthread_join(sender_thread, NULL);

        for(){
            wait(NULL);
        } for each of the processes?
    */

    return 0;
}