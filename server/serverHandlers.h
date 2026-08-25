#ifndef SERVERHANDLERS_H
#define SERVERHANDLERS_H

#include "../server_client/protocol.h"

//dobbiamo allocare negli handler Stream* new_stream;

void handle_regis();

void handle_conne();

void handle_frie();

void handle_mess();

void handle_floo();

void handle_list();

void handle_consu();

void handle_friend_reply();

void handle_quit();

#endif