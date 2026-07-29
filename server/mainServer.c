#include "server.h"

int main(int argc, char *argv[]) 
{
    uint16_t port=(uint16_t)6767;
    Server server;

    if(server_init(&server, port)!=0)
    {
        printf("Errore init server\n");
        return EXIT_FAILURE;
    }
    printf("Server ON, port: %u", port);
    server_run(&server);
    server_cleanup(&server);

    return 0;
}