#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"


static Server *global_server = NULL;

void handle_sigint(int sig) {
    (void)sig;
    printf("\nRicevuto SIGINT (Ctrl+C). Chiusura del server in corso\n");
    if (global_server != NULL) {
        server_cleanup(global_server);
    }
    exit(0);
}


int main(int argc, char *argv[]) 
{
    if (argc < 2){
        printf("Errore argomenti\nUsage: %s <porta-server>  <flag -v per attivare verbosa>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(argc == 3 && strncmp(argv[2], "-v", 2) == 0) verbose = 1;

    int port = atoi(argv[1]);
    if (port < 0 || port > 9999){
        printf("Errore, la porta deve essere compresa tra 0 e 9999\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);
    argc = 0;
    argv = NULL;
    Server server;
    
    global_server=&server;
    struct sigaction sa;
    sa.sa_handler=handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0){
        perror("Errore sigaction\n");
        return EXIT_FAILURE;
    }  

    if(server_init(&server, port)!=0)
    {
        printf("Errore init server\n");
        return EXIT_FAILURE;
    }
    VERB("Server attivo su porta: %u", port);
    server_run(&server);
    server_cleanup(&server);

    return 0;
}