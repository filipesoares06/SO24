#include "HeaderFile.h"

int main(int argc, char *argv[]) {
    int inicialPlafond;
    int numAuthRequests;
    int videoInterval;
    int musicInterval;
    int socialInterval;
    int reservedData;

    pid_t mobileUserId;   //Id do MobileUser, que corresponderá ao PID (Identificador do processo) do processo.

    if (argc != 6) {   //Verifica se os parâmetros estão corretos.
        fprintf(stderr, "mobile_user {plafond inicial} {número de pedidos de autorização} {intervalo VIDEO} {intervalo MUSIC} {intervalo SOCIAL} {dados a reservar}");

        return -1;
    }

    inicialPlafond = atoi(argv[1]);
    numAuthRequests = atoi(argv[2]);
    videoInterval = atoi(argv[3]);
    musicInterval = atoi(argv[4]);
    socialInterval = atoi(argv[5]);
    reservedData = atoi(argv[6]);
    
    mobileUserId = getpid();   //Obtem o identificador do processo.

    //Criar named pipe.

    char registerMessageStr[100];

    snprintf(registerMessageStr, sizeof(registerMessageStr), "%d#%d", mobileUserId, inicialPlafond);
    printf("%s", registerMessageStr);

    //Enviar informação no named pipe.

    while (true) {   //Gerar mensagens e posteriormente enviar para o named pipe.
        char authOrderStr[100];

        snprintf(authOrderStr, sizeof(authOrderStr), "%d#ID Serviço#%d", mobileUserId, reservedData);
        //Ver como efetuar a verificação do intervalo de tempo decorrido e a atualização do valor do plafond.
    }

    return 0;
}
