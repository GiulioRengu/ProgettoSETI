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

void server_accept_client(Server *server){
    if(server->client_count >= MAX_USERS){
        printf("Errore server_accept_client: server->client_count >= MAX_USERS\n");
        return;
    }

    char client_ip[INET6_ADDRSTRLEN];
    uint16_t client_port = 0;
    int client_fd = 0;
    if((client_fd = net_accept(server->fdmax, client_ip, &client_port)) < 0){
        printf("Errore server_run: net_accept ritorna -1\n");
        return;
    }

    User new_user;
    new_user.connected = 0;
    new_user.friend_count = 0;
    strcpy(new_user.ip, client_ip);
    new_user.tcp_fd = client_fd;
    new_user.udp_port = htons(client_port);

    server->users[server->client_count] = new_user;
    server->client_count++;
    FD_SET(client_fd, &server->master_fds); //serve?
}

void server_handle_client_msg(Server *server, int client_fd){
    char msg[MSG_BUFF_MAXSIZE];
    int r = net_recv_msg(client_fd, msg, MSG_BUFF_MAXSIZE);
    if(r<0){
        //gestione errore
        return;
    }

    int type = get_type(msg);
    switch(type){
        case(MSG_REGIS):
            break;
        case(MSG_CONNE):
            break;
        case(MSG_FRIE_REQ):
            break;
        case(MSG_MESS_REQ):
            break;
        case(MSG_FLOO_REQ):
            break;
        case(MSG_LIST_REQ):
            break;
        case(MSG_CONSU):
            break;
        case(MSG_OKIRF):
            break;
        case(MSG_NOKRF):
            break;
        case(MSG_IQUIT):
            break;

        default:
            printf("Errore server_handle_client_msg: tipo messaggio invalido/sconosciuto\n");
            return;
    }
}