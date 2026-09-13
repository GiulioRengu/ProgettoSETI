#ifndef SERVER_H
#define SERVER_H

#include "../server_client/protocol.h"
#include "serverHandlers.h"
#include <sys/select.h>

//Inizializza il server mettendolo in ascolto su una porta TCP (inferiore a 9999)
int server_init(Server *server, uint16_t port);

//Loop principale del server (gestione select per client multipli)
void server_run(Server *server);

//Accetta un nuovo client TCP
void server_accept_client(Server *server);

//Legge e processa un messaggio da un file descriptor client
void server_handle_client_msg(Server *server, int client_fd);

//Controlla se la porta UDP sta gia venendo utilizzata da un altro cliente
//Ritorna 0 se la porta e' disponibile, -1 altrimenti
int is_port_available(uint16_t port, Server* server); 

//Controlla se nel server e' presente un cliente con identificativo id
//Ritorna il puntatore all'utente con quell'id se lo trova, NULL altrimenti
User* get_user_by_id(Server *server, const char *id);

//Controlla se nel server e' presente un cliente con file descriptor fd
//Ritorna il puntatore all'utente con quel fd se lo trova, NULL altrimenti
User* get_user_by_fd(Server *server, int fd);

// Invia una notifica UDP [YXX] a user
int server_send_udp_notification(Server *server, User *user, StreamType type);

// Aggiunge un flusso alla lista dell'utente e manda notifica
void server_add_stream(Server *server, User *user, Stream *new_stream);

//Disconnete il client con fd dal server
void server_disconnect(Server *server, int fd);

//Rimuove il client con client_fd dal server
void server_remove_client(Server* server, int client_fd);

// Pulizia risorse server
void server_cleanup(Server *server);

//Gestisce CTRL C
void handle_sigint(int sig);

#endif