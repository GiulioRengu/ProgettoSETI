#include "client.h"
#include "../server_client/net.h"
#include "../server_client/msgparsing.h"

static void print_menu(void)
{
    printf("\n--- Comandi disponibili ---\n");
    printf("regis                 -> registra questo utente (id/pass/porta UDP gia' impostati)\n");
    printf("conne                 -> connettiti/ri-registrati con id/password gia' impostati\n");
    printf("frie <id>             -> invia richiesta di amicizia a <id>\n");
    printf("mess <id> <testo>     -> invia messaggio a <id>\n");
    printf("floo <testo>          -> invia messaggio in flood a tutti gli amici\n");
    printf("list                  -> richiedi lista utenti registrati\n");
    printf("consu                 -> consulta il prossimo stream in sospeso\n");
    printf("acc                   -> accetta la richiesta di amicizia pendente\n");
    printf("rej                   -> rifiuta la richiesta di amicizia pendente\n");
    printf("quit                  -> disconnettiti e chiudi il client\n");
    printf("---------------------------\n> ");
    fflush(stdout);
}

int client_start(Client *client, const char *id, uint16_t password, uint16_t udp_port)
{
    if (client == NULL || id == NULL) return -1;

    memset(client, 0, sizeof(Client));
    strncpy(client->id, id, ID_LENGTH);
    client->id[ID_LENGTH] = '\0';
    client->password = password;
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

    if (strcmp(cmd, "regis") == 0){
        build_regis(buff, client->id, client->udp_port, client->password);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "conne") == 0){
        build_conne(buff, client->id, client->password);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "frie") == 0){
        sscanf(line, "%*s %8s", arg1);
        build_frie_req(buff, arg1);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "mess") == 0){
        char msgtxt[MSG_LENGTH_MAX] = {0};
        sscanf(line, "%*s %8s %[^\n]", arg1, msgtxt);
        build_mess_req(buff, arg1, msgtxt);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "floo") == 0){
        char msgtxt[MSG_LENGTH_MAX] = {0};
        sscanf(line, "%*s %[^\n]", msgtxt);
        build_floo_req(buff, msgtxt);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "list") == 0){
        build_list_req(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "consu") == 0){
        build_consu(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "acc") == 0){
        build_okirf(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if (strcmp(cmd, "rej") == 0){
        build_nokrf(buff);
        net_send_str(client->tcp_fd, buff);
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

void client_run(Client *client)
{
    if (client == NULL || client->tcp_fd < 0 || client->udp_fd < 0) return;

    fd_set master, read_fds;
    int fdmax = client->tcp_fd > client->udp_fd ? client->tcp_fd : client->udp_fd;

    FD_ZERO(&master);
    FD_SET(STDIN_FILENO, &master);
    FD_SET(client->tcp_fd, &master);
    FD_SET(client->udp_fd, &master);

    print_menu();

    while (1){
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0){
            printf("Errore select | Reason: %s\n", strerror(errno));
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)){
            if (handle_stdin_line(client) < 0){
                client_disconnect(client);
                break;
            }
        }

        if (FD_ISSET(client->tcp_fd, &read_fds)){
            char buf[MSG_BUFF_MAXSIZE];
            int r = net_recv_msg(client->tcp_fd, buf, MSG_BUFF_MAXSIZE);
            if (r <= 0){
                printf("\nIl server ha chiuso la connessione.\n");
                break;
            }
            printf("\n[SERVER] %s\n> ", buf);
            fflush(stdout);
        }

        if (FD_ISSET(client->udp_fd, &read_fds)){
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
}

