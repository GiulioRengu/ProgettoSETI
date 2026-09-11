#ifndef SERVER_H
#define SERVER_H

#include "../server_client/protocol.h"
#include "serverHandlers.h"
#include <sys/select.h>

// Inizializza il server mettendolo in ascolto su una porta TCP (inferiore a 9999)
int server_init(Server *server, uint16_t port);

// Loop principale del server (gestione select per client multipli)
void server_run(Server *server);

// Accetta un nuovo client TCP
void server_accept_client(Server *server);

// Legge e processa un messaggio da un file descriptor client
void server_handle_client_msg(Server *server, int client_fd);

//Ritorna 0 se la porta e' disponibile, -1 altrimenti
int is_port_available(uint16_t port, Server* server); 

// Funzioni di utilità interne per il Server
User* get_user_by_id(Server *server, const char *id);
User* get_user_by_fd(Server *server, int fd);

// Invia una notifica UDP [YXX] al client
int server_send_udp_notification(Server *server, User *user, StreamType type);

// Aggiunge un flusso alla lista dell'utente e manda notifica
void server_add_stream(Server *server, User *user, Stream *new_stream);

void server_disconnect(Server *server, int fd);

void server_remove_client(Server* server, int client_fd);

// Pulizia risorse server
void server_cleanup(Server *server);

void handle_sigint(int sig);

#endif