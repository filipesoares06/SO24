#include "HeaderFile.h"

int main(int argc, char *argv[]) {
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
            printf("Invalid Id!\n");
        }

        else if (strcmp(inputCommands, "data_stats") != 0 && strcmp(inputCommands, "reset") != 0) {
            printf("Invalid Command!\n");
        }

        else {
            if (strcmp(inputCommands, "data_stats")) {
                printf("Service     Total Data     Auth Reqs\n");
            }

            else if (strcmp(inputCommands, "reset")) {
                printf("Reseting\n");
            }
        }
    }

    return 0;
}
