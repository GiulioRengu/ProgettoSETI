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

void print_usage(const char *prog)
{
    fprintf(stderr, "Uso: %s <server_ip> <server_port> <porta_udp>\n", prog);
    fprintf(stderr, "server_ip    indirizzo IP (IPv4 o IPv6) del server\n");
    fprintf(stderr, "server_port  porta TCP del server (< 9999)\n");
    fprintf(stderr, "porta_udp    porta UDP locale su cui ricevere le notifiche (< 9999)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 4){
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 5 && strcmp(argv[4], "-v") == 0) verbose = 1;

    const char *server_ip = argv[1];
    long server_port_l = strtol(argv[2], NULL, 10);
    long udp_port_l = strtol(argv[3], NULL, 10);

    if (net_is_valid_port((uint16_t)server_port_l) != 0){
        fprintf(stderr, "Errore: porta del server non valida (deve essere compresa tra 1 e 9999).\n");
        return EXIT_FAILURE;
    }

    if (net_is_valid_port((uint16_t)udp_port_l) != 0){
        fprintf(stderr, "Errore: porta UDP non valida (deve essere compresa tra 1 e 9999).\n");
        return EXIT_FAILURE;
    }

    Client client;
    global_client = &client;

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0){
        perror("Errore sigaction");
        return EXIT_FAILURE;
    }

    if (client_start(&client, (uint16_t)udp_port_l) != 0){
        fprintf(stderr, "Errore: impossibile avviare il client.\n");
        return EXIT_FAILURE;
    }

    if (client_connect(&client, server_ip, (uint16_t)server_port_l) != 0){
        fprintf(stderr, "Errore: impossibile connettersi al server %s:%ld.\n", server_ip, server_port_l);
        client_cleanup(&client);
        return EXIT_FAILURE;
    }

    printf("Connesso al server %s:%ld (porta UDP locale: %ld). Usa 'regis' o 'conne' per autenticarti.\n",
            server_ip, server_port_l, udp_port_l);

    client_run(&client);
    client_cleanup(&client);

    return EXIT_SUCCESS;
}
