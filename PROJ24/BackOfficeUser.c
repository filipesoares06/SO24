#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "backoffice_user");

        return -1;
    }

    char commandInput[100];
    int backOfficeUserId = 1;   //O identificador a utilizar é sempre 1.

    backOfficeUserCommands();   //Imprime os comandos disponíveis. Tirar?

    while (true) {
        fgets(commandInput, sizeof(commandInput), stdin);

        int verifyId;
        char *verifyInput;
        verifyInput = strtok(commandInput, "#");

        verifyId = atoi(verifyInput);
        
        while (verifyInput != NULL) {
            printf("%s\n", verifyInput);
            verifyInput = strtok(NULL, "#");
        }

    }

    return 0;
}
