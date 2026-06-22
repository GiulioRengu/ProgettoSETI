#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"
#include "serverhandlers.h"

int main(){
    char id[ID_LENGTH+2], port[6], pass[7];

    fgets(id, sizeof(id), stdin);
    //questo serve per fermare subito l'input se l'id è troppo lungo ma dovremo farlo poi nel server.c (credo)
    if (!strchr(id, '\n')) {
        // riga troppo lunga
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("1");
        return 1;
    }
    id[strcspn(id, "\n")] = '\0'; //https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input

    fgets(port, sizeof(port), stdin);
    port[strcspn(port, "\n")] = '\0';

    fgets(pass, sizeof(pass), stdin);
    pass[strcspn(pass, "\n")] = '\0';

    User* u = createNewUser(id, atoi(port), atoi(pass));
    int valid=checkValidData(id, atoi(port), atoi(pass));

    if(valid==0)
    {
        if(u!=NULL)
            printf("\n\nUtente creato: %s\nPorta: %d\nPassword: %d\n", u->id, u->tcp_fd, u->password);
    }
    printf("\n%d\n", valid);
}