#ifndef SERVERHANDLERS_H
#define SERVERHANDLERS_H

#include "../server_client/protocol.h"
#include "../server_client/msgparsing.h"
#include "../server_client/net.h"
#include "server.h"

// Ogni handler riceve il messaggio già letto da net_recv_msg (dispatch fatto
// in server_handle_client_msg con uno switch su get_type(buf, ...)).
// client_fd serve sempre, anche prima di aver risolto lo User (es. REGIS,
// dove lo User non esiste ancora).

int are_friends(const User* first, const User* second);

void handle_regis(Server* server, int client_fd, char* msg);

void handle_conne(Server* server, int client_fd, char* msg);

void handle_frie(Server* server, int client_fd, char* msg);

void handle_mess(Server* server, int client_fd, char* msg);

void handle_floo(Server* server, int client_fd, char* msg);

void handle_list(Server* server, int client_fd);

void handle_consu(Server* server, int client_fd);

void handle_friend_reply(Server* server, int client_fd, char* msg, bool accepted);

void handle_quit(Server* server, int client_fd);

#endif