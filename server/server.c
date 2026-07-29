#include "server.h"
#include "../server_client/net.h"

int server_init(Server *server, uint16_t port)
{
    if(server==NULL || net_is_valid_port(port)!=-1) return -1;
    
    //creiamo il socket listener con il server
    server->tcp_fd=net_create_tcp_server(port);
    
    //set variabili
    FD_ZERO(&server->master_fds);
    //aggiungo tcp_fd alla collezione
    FD_SET(server->tcp_fd, &server->master_fds);
    
    //assegno il socket del server
    server->fdmax=server->tcp_fd;

    server->client_count=0;

    return 0;

}

void server_run(Server *server)
{

    fd_set read_fds;

    while(1)
    {
        read_fds=server->master_fds;
        if(select(server->fdmax+1, &read_fds, NULL, NULL, NULL)==-1)
        {
            printf("Errore select | Reason: %s", strerror(errno));
            break;
        }
        for(int i=0; i<=server->fdmax; i++) //cicliamo tutti i fd
        {
            if(FD_ISSET(i, &read_fds)) //se è
            {
                if(i==server->tcp_fd) server_accept_client(server);
                else server_handle_client_msg(server, i);
            }
        }
    }
}