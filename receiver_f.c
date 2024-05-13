void* receiverFunction(void* arg) {   //Método responsável por implementar a thread receiver.
    writeLogFile("THREAD RECEIVER CREATED");
    fflush(stdout);

    int QUEUE_POS;
    sem_wait(shmSemaphore);
    QUEUE_POS = shMemory->queuePos;
    sem_post(shmSemaphore);

    char (*videoQueue)[100] = (char(*)[100]) arg;
    char (*otherQueue)[100] = (char(*)[100]) arg;

    int queueFront = 0;
    int queueBack = 0;

    int user_pipe_fd = open(USER_PIPE, O_RDONLY);
    int back_pipe_fd = open(BACK_PIPE, O_RDONLY);
    
    if (user_pipe_fd == -1 || back_pipe_fd == -1) {
        perror("Error opening named pipes");
        exit(EXIT_FAILURE);
    }

    fd_set read_fds;
    int max_fd = (user_pipe_fd > back_pipe_fd) ? user_pipe_fd : back_pipe_fd;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(user_pipe_fd, &read_fds);
        FD_SET(back_pipe_fd, &read_fds);

        // Use select() to wait for activity on pipes
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity == -1) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }

        // user_pipe
        if (FD_ISSET(user_pipe_fd, &read_fds)) {
            char buffer[PIPE_BUF];
            ssize_t bytes_read = read(user_pipe_fd, buffer, sizeof(buffer));
            if (bytes_read == -1) {
                perror("Error reading from USER_PIPE");
                exit(EXIT_FAILURE);
            } 
            
            // TODO isto pode dar merda
            else if (bytes_read == 0) {
                close(user_pipe_fd);
                user_pipe_fd = -1;
            } 
            
            else {
                int n1, n2;
                char s[128];
                // msg de registo
                if(sscanf(buffer, "%d#%d", &n1, &n2) == 2){
                    // TODO registar user ??
                }

                else if(sscanf(buffer, "%d#127[^#]#%d", &n1, &s, &n2) == 3){
                    int i;
                    if (strcmp(s, "VIDEO")){
                        // send to video streaming queue
                        for(i = 0; i < QUEUE_POS; i++){
                            if(strcmp(videoQueue[i], "")){
                                strcpy(videoQueue[i], buffer);
                                break;
                            }
                        }
                        if (i == QUEUE_POS){
                            printf("[RT] VIDEO QUEUE IS FULL\n");
                        }
                    }

                    else {
                        // send to other services queue
                        for(i = 0; i < QUEUE_POS; i++){
                            if(strcmp(otherQueue[i], "")){
                                strcpy(otherQueue[i], buffer);
                                break;
                            }
                        }
                        if (i == QUEUE_POS){
                            printf("[RT] OTHER SERVICES QUEUE IS FULL\n");
                        }
                    }
                }

                else {
                    perror("[RT] Wrong user request format\n");
                }
            }
        }

        // back_pipe
        if (FD_ISSET(back_pipe_fd, &read_fds)) {
            char buffer[PIPE_BUF];
            ssize_t bytes_read = read(back_pipe_fd, buffer, sizeof(buffer));
            if (bytes_read == -1) {
                perror("Error reading from BACK_PIPE");
                exit(EXIT_FAILURE);
            } 
            
            // TODO isto pode dar merda
            else if (bytes_read == 0) {
                close(back_pipe_fd);
                back_pipe_fd = -1;
            } 
            
            else {
                // enviar buffer para o other services queue
                int i;
                for(i = 0; i < QUEUE_POS; i++){
                    if(strcmp(otherQueue[i], "")){
                        strcpy(otherQueue[i], buffer);
                        break;
                    }
                }
                if (i == QUEUE_POS){
                    printf("[RT] OTHER SERVICES QUEUE IS FULL\n");
                }
            }
        }
    }

    close(user_pipe_fd);
    close(back_pipe_fd);

    pthread_exit(NULL);
}