#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    if (argc != 1) {
        fprintf(stderr, "backoffice_user");

        return -1;
    }

    backOfficeUserCommands();   //Imprime os comandos disponíveis. Tirar?

    char commandInput[100];

    while (true) {
        fgets(commandInput, sizeof(commandInput), stdin);
        char *verifyInput;
        int backOfficeUserId;
        char inputCommands[100];

        verifyInput = strtok(commandInput, "#");
        backOfficeUserId = atoi(verifyInput);

        while (verifyInput != NULL) {
            strcpy(inputCommands, verifyInput);
            verifyInput = strtok(NULL, "#");
        }

        if (backOfficeUserId != 1) {
            writeLogFile("Invalid Id!\n");
        }

        else if (strcmp(inputCommands, "data_stats\n") != 0 && strcmp(inputCommands, "reset\n") != 0) {
            writeLogFile("Invalid Command!\n");
        }

        else {
            if (strcmp(inputCommands, "data_stats\n") == 0) {
                // writeLogFile("Service     Total Data     Auth Reqs\n");

                int fd = mkfifo(BACK_PIPE, O_WRONLY);
                if(fd == -1){
                    perror("Error while opening back_pipe");
                    writeLogFile("[BOU] Error while opening back_pipe");
                    exit(1);
                }

                char msg[24];
                snprintf(msg, sizeof(msg), "%d#data_stats", backOfficeUserId);

                write(fd, msg, strlen(msg) + 1);

                close(fd); 
                
                // TODO ler estatisticas da message queue
                message aux; bool flag = true;
                while(flag){
                    if(msgrcv(msgq_id, &aux, sizeof(struct message), 200, 0) == -1){
                        perror("Error while receiving stats from message queue");
                        writeLogFile("[BOU] Error while receiving stats from message queue");
                        exit(1);
                    }

                    printf("%s\n", aux.msg);
                    flag = false;
                }
            }

            else if (strcmp(inputCommands, "reset\n") == 0) {
                // writeLogFile("Reseting\n");

                int fd = mkfifo(BACK_PIPE, O_WRONLY);
                if(fd == -1){
                    perror("Error while opening back_pipe");
                    writeLogFile("[BOU] Error while opening back_pipe");
                    exit(1);
                }

                char msg[24]; // TODO is size enough?
                snprintf(msg, sizeof(msg), "%d#reset", backOfficeUserId);

                write(fd, msg, strlen(msg) + 1);

                close(fd);
            }
        }
    }

    return 0;
}
