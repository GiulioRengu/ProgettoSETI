#include "serverHandlers.h"
#include "streamHandlers.h"
#include "floodHandler.h"

int are_friends(const User* first, const User* second){
    int friends_found = 0, i = 0, found = -1;
    while(friends_found < first->friend_count && i < MAX_USERS){
        if (first->friends[i][0] != '\0'){
            friends_found++;
            if (strcmp(first->friends[i], second->id) == 0){
                found = 0;
                break;
            }
        }
        i++;
    }
    return found;
}

void handle_regis(Server* server, int client_fd, char* msg){
    if(server == NULL || client_fd < 0 || msg == NULL) return;

    char retmsg[9];

    char id[ID_LENGTH+1];
    char port[5];
	get_port(msg, port, 15);
    get_id(msg, id);

    uint16_t udp_port = (uint16_t)atoi(port);

    if (net_is_valid_port(udp_port) < 0 || is_port_available(udp_port, server) < 0){
        VERB("Errore REGIS di %s, la porta %u non e' valida o e' gia utilizzata", id, udp_port);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    uint16_t password = (unsigned char)msg[20] | ((unsigned char)msg[21] << 8);

    if (net_is_valid_id(id) != 0){
        VERB("Errore REGIS, id %s non valido", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (get_user_by_id(server, id) != NULL){
        VERB("Errore REGIS: l'utente %s e' gia esistente", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (server->client_count >= MAX_USERS){
        VERB("Errore REGIS: il server ha raggiunto la capacita massima di utenti");
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

    VERB("Utente %s registrato correttamente con ip %s, porta %s e fd %d", id, new_user.ip, port, client_fd);
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
        VERB("Errore CONNE: utente %s non esistente", id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }  
    
    if (target->connected == 0){
        VERB("Errore CONNE: utente %s gia connesso", target->id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }

    if (target->password != password){
        VERB("Errore CONNE: password per %s errata", target->id);
        build_gobye(retmsg);
        net_send_str(client_fd, retmsg);
        server_disconnect(server, client_fd);
        return;
    }


    target->tcp_fd = client_fd;
    target->connected = 0;
    strncpy(target->ip, server->pendingClients[client_fd], INET6_ADDRSTRLEN);
    memset(server->pendingClients[client_fd], 0, INET6_ADDRSTRLEN);

    build_hello(retmsg);
    if(net_send_str(client_fd, retmsg) < 0){
        VERB("Errore CONNE: errore durante l'invio di HELLO+++");
        server_disconnect(server, client_fd);
        return;
    }

    //Il nuovo client deve ricevere di nuovo la richiesta ancora senza risposta.
    if(target->pending_frie_req && target->pending_frie_id[0]!='\0')
    {
        char frie_reminder[6 +ID_LENGTH+3+1]; //eirf> id +++\0
        build_eirf(frie_reminder, target->pending_frie_id);
        if(net_send_str(client_fd, frie_reminder)<0)
        {
            VERB("Errore CONNE: errore invio promemoria EIRF>");
            server_disconnect(server, client_fd);
            return;
        }
    }

    VERB("Utente %s connesso con successo", target->id);
}

void handle_frie(Server* server, int client_fd, char* msg){
    if (server == NULL || client_fd < 0 || msg == NULL) return;
    char retmsg[9]; 

    User* src = get_user_by_fd(server, client_fd);
    char dest_id[ID_LENGTH+1];
    get_id(msg, dest_id);
    User* dest = get_user_by_id(server, dest_id);

    if (src == NULL || dest == NULL){
        VERB("Errore FRIE?: utente/i non esistenti");
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (strcmp(src->id, dest->id) == 0){
        VERB("Errore FRIE?: %s ha provato a inviare una richiesta di amicizia a se stesso", src->id);
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (are_friends(src, dest) == 0){
        VERB("Errore FRIE?: %s e' gia amico di %s", src->id, dest->id);
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (dest->pending_frie_req && (strcmp(dest->pending_frie_id, src->id) == 0)){
        VERB("Errore FRIE?: %s deve ancora accettare/rifiutare una richiesta da parte di %s", dest->id, src->id);
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    Stream* aux = dest->streams;
    while(aux != NULL){
        if (strcmp(aux->from_id, src->id) == 0 && aux->type == STREAM_FRIEND_REQ){
            VERB("Errore FRIE?: %s ha gia mandato una richiesta di amicizia a %s", src->id, dest->id);
            build_frie_ko(retmsg);
            if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
            return;
        }
        aux = aux->next;
    }

    aux = src->streams;
    while(aux != NULL){
        if (strcmp(aux->from_id, dest->id) == 0 && aux->type == STREAM_FRIEND_REQ){
            VERB("Errore FRIE?: %s ha gia ricevuto una richiesta di amicizia da %s", src->id, dest->id);
            build_frie_ko(retmsg);
            if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
            return;
        }
        aux = aux->next;
    }

    if(stream_add(dest, src->id, NULL, STREAM_FRIEND_REQ)<0)
    {
        VERB("Errore FRIE?: la lista di flussi di %s e' piena", dest->id);
        build_frie_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if(server_send_udp_notification(server, dest, STREAM_FRIEND_REQ) < 0){ //serve????? non credo...
        VERB("Errore FRIE?: invio notifica udp a %s non riuscito", dest->id);
    }

    build_frie_ok(retmsg);
    if(net_send_str(client_fd, retmsg) < 0){
        VERB("Errore FRIE?: errore invio FRIE>+++");
        server_disconnect(server, client_fd);
        return;
    }

    VERB("Richiesta di amicizia da %s inviata correttamente a %s", src->id, dest->id);
}

void handle_mess(Server* server, int client_fd, char* msg){
    if (server == NULL || client_fd < 0 || msg == NULL) return;
    char retmsg[9]; 
    char to_send[MSG_LENGTH_MAX+1];

    User* src = get_user_by_fd(server, client_fd);
    char dest_id[ID_LENGTH+1];
    get_id(msg, dest_id);
    User* dest = get_user_by_id(server, dest_id);

    if (src == NULL || dest == NULL){
        VERB("Errore MESS?: utente/i non esistenti");
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    get_msg(msg, to_send, 15);
    if (net_is_valid_msg(to_send) < 0){
        build_mess_ko(retmsg);
        if (net_send_str(client_fd, retmsg) < 0){
            VERB("Errore MESS?: errore durante invio MESS<");
            server_disconnect(server, client_fd);
            return;
        }
        return;
    }

    
    if (strcmp(src->id, dest->id) == 0){
        VERB("Errore MESS?: %s ha provato a inviare un messaggio a se stesso", src->id);
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (are_friends(src, dest) < 0){
        VERB("Errore MESS?: %s non e' amico di %s", src->id, dest->id);
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    if (stream_add(dest, src->id, to_send, STREAM_MSG) < 0){
        VERB("Errore MESS?: lista stream di %s piena", dest->id);
        build_mess_ko(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    server_send_udp_notification(server, dest, STREAM_MSG);
    build_mess_ok(retmsg);
    if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
    VERB("Messaggio da parte di %s a %s inviato con successo", src->id, dest->id);
    return;
}

void handle_floo(Server* server, int client_fd, char* msg)
{
    if(server==NULL || client_fd<0 || msg==NULL)
    {
        return;
    }

    char retmsg[9];
    char to_send[MSG_BUFF_MAXSIZE+1];

    User *sender=get_user_by_fd(server, client_fd);
    if(sender==NULL)return;

    if (sender->friend_count == 0){
        VERB("%s non ha amici su cui propagare la flood", sender->id);
        build_floo_ok(retmsg);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    get_msg(msg, to_send, 6);
    if(net_is_valid_msg(to_send)<0)
    {
        build_floo_ok(retmsg); //??
        VERB("Errore FLOO?: messaggio di %s non valido", sender->id);
        if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
        return;
    }

    VERB("Inizio FLOO?");
    execute_flood_bfs(server, sender, to_send);

    build_floo_ok(retmsg);
    if(net_send_str(client_fd, retmsg) < 0) server_disconnect(server, client_fd);
    VERB("Flood partito da %s completato con successo", sender->id);
    return;

}

void handle_list(Server* server, int client_fd){
    char u_id[24];
    User* u = get_user_by_fd(server, client_fd);
    if (u == NULL || server == NULL) return;

    build_rlist(u_id, server->client_count);
    if (net_send_str(client_fd, u_id) < 0){
        VERB("Errore LIST?: errore invio RLIST"); 
        server_disconnect(server, client_fd);
        return;
    }

    for (int i = 0; i<server->client_count; i++){
        build_linum(u_id, server->users[i].id);
        if (net_send_str(client_fd, u_id) < 0){
            VERB("Errore LIST?: errore invio LINUM, inviati %d", i);
            server_disconnect(server, client_fd);
            return;
        }
    }

    VERB("RLIST eseguito correttamente: inviata lista di %d utenti a %s", server->client_count, u->id);
    return;
}

void handle_consu(Server* server, int client_fd){
    if(server==NULL || client_fd<0) return;

    User*user;
    if((user=get_user_by_fd(server, client_fd))==NULL)
    {
        VERB("Errore CONSU: utente con fd %d non trovato", client_fd);
        return;
    }

    if(user->streams==NULL && !user->has_pending_stream)
    {
        VERB("Errore CONSU: %s non ha flussi da consultare", user->id);
        char retmsg[9];
        build_nocon(retmsg);
        if(net_send_str(client_fd, retmsg)<0) server_disconnect(server, client_fd);
        return;
    }

    Stream *current_stream;
    if (!user->has_pending_stream){
        current_stream=stream_remove(user);
        user->pending_stream = current_stream;
        user->has_pending_stream = true;
    }
    else current_stream = user->pending_stream;

    if(current_stream==NULL)
    {
        VERB("Errore CONSU durante la rimozione del flusso di %s", user->id);
        return;
    }


    char retmsg[MSG_BUFF_MAXSIZE];
    switch (current_stream->type)
    {
        case STREAM_FRIEND_REQ:
            build_eirf(retmsg, current_stream->from_id);
            if(net_send_str(client_fd, retmsg) < 0) 
            {
                VERB("Errore CONSU durante l'invio di EIRF> a %s", user->id);
                goto consu_fail;
            }
            user->pending_frie_req=true;
            strncpy(user->pending_frie_id, current_stream->from_id, ID_LENGTH+1);
            user->pending_frie_id[ID_LENGTH] = '\0';
            break;

        case STREAM_FRIEND_ACC:
            build_frien(retmsg, current_stream->from_id);
            if(net_send_str(client_fd, retmsg) < 0) 
            {
                VERB("Errore CONSU durante l'invio di FRIEN a %s", user->id);
                goto consu_fail;
            }
            break;

        case STREAM_FRIEND_REJ:
            build_nofri(retmsg, current_stream->from_id);
            if(net_send_str(client_fd, retmsg) < 0) 
            {
                VERB("Errore CONSU durante l'invio di NOFRI a %s", user->id);
                goto consu_fail;
            }
            break;

        case STREAM_MSG:
            build_ssem(retmsg, current_stream->from_id, current_stream->msg);
            if(net_send_str(client_fd, retmsg) < 0) 
            {
                VERB("Errore CONSU durante l'invio di SSEM> a %s", user->id);
                goto consu_fail;
            }
            break;

        case STREAM_FLOO:
            build_oolf(retmsg, current_stream->from_id, current_stream->msg);
            if(net_send_str(client_fd, retmsg) < 0) 
            {
                VERB("Errore CONSU durante l'invio di OOLF> a %s", user->id);
                goto consu_fail;
            }
            break;

        default:
            VERB("Errore CONSU: tipo di flusso sconosciuto");
            free(current_stream);
            user->pending_stream = NULL;
            user->has_pending_stream = false;
            return;
    }

    free(current_stream);
    VERB("Flusso di %s consultato correttamente", user->id);
    user->has_pending_stream = false;
    user->pending_stream = NULL;
    return;

    consu_fail:
        server_disconnect(server, client_fd);
        return;
}

void handle_friend_reply(Server* server, int client_fd, const bool accepted){
    if (server == NULL || client_fd < 0) return;

    User* target = get_user_by_fd(server, client_fd);
    if (target == NULL){
        VERB("Errore accettazione amicizia: utente destinatario non trovato");
        return;
    }

    if (!target->pending_frie_req || target->pending_frie_id[0] == '\0'){
        VERB("Errore accettazione amicizia: %s non ha richieste da accettare", target->id);
        goto fine;
    }

    User* requester = get_user_by_id(server, target->pending_frie_id);
    if (requester == NULL){
        VERB("Errore accettazione amicizia: utente richiedente non trovato");
        goto fine;
    }

    char retmsg[9];
    build_ackrf(retmsg);
    int r = stream_add(requester, target->id, NULL, (accepted ? STREAM_FRIEND_ACC : STREAM_FRIEND_REJ));
    if (r < 0){
        VERB("Errore accettazione amicizia: aggiunta stream a %s non riuscita", requester->id);
        goto fine;
 
    }

    if (accepted){
        if (target->friend_count >= MAX_USERS || requester->friend_count >= MAX_USERS){
        VERB("Errore accettazione amicizia: uno dei due utenti ha la lista amici piena");
        goto fine;
        }

        int i = 0;
        while(i < MAX_USERS && target->friends[i][0] != '\0') i++;  
        strncpy(target->friends[i], requester->id, ID_LENGTH);
        target->friends[i][ID_LENGTH] = '\0';
        target->friend_count++;

        i = 0;
        while(i < MAX_USERS && requester->friends[i][0] != '\0') i++;  
        strncpy(requester->friends[i], target->id, ID_LENGTH);
        requester->friends[i][ID_LENGTH] = '\0';
        requester->friend_count++;

        server_send_udp_notification(server, requester, STREAM_FRIEND_ACC);
    }
    else
    {
        server_send_udp_notification(server, requester, STREAM_FRIEND_REJ);

    }

    target->pending_frie_req = false;
    target->pending_frie_id[0] = '\0';
    target->has_pending_stream = false;

    fine:
    if (net_send_str(client_fd, retmsg) < 0){
        VERB("Errore accettazione amicizia: invio ACKRF a %s non riuscito", target->id);
        server_disconnect(server, client_fd);
        return;
    }
    else VERB("Richiesta di amicizia da parte di %s a %s gestita correttamente", requester->id, target->id);

}

void handle_quit(Server* server, int client_fd){
    User* u = get_user_by_fd(server, client_fd);
    if (u == NULL || server == NULL) return;

    char retmsg[9];
    build_gobye(retmsg);
    net_send_str(client_fd, retmsg);
    server_disconnect(server, client_fd);
    VERB("Utente %s disconnesso correttamente", u->id);
}
