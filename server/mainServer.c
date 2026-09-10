#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"


static Server *global_server = NULL;

void handle_sigint(int sig) {
    (void)sig;
    printf("\nRicevuto SIGINT (Ctrl+C). Chiusura del server in corso...\n");
    if (global_server != NULL) {
        server_cleanup(global_server);
    }
    exit(0);
}


int main(int argc, char *argv[]) 
{
    signal(SIGPIPE, SIG_IGN);
    argc = 0;
    argv = NULL;
    uint16_t port=(uint16_t)6767;
    Server server;
    
    global_server=&server;
    struct sigaction sa;
    sa.sa_handler=handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0){
        perror("Errore sigaction");
        return EXIT_FAILURE;
    }  

    if(server_init(&server, port)!=0)
    {
        printf("Errore init server\n");
        return EXIT_FAILURE;
    }
    printf("Server ON, port: %u\n", port);
    server_run(&server);
    server_cleanup(&server);

    return 0;
}