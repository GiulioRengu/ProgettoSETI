#include "serverHandlers.h"
#include "streamHandlers.h"

void handle_regis(Server* server, int client_fd, char* msg){ //serve fare disconnect quando fallisce?
    if(server == NULL || client_fd < 0 || msg == NULL) return;

    char retmsg[9];

    char id[ID_LENGTH+1];
    char port[5];
	get_port(msg, port, 15);

    uint16_t udp_port = (uint16_t)atoi(port);
    if (net_is_valid_port(udp_port) < 0){
        printf("Errore handle_regis: port non valida\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    uint16_t password = (unsigned char)msg[20] | ((unsigned char)msg[21] << 8);

    get_id(msg, id);
    if (net_is_valid_id(id) < 0){
        printf("Errore handle_regis: id non valido\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (get_user_by_id(server, id) != NULL){
        printf("Errore handle_regis: utente gia esistente\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (server->client_count >= MAX_USERS){
        printf("Errore handle_regis: server pieno\n");
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    char client_ip[INET6_ADDRSTRLEN];
    strncpy(client_ip, server->pendingClients[client_fd], INET6_ADDRSTRLEN);
    memset(server->pendingClients[client_fd], 0, INET6_ADDRSTRLEN);
    User new_user;
    memset(&new_user, 0, sizeof(User));

    new_user.connected = 0;
    strncpy(new_user.id, id, 9);
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

    if(success < 0){
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }
    else build_welco(retmsg);

    printf("Utente %s registrato con ip %s, porta %s e fd %d\n", id, new_user.ip, port, client_fd);
    net_send_str(client_fd, retmsg);
}

void handle_conne(Server* server, int client_fd, char* msg){
    if (server == NULL || client_fd < 0 || msg == NULL) return;

    char retmsg[9];
    char id[ID_LENGTH+1];
    get_id(msg, id);
    uint16_t password = (unsigned char)msg[15] | ((unsigned char)msg[16] << 8);
    
    User* target = get_user_by_id(server, id);
    if (target == NULL){
        printf("Errore handle_conne: utente %s non esistente\n", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }  

    if (target->password != password){
        printf("Errore handle_conne: password per %s errata\n", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (target->connected == 0){
        printf("Errore handle_conne: utente %s gia connesso\n", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    target->tcp_fd = client_fd;
    target->connected = 0;
    strncpy(target->ip, server->pendingClients[client_fd], INET6_ADDRSTRLEN);
    memset(server->pendingClients[client_fd], 0, INET6_ADDRSTRLEN);

    build_welco(retmsg);
    if(net_send_str(client_fd, retmsg) < 0){
        printf("Errore handle_conne: errore invio HELLO+++\n");
        server_disconnect(server, client_fd);
        return;
    }

}

void handle_frie(Server* server, int client_fd, char* msg){
    if (server == NULL || client_fd < 0 || msg == NULL) return;
    char retmsg[9]; 

    User* src = get_user_by_fd(server, client_fd);
    char dest_id[ID_LENGTH+1];
    get_id(msg, dest_id);
    User* dest = get_user_by_id(server, dest_id);

    if (src == NULL || dest == NULL){
        printf("Errore handle_frie: utente/i non esistenti\n");
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (strcmp(src->id, dest->id) == 0){
        printf("Errore handle_frie: sei gia amico di te stesso!\n");
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    int friends_found = 0, i = 0;
    while(friends_found < src->friend_count){
        if (src->friends[i][0] != '\0'){
            friends_found++;
            if (strcmp(src->friends[i], dest->id) == 0){
                printf("Errore handle_frie: sei gia amico di %s!\n", dest->id);
                build_frie_ko(retmsg);
                if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
                return;
            }
        }
        i++;
    }

    Stream* aux = dest->streams;
    while(aux != NULL){
        if (strcmp(aux->from_id, src->id) == 0 && aux->type == STREAM_FRIEND_REQ){
            printf("Errore handle_frie: hai gia mandato una richiesta di amicizia a %s!\n", dest->id);
            build_frie_ko(retmsg);
            if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
            return;
        }
        aux = aux->next;
    }

    // if (dest->stream_count >= MAX_STREAMS){
    //     printf("Errore handle_frie: %s ha troppe richieste\n", dest->id);
    //     build_frie_ko(retmsg);
    //     if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
    //     return;
    // }

    if(stream_add(dest, src->id, NULL, STREAM_FRIEND_REQ)<0)
    {
        printf("Errore invio amicizia da %s a %s", src, dest);
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    // Stream* new_stream = malloc(sizeof(Stream));
    // new_stream->type = STREAM_FRIEND_REQ;
    // strncpy(new_stream->from_id, src->id, ID_LENGTH+1);
    // new_stream->next = NULL;

    // if (dest->streams == NULL){
    //     dest->streams = new_stream;
    // } 
    // else {
    //     aux = dest->streams;
    //     while (aux->next != NULL) aux = aux->next;
    //     aux->next = new_stream;
    // }
    // dest->stream_count++;

    if(server_send_udp_notification(server, dest, STREAM_FRIEND_REQ) < 0){
        printf("Errore handle_frie: invio notifica udp a %s non riuscito\n", dest->id);
    }

    build_frie_ok(retmsg);
    if(net_send_str(client_fd, retmsg) < 0){
        printf("Errore handle_frie: errore invio FRIE>+++\n");
        server_disconnect(server, client_fd);
        return;
    }

    printf("Richiesta di amicizia da %s inviata correttamente a %s\n", src->id, dest->id);
}

void handle_mess(Server* server, int client_fd, char* msg){
    if (server == NULL || client_fd < 0 || msg == NULL) return;
    char retmsg[9]; 
    char to_send[MSG_LENGTH_MAX];

    User* src = get_user_by_fd(server, client_fd);
    char dest_id[ID_LENGTH+1];
    get_id(msg, dest_id);
    User* dest = get_user_by_id(server, dest_id);
    get_msg(msg, to_send, 9);

    if (net_is_valid_msg(to_send) < 0){
        build_mess_ko(retmsg);
        if (net_send_str(client_fd, retmsg) < 0){
            printf("Errore handle_mess: errore durante invio MESS<\n");
            server_disconnect(server, client_fd);
            return;
        }
        return;
    }

    if (src == NULL || dest == NULL){
        printf("Errore handle_mess: utente/i non esistenti\n");
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (strcmp(src->id, dest->id) == 0){
        printf("Errore handle_mess: non puoi inviare un messaggio a te stesso!\n");
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    //da fare una funzione che controlla se due utenti sono gia amici
    int friends_found = 0, i = 0, found = -1;
    while(friends_found < src->friend_count){
        if (src->friends[i][0] != '\0'){
            friends_found++;
            if (strcmp(src->friends[i], dest->id) == 0){
                found = 0;
                break;
            }
        }
        i++;
    }

    if (found < 0){
        printf("Errore handle_mess: non sei amico di %s!\n", src->id);
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    server_send_udp_notification(server, dest, STREAM_MSG);

    //inviare stream
}

void handle_floo(Server* server, int client_fd, char* msg){return;}

void handle_list(Server* server, int client_fd, char* msg){
    char u_id[24];
    User* u = get_user_by_fd(server, client_fd);
    if (u == NULL || server == NULL) return;

    build_rlist(u_id, server->client_count);
    if (net_send_str(client_fd, u_id) < 0){
        printf("Errore handle_list: errore invio RLIST\n"); 
        server_disconnect(server, client_fd);
        return;
    }

    for (unsigned i = 0; i<server->client_count; i++){
        build_linum(u_id, server->users[i].id);
        if (net_send_str(client_fd, u_id) < 0){
            printf("Errore handle_list: errore invio LINUM, inviati %d\n", i);
            server_disconnect(server, client_fd);
            return;
        }
    }

    printf("RLIST eseguito correttamente: inviata lista di %d utenti a %s\n", server->client_count, u->id);
    return;
}

void handle_consu(Server* server, int client_fd, char* msg){
    return;
}

void handle_friend_reply(Server* server, int client_fd, char* msg, bool accepted){return;}

void handle_quit(Server* server, int client_fd, char* msg){
    User* u = get_user_by_fd(server, client_fd);
    if (u == NULL || server == NULL) return;

    char retmsg[9];
    build_gobye(retmsg);
    net_send_str(client_fd, retmsg);
    server_disconnect(server, client_fd);
}