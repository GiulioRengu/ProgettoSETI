#ifndef CLIENT_H
#define CLIENT_H

#include "../server_client/protocol.h"

typedef struct {
    char id[ID_LENGTH+1];
    uint16_t password;
    uint16_t udp_port;
    int tcp_fd;
    int udp_fd;
    bool auth;
    bool pending_friend_reply; // EIRF> ricevuto, risposta ancora da inviare
    bool awaiting_ackrf;       // risposta inviata, in attesa di ACKRF
} Client;

//Stampa il menu principale
void print_menu();

//Inizializzazione client (id/password vengono impostati in seguito con regis/conne)
int client_start(Client *client, uint16_t udp_port);

//Connessione TCP al server
int client_connect(Client *client, const char* server_ip, uint16_t server_port);

//Gestisce comando di autenticazione (conne/regis)
void client_handle_auth_command(Client *client, const char *line, bool registration);

//Gestisce una linea di input del cliente
int client_handle_stdin_line(Client *client);

//Gestisce un messaggio EIRF> ricevuto dal server
void client_handle_server_message(Client *client, const char *buf, int len);

//Loop principale del client (gestisce stdin, TCP e UDP)
void client_run(Client *client);

//Disconnessione
void client_cleanup(Client *client);

//IQUIT+++
void client_disconnect(Client *client);

#endif
