#include "HeaderFile.h"

/*bool validateMobileUser(char *inputMobileUser[]) {   //Método responsável por validar os inputs do Mobile User.
    bool validateId;
    
    return;
}*/

void backOfficeUserCommands() {   //Método responsável por imprimir os comandos disponíveis no processo BackOfficeUser.
    printf("Available Operations:\n");
    printf("1) data_stats\n");
    printf("2) reset\n");
    printf("ID_backoffice_user#[data_stats | reset]\n\n");
    fflush(stdout);
}
