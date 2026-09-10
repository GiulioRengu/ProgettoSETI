#ifndef SERVERHANDLERS_H
#define SERVERHANDLERS_H

#include "../server_client/protocol.h"
#include "../server_client/msgparsing.h"
#include "../server_client/net.h"
#include "server.h"
/*
 * Utility function, checks whether first and second are friends.
 * @retval 0 if friends 
 * @retval -1 if not friends
 */
int are_friends(const User* first, const User* second);

void handle_regis(Server* server, int client_fd, char* msg);

void handle_conne(Server* server, int client_fd, char* msg);

void handle_frie(Server* server, int client_fd, char* msg);

void handle_mess(Server* server, int client_fd, char* msg);

void handle_floo(Server* server, int client_fd, char* msg);

void handle_list(Server* server, int client_fd);

void handle_consu(Server* server, int client_fd);

void handle_friend_reply(Server* server, int client_fd, const bool accepted); //msg 

void handle_quit(Server* server, int client_fd);

#endif