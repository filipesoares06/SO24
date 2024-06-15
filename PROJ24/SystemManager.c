//Filipe Freire Soares 2020238986
//Francisco Cruz Macedo 2020223771

#include "HeaderFile.h"

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

void writeLogFile(char *strMessage) {   //Método responsável por escrever no ficheiro de log e imprimir na consola sincronizadamente.
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

sharedMemory* attatchSharedMemory(int shmId) {   //Método responsável por atribuir uma zona (endereço) de memória partilhada.
    char *shmAddr = shmat(shmId, NULL, 0);

    if (shmAddr == (char *) - 1) {
        writeLogFile("Shmat error");

        exit(1);
    }

    return (sharedMemory *) shmAddr;
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

    shMemory->mobileUsers = (mobileUser *)((char *)shMemory + sizeof(sharedMemory));

    for (int i = 0; i < n_users; i++) {
        shMemory->mobileUsers[i].user_id = 0;
        shMemory->mobileUsers[i].inicialPlafond = 0;
        shMemory->mobileUsers[i].numAuthRequests = 0;
        shMemory->mobileUsers[i].videoInterval = 0;
        shMemory->mobileUsers[i].musicInterval = 0;
        shMemory->mobileUsers[i].socialInterval = 0;
        shMemory->mobileUsers[i].reservedData = 0;
        shMemory->mobileUsers[i].usedData = 0;
        shMemory->mobileUsers[i].alertAux = 0;
    }

    writeLogFile("SHARED MEMORY INITIALIZED");
}

void createProcess(void (*functionProcess) (void*), void *args) {   //Método responsável por criar um novo processo. 
    pid_t pid = fork();   //Cria um novo processo.

    if (pid == -1) {   //Erro ao criar o processo.
        perror("Error while forking");

        exit(1);
    }
    
    else if (pid == 0) {   //Código executado no processo filho.
        if (args) {   
            functionProcess(args);
            free(args);
            exit(0);
        }

        else {
            functionProcess(NULL);
        }
    }
}

void initializeMessageQueue() {   //Método responsável por inicializar a message queue.
    /*
    key_t key = ftok("msgfile", 'A');

    int msgq_id = msgget(key, 0666 | IPC_CREAT);
    */
}

int addVideoQueue(char *fdBuffer) {   //Método responsável por adicionar à video streaming queue.
    int auxQp;

    sem_wait(shmSemaphore);   //Semáforo para aceder à shared memory.
    auxQp = shMemory->queuePos;
    sem_post(shmSemaphore);
    
    if ((queueBackVideo + 1) % (auxQp) == queueFrontVideo) {
        writeLogFile("MESSAGE NOT ADDED: VIDEO STREAMING QUEUE MAX SIZE WAS REACHED");

        return 1;
    } 
    
    else {
        sem_wait(videoQueueSemaphore);   //Semáforo para aceder à video streaming queue.
        strncpy(videoQueue[queueBackVideo], fdBuffer, sizeof(videoQueue[queueBackVideo]));
        sem_post(videoQueueSemaphore);

        //printf("%s - %s\n", videoQueue[queueBackVideo], videoQueue[0]);   //Retirar. Apenas verifica se foi adicionar corretamente à queue.
        //fflush(stdout);

        queueBackOther = (queueBackOther + 1) % (auxQp + 1);
    }

    return 0;
}

int addOtherQueue(char *fdBuffer) {   //Método responsável por adicionar à other services queue.
    int auxQp;
    
    sem_wait(shmSemaphore);   //Semáforo para aceder à shared memory.
    auxQp = shMemory->queuePos;
    sem_post(shmSemaphore);
    
    if ((queueBackOther + 1) % auxQp == queueFrontOther) {
        writeLogFile("MESSAGE NOT ADDED: OTHER SERVICES QUEUE MAX SIZE WAS REACHED");

        return 1;
    } 
    
    else {
        sem_wait(otherQueueSemaphore);   //Semáforo para aceder à other services queue.
        strncpy(otherQueue[queueBackOther], fdBuffer, sizeof(otherQueue[queueBackOther]));
        sem_post(otherQueueSemaphore);

        //printf("%s - %s\n", otherQueue[queueBackOther], otherQueue[0]);   //Retirar. Apenas verifica se foi adicionar corretamente à queue.
        //fflush(stdout);

        queueBackOther = (queueBackOther + 1) % (auxQp + 1);
    }

    return 0;
}

char *getFromQueue(char* queue[100], sem_t *queue_sem) {   //Método responsável por retirar da queue.
    sem_wait(queue_sem);

    if (queue[0] == NULL) {   //A queue encontra-se vazia.
        sem_post(queue_sem);

        return NULL;
    }

    char *first = queue[0];

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

    int fdMax = (fdUserPipe > fdBackPipe) ? fdUserPipe : fdBackPipe;

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fdUserPipe, &read_fds);
        FD_SET(fdBackPipe, &read_fds);

        int activity = select(fdMax + 1, &read_fds, NULL, NULL, NULL);   // Use select() to wait for activity on pipes

        int verifyVideoAdd = 0;
        //int verifyOtherAdd = 0;

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
            
            else if (bytesRead == 0) {
                close(fdUserPipe);

                fdUserPipe = -1;
            }
            
            else {
                int n1, n2;
                char serviceToken[128];
                
                if(sscanf(fdBuffer, "%d#%d", &n1, &n2) == 2) {   //Mensagem de registo.
                    // TODO registar user. Guardar valores na shared memory.
                    printf("%s\n", fdBuffer);
                }
                
                else if(sscanf(fdBuffer, "%d#%s#%d", &n1, serviceToken, &n2) == 2) {
                    char *serviceType = strtok(serviceToken, "#");
                    //int dataService = atoi(strtok(NULL, "#"));
                    
                    if (strcmp(serviceType, "VIDEO") == 0){   //Envia para a video streaming queue.
                        verifyVideoAdd = addVideoQueue(fdBuffer);
                        
                        if (verifyVideoAdd == 1) {
                            usleep(10000);
                        }

                        //printf("%s\n", fdBuffer);
                        //fflush(stdout);
                    }

                    else {   //Envia para a other services queue.
                        //verifyOtherAdd = addOtherQueue(fdBuffer);

                        if (verifyVideoAdd == 1) {
                            usleep(10000);
                        }

                        //printf("%s\n", fdBuffer);
                        //fflush(stdout);
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
    
    pthread_exit(NULL);
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

    for(int i = 0; i < N_AUTH_ENG; i++) {
        if(pipe(fd_sender_pipes[i]) == -1) {
            perror("Error while creating sender's unnamed pipe");

            exit(1);
        }

        writeLogFile("UNNAMED PIPE IS READY");
    }

    sem_wait(shmSemaphore);
    int queueSize = shMemory -> queuePos;
    int authEngines = shMemory -> maxAuthServers;
    sem_post(shmSemaphore);

    videoQueue = malloc(sizeof(char[queueSize][100]));   //Inicializa a video streaming queue.
    otherQueue = malloc(sizeof(char[queueSize][100]));   //Incializa a other services queue.

    initThreads();

    for (int i = 0; i < authEngines; i++) {
        int* authEngineId = malloc(sizeof(int));
        *authEngineId = i;
        printf("Imprime aqui\n");
        createProcess(authorizationEngineWrapper, authEngineId);
    }
    
    for (int i = 0; i < authEngines; i++) {   //Espera que todos os processos filhos terminem.
        wait(NULL);
    }

    pthread_join(receiverThread, NULL);
    pthread_join(senderThread, NULL);
}

void authorizationEngineWrapper(void *arg) {   //Permite converter o pointeiro void para int.
    int authEngineId = *(int *) arg;

    authorizationEngine(authEngineId);
}

void authorizationEngine(int engineId) {   //Método responsável por implementar o Authorization Engine.
    char initializeMessage[32];
    snprintf(initializeMessage, sizeof(initializeMessage), "AUTHORIZATION_ENGINE %d READY", engineId);

    writeLogFile(initializeMessage);
    fflush(stdout);
}

void monitorEngineFunction() {
    writeLogFile("PROCESS MONITOR_ENGINE CREATED");
    fflush(stdout);
}

void authorizationRequestManager() {   //Método responsável por criar o processo Authorization Request Manager.
    createProcess(authorizationRequestManagerFunction, NULL);
}

void monitorEngine() {   //Método responsável por criar o processo Monitor Engine.
    createProcess(monitorEngineFunction, NULL);
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

    sem_wait(shmSemaphore);
    int authEngines = shMemory -> maxAuthServers;
    sem_post(shmSemaphore);

    for (int i = 0; i < authEngines; i++) {
        auth_eng_state[i] = true;
    }

    //sem_init(&videoQueueSemaphore, 0, 1);   //Inicializa o semáforo para a video streaming queue.
    //sem_init(&otherQueueSemaphore, 0, 1);   //Inicializa o semáforo para a other services queue.

    authorizationRequestManager();

    /*
        pthread_join(receiver_thread, NULL);
        pthread_join(sender_thread, NULL);

        for(){
            wait(NULL);
        } for each of the processes?
    */

    return 0;
}
