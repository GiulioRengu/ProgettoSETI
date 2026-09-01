#include "serverHandlers.h"

void handle_regis(Server* server, int client_fd, char* msg){
    char retmsg[9];

    char id[ID_LENGTH+1];
    char port[5];
	get_port(msg, port, 15);

    if (net_is_valid_port(port) < 0){
        printf("Errore handle_regis: port non valida\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        return;
    }

	uint16_t udp_port = (uint16_t)atoi(port);
    uint16_t password = (unsigned char)msg[20] | ((unsigned char)msg[21] << 8);

    get_id(msg, id);
    if (net_is_valid_id(id) < 0){
        printf("Errore handle_regis: id non valido\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        return;
    }

    if (get_user_by_id(server, id) != NULL){
        printf("Errore handle_regis: utente gia esistente\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        return;
    }

    if (server->client_count >= MAX_USERS){
        printf("Errore handle_regis: server pieno\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        return;
    }

    char client_ip[INET6_ADDRSTRLEN];
    strncpy(client_ip, server->pendingClients[client_fd], INET6_ADDRSTRLEN);
    User new_user;
    memset(&new_user, 0, sizeof(User));

    new_user.connected = 0;
    strcpy(new_user.id, id);
    strncpy(new_user.ip, client_ip, INET6_ADDRSTRLEN);
    new_user.password = password;
    new_user.udp_port = udp_port;
    new_user.tcp_fd = client_fd;

    int success = -1;
    for(unsigned i = 0; i<MAX_USERS; i++){
        if(server->users[i].id[0] == '\0'){
            server->users[i] = new_user;
            success = 0;
            server->client_count++;
            break;
        }
    }

    if(success < 0) build_gobye(retmsg);
    else build_welco(retmsg);

    net_send_str(client_fd, retmsg);
}

void handle_conne(Server* server, int client_fd, char* msg);

void handle_frie(Server* server, int client_fd, char* msg);

void handle_mess(Server* server, int client_fd, char* msg);

void handle_floo(Server* server, int client_fd, char* msg);

void handle_list(Server* server, int client_fd, char* msg);

void handle_consu(Server* server, int client_fd, char* msg);

void handle_friend_reply(Server* server, int client_fd, char* msg, bool accepted);

void handle_quit(Server* server, int client_fd, char* msg);