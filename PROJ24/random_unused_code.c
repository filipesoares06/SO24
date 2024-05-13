// --- Auth engine old version ---

void authorization_engine(int engine_id){
    // le mensagens do sender pelo unnamed pipe
    char aux[1024];
    read(fd_sender_pipes[engine_id][0], &aux, sizeof(aux));

    int user_id; int req_value;
    if (sscanf(aux, "%d#%d", &user_id, &req_value) != 2) {
        perror("[AE] Error - Failed to parse the string\n");

        exit(1);
    }

    int n_users; bool found = false;

    sem_wait(shmSemaphore);
    n_users = shMemory->n_users;
    sem_post(shmSemaphore);

    
    for(int i = 0; i < n_users; i++){
        sem_wait(shmSemaphore);       
        if(shMemory->mobileUsers[i].user_id == user_id && shMemory->mobileUsers[i].usedData + req_value <= shMemory->mobileUsers[i].inicialPlafond){
            shMemory->mobileUsers[i].usedData += req_value;
    
            if((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.8 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 0.9){
                shMemory->mobileUsers[i].alertAux = 1;
                char alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 80%% of DATA USAGE\n", user_id);
                //writeLogFile(alert);
            }

            else if((shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) >= 0.9 && (shMemory->mobileUsers[i].usedData / shMemory->mobileUsers[i].inicialPlafond) < 1.0){
                shMemory->mobileUsers[i].alertAux = 2;
                char alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 90%% of DATA USAGE\n", user_id);
                //writeLogFile(alert);
            }

            else if(shMemory->mobileUsers[i].usedData == shMemory->mobileUsers[i].inicialPlafond){
                shMemory->mobileUsers[i].alertAux = 3;
                char alert[40];
                snprintf(alert, sizeof(alert), "USER %d REACHED 100%% of DATA USAGE\n", user_id);
                //writeLogFile(alert);
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
        //writeLogFile("[AE] Error while opening back_pipe");
        exit(1);
    }
    
    if(read(fd, back_msg, sizeof(back_msg)) == -1){
        perror("Error reading from named pipe");
        //writeLogFile("[AE] Error reading from named pipe");
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
        shMemory->totalVideoData, shMemory->totalVideoAuthReq, 
        shMemory->totalMusicData, shMemory->totalMusicAuthReq, 
        shMemory->totalSocialData, shMemory->totalSocialAuthReq);
        sem_post(shmSemaphore);

        key_t key = ftok("msgfile", 'A');
        int msgq_id = msgget(key, 0666 | IPC_CREAT);

        if(msgq_id == -1){
            perror("Error while opening Message Queue");
            //writeLogFile("[AE] Error while opening Message Queue");
            exit(1);
        }

        if(msgsnd(msgq_id, &msg, 1024, 0) == -1){
            perror("Error while sending message");
            //writeLogFile("[AE] Error while sending message");
            exit(1);
        }

        //writeLogFile("[AE] Stats Executed");
    }

    else if(strcmp(string_part, "reset")){
        sem_wait(shmSemaphore);
        shMemory->totalVideoData = 0;
        shMemory->totalVideoAuthReq = 0;
        shMemory->totalMusicData = 0;
        shMemory->totalMusicAuthReq = 0; 
        shMemory->totalSocialData = 0;
        shMemory->totalSocialAuthReq = 0;
        sem_post(shmSemaphore);

        //writeLogFile("[AE] Reset executed");
    }
}


// -------------------------------

// --- RECEIVER OLD ---
void* receiverFunction() {   //Método responsável por implementar a thread receiver.
    writeLogFile("THREAD RECEIVER CREATED");
    fflush(stdout);
    
    int fdUserPipe = open(USER_PIPE, O_RDONLY);
    int fdBackPipe = open(BACK_PIPE, O_RDONLY);
    
    if (fdUserPipe == -1 || fdBackPipe == -1) {
        perror("Error while opening named pipes");
        
        exit(EXIT_FAILURE);
    }
    
    fd_set read_fds;
    int fdMax = (fdUserPipe > fdBackPipe) ? fdUserPipe : fdBackPipe;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(fdUserPipe, &read_fds);
        FD_SET(fdBackPipe, &read_fds);

        int activity = select(fdMax + 1, &read_fds, NULL, NULL, NULL);   // Use select() to wait for activity on pipes
        if (activity == -1) {
            perror("Error selecting pipe");

            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(fdUserPipe, &read_fds)) {   //Name pipe USER_PIPE.
            int queueFront = 0;
            int queueBack = 0;
            char fdBuffer[PIPE_BUF];

            ssize_t bytes_read = read(fdUserPipe, fdBuffer, sizeof(fdBuffer));
            if (bytes_read == -1) {
                perror("Error reading from USER_PIPE");

                exit(EXIT_FAILURE);
            } 
            
            else if (bytes_read == 0) {   // TODO isto pode dar merda
                close(fdUserPipe);

                fdUserPipe = -1;
            } 
            
            else {
                int n1, n2;
                char s[128];
                
                if(sscanf(fdBuffer, "%d#%d", &n1, &n2) == 2) {   //Mensagem de registo.
                    // TODO NÃO É PARA REGISTAR! É PARA ENVIAR PARA O SENDER E O SENDER ENVIA PARA O AUTH ENGINE ONDE ELE FAZ O REGIST
                    // TODO APROVEITA O CODIGO QUE ESTÁ AQUI
                    mobileUser aux_user;
                    aux_user.user_id = n1;
                    aux_user.inicialPlafond = n2;
                    // aux_user.numAuthRequests = 0; aux_user.videoInterval = 0; aux_user.musicInterval = 0;
                    int n_users;
                    sem_wait(shmSemaphore);
                    n_users = shMemory->n_users;
                    sem_post(shmSemaphore);

                    for(int i = 0; i < n_users; i++){
                        sem_wait(shmSemaphore);
                        if(&(shMemory->mobileUsers[i]) == NULL){
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

                else if(sscanf(fdBuffer, "%d#127[^#]#%d", &n1, &s, &n2) == 3){
                    if (strcmp(s, "VIDEO")){   //Envia para a video streaming queue.
                        addVideoQueue(fdBuffer, queueFront, queueBack);
                        #if DEBUG
                        printf("%s\n", fdBuffer);
                        #endif
                    }

                    else {   //Envia para a other services queue.
                        addOtherServQueue(fdBuffer, queueFront, queueBack);
                        #if DEBUG
                        printf("%s\n", fdBuffer);
                        #endif
                    }
                }

                else {
                    perror("[RT] Wrong user request format\n");
                }
            }
        }

        if (FD_ISSET(fdBackPipe, &read_fds)) {   //Named pipe BACK_PIPE.
            int queueFront = 0;
            int queueBack = 0;
            
            char buffer[PIPE_BUF];

            ssize_t bytes_read = read(fdBackPipe, buffer, sizeof(buffer));
            if (bytes_read == -1) {
                perror("Error reading from BACK_PIPE");

                exit(EXIT_FAILURE);
            } 
            
            else if (bytes_read == 0) {   // TODO isto pode dar merda.
                close(fdBackPipe);

                fdBackPipe = -1;
            } 
            
            else {   //Envia para a other services queue.
                addOtherServQueue(buffer, queueFront, queueBack);
                #if DEBUG
                printf("%s\n", fdBuffer);
                #endif
            }
        }
    }

    close(fdUserPipe);
    close(fdBackPipe);

    pthread_exit(NULL);
}
// ----------------------------