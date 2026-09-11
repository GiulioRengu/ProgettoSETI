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

// inizializziamo il client (id/password vengono impostati in seguito con regis/conne)
int client_start(Client *client, uint16_t udp_port);

// connessione TCP al server
int client_connect(Client *client, const char* server_ip, uint16_t server_port);

// Loop principale del client (gestisce stdin, TCP e UDP)
void client_run(Client *client);

// disconnessione
void client_cleanup(Client *client);

// IQUIT+++
void client_disconnect(Client *client);

#endif
