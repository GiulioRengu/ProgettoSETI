#include <signal.h>
#include "server.h"
#include "../server_client/net.h"

int server_init(Server *server, uint16_t port)
{
    if(server==NULL || net_is_valid_port(port)==-1) return -1;
    memset(server->users, 0, sizeof(server->users));
    for(int i=0; i<MAX_USERS; i++) server->users[i].connected=-1;
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
    // if(server->client_count >= MAX_USERS){
    //     printf("Errore server_accept_client: server->client_count >= MAX_USERS\n");
    //     return;
    // }
    char client_ip[INET6_ADDRSTRLEN];
    uint16_t client_tcp_port = 0;
    int client_fd = 0;
    if((client_fd = net_accept(server->tcp_fd, client_ip, &client_tcp_port)) < 0){
        printf("Errore server_accept_client: net_accept ritorna -1\n");
        return;
    }

    FD_SET(client_fd, &server->master_fds);
    if (client_fd > server->fdmax){
        server->fdmax = client_fd;
    }

    strncpy(server->pendingClients[client_fd], client_ip, INET6_ADDRSTRLEN);
}

void server_handle_client_msg(Server *server, int client_fd){
    char msg[MSG_BUFF_MAXSIZE];
    int r = net_recv_msg(client_fd, msg, MSG_BUFF_MAXSIZE);
    if(r==0){
        //gestione errore
        printf("Errore server_handle_client_msg: errore client disconensso");
        return;
    }

    int type = get_type(msg);
    switch(type){

        case MSG_REGIS:    handle_regis(server, client_fd, msg); break;
        case MSG_CONNE:    handle_conne(server, client_fd, msg); break;
        case MSG_FRIE_REQ: handle_frie(server, client_fd, msg); break;
        case MSG_MESS_REQ: handle_mess(server, client_fd, msg); break;
        case MSG_FLOO_REQ: handle_floo(server, client_fd, msg); break;
        case MSG_LIST_REQ: handle_list(server, client_fd); break;
        case MSG_CONSU:    handle_consu(server, client_fd); break;
        case MSG_OKIRF:    handle_friend_reply(server, client_fd, true); break;
        case MSG_NOKRF:    handle_friend_reply(server, client_fd, false); break;
        case MSG_IQUIT:    handle_quit(server, client_fd); break;

        default:
            printf("Errore server_handle_client_msg: tipo messaggio invalido/sconosciuto\n");
            return;
    }
}

User* get_user_by_id(Server *server, const char *id)
{
    if(server==NULL || server->client_count==0)
    {
        printf("Nessun utente registrato nel server o nessun server disponibile\n");
        return NULL;
    }
    for(int i=0; i<server->client_count; i++)
    {
        if(strcmp(server->users[i].id, id) == 0)
        {
            return &server->users[i];
        }
    }
    printf("User %s non trovato", id);
    return NULL;
}

User* get_user_by_fd(Server *server, int fd)
{
    if(server==NULL || server->client_count==0)
    {
        printf("Nessun utente registrato nel server o nessun server disponibile\n");
        return NULL;
    }
    for(int i=0; i<MAX_USERS; i++)
    {
        if(server->users[i].tcp_fd==fd)
        {
            return &server->users[i];
        }
    }
    printf("User %d non trovato\n", fd);
    return NULL;
}

// Invia una notifica UDP [YXX] al client
int server_send_udp_notification(Server *server, User *user, StreamType type)
{
    if(server==NULL || user==NULL)
    {
        printf("Server o user null\n");
        return -1;
    }
    if(net_send_udp(user, type, user->stream_count)<0)
    {
        printf("Problema a mandare UDP notif a %s",user->id);
        return -1;
    }

    return 0;
}

//da controllare
void server_disconnect(Server *server, int fd){
    for (int i = 0; i < server->client_count; i++){
        if (server->users[i].tcp_fd == fd){
            server->users[i].connected = -1;   
            server->users[i].tcp_fd = -1;       

            FD_CLR(fd, &server->master_fds);
            close(fd);
            return;
        }
    }

    memset(server->pendingClients[fd], 0, INET6_ADDRSTRLEN);
    FD_CLR(fd, &server->master_fds);
    close(fd);
}

// Pulizia risorse server
void server_cleanup(Server *server)
{
    if(server==NULL) return;

    if(server->tcp_fd>=0) close(server->tcp_fd);

    for(int i=0; i<server->client_count; i++)
    {
        if(server->users[i].connected==0)
        {
            close(server->users[i].tcp_fd);
        }
        Stream *current=server->users[i].streams;
        while(current!=NULL)
        {
            Stream *tmp=current;
            current=current->next;
            free(tmp);
        }
        server->users[i].streams=NULL;
    }
    server->client_count=0;
    server->fdmax=0;
    FD_ZERO(&server->master_fds);
}
