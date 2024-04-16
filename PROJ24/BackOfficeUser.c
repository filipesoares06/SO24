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
                writeLogFile("Service     Total Data     Auth Reqs\n");
            }

            else if (strcmp(inputCommands, "reset\n") == 0) {
                writeLogFile("Reseting\n");
            }
        }
    }

    return 0;
}
