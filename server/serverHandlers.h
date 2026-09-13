#ifndef SERVERHANDLERS_H
#define SERVERHANDLERS_H

#include "../server_client/protocol.h"
#include "../server_client/msgparsing.h"
#include "../server_client/net.h"
#include "server.h"

//Controlla se first e second sono amici
//Ritorna 0 se sono amici, -1 altrimenti
int are_friends(const User* first, const User* second);

//Gestisce una richiesta di registrazione
void handle_regis(Server* server, int client_fd, char* msg);

//Gestisce una richiesta di connessione
void handle_conne(Server* server, int client_fd, char* msg);

//Gestisce una richiesta di amicizia
void handle_frie(Server* server, int client_fd, char* msg);

//Gestisce l'invio di un messaggio
void handle_mess(Server* server, int client_fd, char* msg);

//Gestisce una richiesta di flood
void handle_floo(Server* server, int client_fd, char* msg);

//Gestisce una richiesta di rlist
void handle_list(Server* server, int client_fd);

//Gestisce una richiesta di consu
void handle_consu(Server* server, int client_fd);

//Gestisce una richiesta di amicizia da parte di chi la riceve
void handle_friend_reply(Server* server, int client_fd, const bool accepted);  

//Gestisce una richiesta di disconnessione
void handle_quit(Server* server, int client_fd);

#endif