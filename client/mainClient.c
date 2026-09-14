#define _POSIX_C_SOURCE 200809L
#include "client.h"
#include "../server_client/net.h"
#include "../server_client/msgparsing.h"

Client *global_client = NULL;

void handle_sigint(int sig)
{
    (void)sig;
    VERB("Ricevuto SIGINT (Ctrl+C). Disconnessione in corso");
    if (global_client != NULL){
        client_disconnect(global_client);
        client_cleanup(global_client);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 4){
        printf("Uso: %s <server-ip> <server-port> <udp-port> <flag -v opzionale>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 5 && strcmp(argv[4], "-v") == 0) verbose = 1;

    const char *server_ip = argv[1];
    long server_port_l = strtol(argv[2], NULL, 10);
    long udp_port_l = strtol(argv[3], NULL, 10);

    if (net_is_valid_port((uint16_t)server_port_l) != 0){
        fprintf(stderr, "Errore: porta del server non valida (deve essere compresa tra 1 e 9999)\n");
        return EXIT_FAILURE;
    }

    if (net_is_valid_port((uint16_t)udp_port_l) != 0){
        fprintf(stderr, "Errore: porta UDP non valida (deve essere compresa tra 1 e 9999)\n");
        return EXIT_FAILURE;
    }

    Client client;
    if (client_start(&client, (uint16_t)udp_port_l) != 0){
        fprintf(stderr, "Errore: impossibile avviare il client\n");
        return EXIT_FAILURE;
    }
    global_client = &client;

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0){
        perror("Errore sigaction");
        return EXIT_FAILURE;
    }


    if (client_connect(&client, server_ip, (uint16_t)server_port_l) != 0){
        fprintf(stderr, "Errore: impossibile connettersi al server con ip %s e porta %lu\n", server_ip, server_port_l);
        client_cleanup(&client);
        return EXIT_FAILURE;
    }

    VERB("Connessione effettuata con successo sul server con ip %s, porta  tcp %lu e porta udp %lu", server_ip, server_port_l, udp_port_l);

    client_run(&client);
    client_cleanup(&client);

    return EXIT_SUCCESS;
}
