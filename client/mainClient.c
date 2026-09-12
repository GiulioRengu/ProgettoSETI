#define _POSIX_C_SOURCE 200809L
#include "client.h"
#include "../server_client/net.h"
#include "../server_client/msgparsing.h"

static void print_menu(void)
{
    puts("------------------------------------------------------------------------");
    puts("                                  IPortBook");
    puts("------------------------------------------------------------------------");
    puts("REGIS <id> <port> <password>  registra un nuovo utente");
    puts("CONNE <id> <password>         riconnettiti con un utente registrato");
    puts("FRIE? <id>                    richiedi un'amicizia");
    puts("MESS? <id> <mess>             invia un messaggio a un amico");
    puts("FLOO? <mess>                  invia un messaggio di flood");
    puts("LIST?                         elenca gli utenti registrati");
    puts("CONSU                         consulta un solo flusso");
    puts("OKIRF                         accetta la richiesta ricevuta con EIRF>");
    puts("NOKRF                         rifiuta la richiesta ricevuta con EIRF>");
    puts("IQUIT                         disconnettiti e attendi GOBYE");
    puts("HELP                          mostra questi comandi (solo locale)");
    puts("Id: 8 caratteri alfanumerici; password: 0-65535; porta: 1-9999");
    puts("Messaggi: massimo 200 caratteri, senza +++");
    puts("Scrivi i comandi senza +++, il client aggiunge il terminatore");
    puts("------------------------------------------------------------------------");
}

int client_start(Client *client, uint16_t udp_port)
{
    if (client == NULL) return -1;

    memset(client, 0, sizeof(Client));
    client->id[0] = '\0';
    client->password = 0;
    client->udp_port = udp_port;
    client->auth = false;
    client->tcp_fd = -1;

    client->udp_fd = net_create_udp_socket(udp_port);
    if (client->udp_fd < 0){
        printf("Errore client_start: impossibile creare/bindare il socket UDP sulla porta %u\n", udp_port);
        return -1;
    }

    return 0;
}

int client_connect(Client *client, const char *server_ip, uint16_t server_port)
{
    if (client == NULL || server_ip == NULL) return -1;

    client->tcp_fd = net_connect_tcp(server_ip, server_port);
    if (client->tcp_fd < 0){
        printf("Errore client_connect: connessione a %s:%u fallita\n", server_ip, server_port);
        return -1;
    }

    return 0;
}

static void handle_auth_command(Client *client, const char *line, bool registration)
{
    char id[10], port_str[6], pwd_str[7];
    char extra, buff[MSG_BUFF_MAXSIZE];
    /* Read complete tokens and reject missing or extra arguments. */
    int fields = registration ? sscanf(line, "%*s %9s %5s %6s %c", id, port_str, pwd_str, &extra) : sscanf(line, "%*s %9s %6s %c", id, pwd_str, &extra);
    if (fields != (registration ? 3 : 2))
    { 
        printf(registration ? "Uso: regis <id> <port> <password>" : "Uso: conne <id> <password>\n");
        return;
    }
    if (net_is_valid_id(id) != 0){
        printf("Id non valido: deve essere alfanumerico ed esattamente di 8 caratteri\n");
        return;
    }
    
    // uint16_t port=client->udp_port;
    uint16_t password=atol(pwd_str);
    uint16_t udp_port=atol(port_str);
    if (registration && net_is_valid_port(udp_port) != 0){
        printf("Porta UDP non valida: deve essere compresa tra 1 e 9999\n");
        return;
    }
    if (net_is_valid_password(password) != 0){
        printf("Password non valida: deve essere compresa tra 0 e 65535\n");
        return;
    }
    if (registration && (udp_port != client->udp_port || client->udp_fd < 0)){
        /* Bind before advertising the port; retain the old socket on failure. */
        int udp_fd = net_create_udp_socket(udp_port);
        if (udp_fd < 0){
            printf("Impossibile usare la porta UDP richiesta\n");
            return;
        }
        if (client->udp_fd >= 0) close(client->udp_fd);
        client->udp_fd = udp_fd;
        client->udp_port = udp_port;
    }
    strcpy(client->id, id);
    client->password = password;
    int len = registration ? build_regis(buff, id, udp_port, password) : build_conne(buff, id, password);
    net_send(client->tcp_fd, buff, len);
}

/* interpreta una riga digitata dall'utente e la manda al server */
static int handle_stdin_line(Client *client)
{
    char line[MSG_LENGTH_MAX + ID_LENGTH + 32];
    if (fgets(line, sizeof(line), stdin) == NULL) return -1;

    /* rimuove il newline finale */
    line[strcspn(line, "\n")] = '\0';

    char cmd[16] = {0};
    char arg1[ID_LENGTH + 1] = {0};
    char buff[MSG_BUFF_MAXSIZE];

    sscanf(line, "%15s", cmd);

    if (strcmp(cmd, "REGIS") == 0 || strcmp(cmd, "CONNE") == 0){
        handle_auth_command(client, line, strcmp(cmd, "REGIS") == 0);
    }
    else if (strcmp(cmd, "FRIE?") == 0){
        sscanf(line, "%*s %8s", arg1);
        build_frie_req(buff, arg1);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "MESS?") == 0){
        char msgtxt[MSG_LENGTH_MAX] = {0};
        sscanf(line, "%*s %8s %[^\n]", arg1, msgtxt);
        build_mess_req(buff, arg1, msgtxt);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "FLOO?") == 0){
        char msgtxt[MSG_LENGTH_MAX] = {0};
        sscanf(line, "%*s %[^\n]", msgtxt);
        build_floo_req(buff, msgtxt);
        printf("%s\n",buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "LIST?") == 0){
        build_list_req(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "CONSU") == 0){
        if (client->pending_friend_reply){
            prinft("Rispondi prima alla richiesta con OKIRF o NOKRF\n");
        }
        else if (client->awaiting_ackrf){
            printf("Attendi ACKRF dal server\n");
        }
        else{
            build_consu(buff);
            net_send_str(client->tcp_fd, buff);
        }
    }
    else if (strcmp(cmd, "OKIRF") == 0 || strcmp(cmd, "NOKRF") == 0){
        if (client->awaiting_ackrf){
            printf("Attendi ACKRF dal server\n");
        }
        else if (!client->pending_friend_reply){
            printf("Nessuna richiesta EIRF> a cui rispondere\n");
        }
        else{
            if (strcmp(cmd, "OKIRF") == 0)
                build_okirf(buff);
            else
                build_nokrf(buff);
            if (net_send_str(client->tcp_fd, buff) >= 0){
                client->pending_friend_reply = false;
                client->awaiting_ackrf = true;
            }
        }
    }
    else if(strcmp(cmd, "IQUIT")==0)
    {
        build_iquit(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if(strcmp(cmd, "HELP")==0)
    {
        print_menu();
    }
    else if (strcmp(cmd, "quit") == 0){
        return -1;
    }
    else if (strlen(cmd) > 0){
        printf("Comando sconosciuto: %s\n", cmd);
    }

    printf("> ");
    fflush(stdout);
    return 0;
}

static void handle_server_message(Client *client, const char *buf, int len)
{
    /* Enable replies only for a complete EIRF> <8-character id>+++ frame. */
    if(len==6+ID_LENGTH+3 && memcmp(buf, "EIRF> ", 6)==0 && memcmp(buf+6+ID_LENGTH, "+++", 3)==0){
        char id[ID_LENGTH+1];
        memcpy(id, buf + 6, ID_LENGTH);
        id[ID_LENGTH]='\0';
        if(net_is_valid_id(id)==0 && !client->awaiting_ackrf){
            client->pending_friend_reply=true;
            puts("Richiesta di amicizia ricevuta: rispondi con OKIRF o NOKRF");
        }
    }
    else if(len==8 && memcmp(buf, "ACKRF+++", 8)==0){
        client->awaiting_ackrf=false;
    }
    // else if(len==8 && memcmp(buf, "GOBYE+++", 8)==0){
    //     client->pending_friend_reply=false;
    //     client->awaiting_ackrf=false;
    // }
}

void client_run(Client *client)
{
    if (client == NULL || client->tcp_fd < 0 || client->udp_fd < 0) return;

    print_menu();

    while (1){
        /* REGIS may replace the UDP socket, so rebuild the set each time. */
        int selected_udp_fd = client->udp_fd;
        int fdmax = client->tcp_fd > selected_udp_fd ? client->tcp_fd : selected_udp_fd;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client->tcp_fd, &read_fds);
        FD_SET(selected_udp_fd, &read_fds);
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0){
            printf("Errore select, motivo: %s\n", strerror(errno));
            break;
        }

        if (FD_ISSET(client->tcp_fd, &read_fds)){
            char buf[MSG_BUFF_MAXSIZE];
            int r = net_recv_msg(client->tcp_fd, buf, MSG_BUFF_MAXSIZE);
            if (r <= 0){
                printf("\nIl server ha chiuso la connessione.\n");
                break;
            }
            printf("\n[SERVER] %s\n", buf);
            handle_server_message(client, buf, r);
            printf("> ");
            fflush(stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)){
            if (handle_stdin_line(client) < 0){
                client_disconnect(client);
                break;
            }
        }

        if (client->udp_fd == selected_udp_fd && FD_ISSET(selected_udp_fd, &read_fds)){
            char buf[8];
            struct sockaddr_storage src_addr;
            socklen_t addr_len = sizeof(src_addr);
            ssize_t n = recvfrom(client->udp_fd, buf, sizeof(buf) - 1, 0,
                                  (struct sockaddr *)&src_addr, &addr_len);
            if (n > 0){
                buf[n] = '\0';
                printf("\n[NOTIFICA UDP] %s\n> ", buf);
                fflush(stdout);
            }
        }
    }
    client->pending_friend_reply = false;
    client->awaiting_ackrf = false;
}

void client_disconnect(Client *client)
{
    if (client == NULL || client->tcp_fd < 0) return;
    char buf[9];
    build_iquit(buf);
    net_send_str(client->tcp_fd, buf);
}

void client_cleanup(Client *client)
{
    if (client == NULL) return;
    if (client->tcp_fd >= 0) close(client->tcp_fd);
    if (client->udp_fd >= 0) close(client->udp_fd);
    client->tcp_fd = -1;
    client->udp_fd = -1;
    client->auth=0;
    client->pending_friend_reply = false;
    client->awaiting_ackrf = false;
}

/* ═══════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════ */

static Client *global_client = NULL;

static void handle_sigint(int sig)
{
    (void)sig;
    printf("\nRicevuto SIGINT (Ctrl+C). Disconnessione in corso\n");
    if (global_client != NULL){
        client_disconnect(global_client);
        client_cleanup(global_client);
    }
    exit(0);
}

static void print_usage(const char *prog)
{
    fprintf(stderr, "Uso: %s <server_ip> <server_port> <porta_udp>\n", prog);
    fprintf(stderr, "server_ip    indirizzo IP (IPv4 o IPv6) del server\n");
    fprintf(stderr, "server_port  porta TCP del server (< 9999)\n");
    fprintf(stderr, "porta_udp    porta UDP locale su cui ricevere le notifiche (< 9999)\n");
}

int main(int argc, char *argv[])
{
    if (argc != 4){
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

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
