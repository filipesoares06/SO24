//Filipe Freire Soares 2020238986
//Francisco Cruz Macedo 2020223771

#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint);

    if (argc != 1) {
        fprintf(stderr, "backoffice_user");

        return -1;
    }

    int fd = open(BACK_PIPE, O_WRONLY);   //É aberto o named pipe BACK_PIPE para escrita. Named pipe é criado no Authorization Request Manager.
    if(fd == -1){
        perror("Error while opening BACK_PIPE");
                    
        exit(1);
    }

    backOfficeUserCommands();   //Imprime os comandos disponíveis.

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
            printf("Invalid Id!\n");
        }

        else if (strcmp(inputCommands, "data_stats\n") != 0 && strcmp(inputCommands, "reset\n") != 0) {
            printf("Invalid Command!\n");
        }

        else { 
            if (strcmp(inputCommands, "data_stats\n") == 0) {   //Executa a operação data_stats.
                char msg[64];

                snprintf(msg, sizeof(msg), "%d#data_stats", backOfficeUserId);

                write(fd, msg, strlen(msg) + 1); 
                /*
                message aux; bool flag = true;

                
                while(flag) {   //TODO ler estatisticas da message queue
                    if(msgrcv(msgq_id, &aux, sizeof(struct message), 200, 0) == -1) {
                        perror("Error while receiving stats from message queue");
                        
                        exit(1);
                    }

                    printf("%s\n", aux.msg);

                    flag = false;
                }
                */
               
            }

            else if (strcmp(inputCommands, "reset\n") == 0) {   //Executa a operação reset.
                char msg[64];

                snprintf(msg, sizeof(msg), "%d#reset", backOfficeUserId);

                write(fd, msg, strlen(msg) + 1);
            }
        }
    }

    close(fd);

    return 0;
}
