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

void* receiverFunction() {   //Método responsável por implementar a thread receiver.
    char *userPipeDescriptor = "/tmp/userpipe";
    char *backPipeDescriptor =  "/tmp/backpipe";

    int userFd;
    int backFd;
    int maxFd;
    char fdBuffer[128];

    fd_set readFds;

    mkfifo(userPipeDescriptor, 0666);
    mkfifo(backPipeDescriptor, 0666);

    userFd = open(userPipeDescriptor, O_RDONLY | O_NONBLOCK);
    backFd = open(backPipeDescriptor, O_RDONLY | O_NONBLOCK);
    if (userFd == -1 || backFd == -1) {
        perror("Error while opening FIFO");

        pthread_exit(NULL);

        exit(1);
    }

    writeLogFile("THREAD RECEIVER CREATED");
    fflush(stdout);

    int numUserFdMessages = 0;

    maxFd = userFd > backFd ? userFd : backFd;

    while (1) {
        FD_ZERO(&readFds);
        FD_SET(userFd, &readFds);
        FD_SET(backFd, &readFds);

        struct timeval timeOut;
        timeOut.tv_sec = 0;
        timeOut.tv_usec = 1000000;   //Permite ao select ver atividade nas fifo de 100 em 100 ms.
        
        int newActivity = select(maxFd + 1, &readFds, NULL, NULL, &timeOut);

        if (newActivity < 0) {
            perror("Error in select");
            
            pthread_exit(NULL);

            exit(1);
        }

        if (FD_ISSET(userFd, &readFds)) {
            int bytesRead = read(userFd, fdBuffer, sizeof(fdBuffer) - 1);

            if (bytesRead > 0) {
                if (numUserFdMessages == 0) {   //A primeira mensagem de registo recebida pelo user pipe é a de registo do mobile user.
                    printf("Register message: %s\n", fdBuffer);
                }
                
                else {
                    fdBuffer[bytesRead] = '\0';
                
                    printf("%s\n", fdBuffer);
                    addToQueue(fdBuffer);
                }

                numUserFdMessages++;
            } 
        }

        if (FD_ISSET(backFd, &readFds)) {
            int bytesRead = read(backFd, fdBuffer, sizeof(fdBuffer) - 1);

            if (bytesRead > 0) {
                fdBuffer[bytesRead] = '\0';

                printf("%s\n", fdBuffer);
                addToQueue(fdBuffer);
            } 
        }
    }

    close(userFd);
    close(backFd);

    pthread_exit(NULL);
}

void addToQueue(char *authOrder) {   //Método responsável por adicionar o pedido de autorização à respetiva queue.
    char authOrderBck[128];
    char *authOrderToken;
    char authOrderType[128];

    strcpy(authOrderBck, authOrder);   //De forma a não alterar o pedido de alteração original.

    authOrderToken = strtok(authOrderBck, "#");   //Primeiro token (mobileUserId) não é necessário verificar aqui, somente o tipo de pedido de autorização, logo não guardamos este valor.
    authOrderToken = strtok(NULL, "#");

    strcpy(authOrderType, authOrderToken);   //Guardamos o tipo de pedido de autorização para verificar em qual queue guardar.
 
    sem_wait(shmSemaphore);
    int queueSize = shMemory -> queuePos;
    sem_post(shmSemaphore);

    if (strcmp(authOrderType, "VIDEO") == 0) {   //Pedido de autorização de vídeo, logo guardamos na videoQueue.
        sem_wait(videoQueueSemaphore);

        if ((queueBackVideo + 1) % queueSize == queueFrontVideo) {   //O pedido de autorização de vídeo não é adicionado.
            writeLogFile("VIDEO AUTHORIZATION REQUEST NOT ADDED: THE QUEUE IS FULL");
        }

        else {   //O pedido de autorização de vídeo é adicionado.
            strncpy(videoQueue[queueBackVideo], authOrder, sizeof(videoQueue[queueBackVideo]));

            queueBackVideo = (queueBackVideo + 1) % queueSize;
        }
        printf("VerificaçãoADD:%s\n", videoQueue[0]);   //TODO Tem uns caracteres todos feios, está a ser mal guardado.
        fflush(stdout);
        sem_post(videoQueueSemaphore);
    }

    else {   //Pedido de autorização de outro tipo, logo guardamos na otherQueue.
        sem_wait(otherQueueSemaphore);

        if ((queueBackOther + 1) % queueSize == queueFrontOther) {   //O pedido de autorização de vídeo não é adicionado.
            writeLogFile("OTHER AUTHORIZATION REQUEST NOT ADDED: THE QUEUE IS FULL");
        }

        else {   //O pedido de autorização de vídeo é adicionado.
            strncpy(otherQueue[queueBackOther], authOrder, sizeof(otherQueue[queueBackOther]));

            queueBackOther = (queueBackOther + 1) % queueSize;
        }

        sem_post(otherQueueSemaphore);
    }
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
    
    sem_wait(shmSemaphore);   //Acessa a memória partilhada de forma a obter o valor de queuePos e inicializar as queues.

    int queueSize = shMemory -> queuePos;

    videoQueue = (char **) malloc(queueSize * sizeof(char *));
    otherQueue = (char **) malloc(queueSize * sizeof(char *));

    for (int i = 0; i < queueSize; i++) {   //TODO Não esquecer e libertar esta memória.
        videoQueue[i] = (char *) malloc(128 * sizeof(char));
        otherQueue[i] = (char *) malloc(128 * sizeof(char));
    }

    sem_post(shmSemaphore);

    initThreads();
    
    pthread_join(receiverThread, NULL);
    pthread_join(senderThread, NULL);
}

void authorizationRequestManager() {   //Método responsável por criar o processo Authorization Request Manager.
    createProcess(authorizationRequestManagerFunction, NULL);

    wait(NULL);   //Aguarda que os processos fiilhos terminem.
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

    authorizationRequestManager();

    return 0;
}
