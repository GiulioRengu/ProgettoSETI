#define _POSIX_C_SOURCE 200809L
#include "client.h"
#include "../server_client/net.h"
#include "../server_client/msgparsing.h"

void print_menu()
{
    puts("------------------------------------------------------------------------");
    puts("                                  COMANDI");
    puts("------------------------------------------------------------------------");
    puts("regis <id> <password>         registra un nuovo utente");
    puts("conne <id> <password>         riconnettiti con un utente registrato");
    puts("frie <id>                     richiedi un'amicizia");
    puts("mess <id> <mess>              invia un messaggio a un amico");
    puts("floo <mess>                   invia un messaggio di flood");
    puts("list                          elenca gli utenti registrati");
    puts("consu                         consulta un solo flusso");
    puts("okirf                         accetta la richiesta ricevuta con EIRF>");
    puts("nokrf                         rifiuta la richiesta ricevuta con EIRF>");
    puts("iquit                         disconnettiti e attendi GOBYE");
    puts("help                          mostra questi comandi (solo locale)");
    puts("------------------------------------------------------------------------");
    puts("Id: 8 caratteri alfanumerici; password: 0-65535; porta: 1-9999");
    puts("Messaggi: massimo 200 caratteri, senza +++");
    puts("Non necessario inserire terminatore alla fine dei comandi");
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
    
    VERB("Cliente avviato correttamente su porta %u", udp_port);
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
    
    VERB("Cliente connesso al server con IP %s e porta %u", server_ip, server_port);
    return 0;
}

void client_handle_auth_command(Client *client, const char *line, bool registration)
{
    char id[10], pwd_str[7];
    char extra, buff[MSG_BUFF_MAXSIZE];

    int fields = sscanf(line, "%*s %9s %6s %c", id, pwd_str, &extra);
    if (fields != 2)
    { 
        printf(registration ? "Uso: regis <id> <password>\n" : "Uso: conne <id> <password>\n");
        return;
    }
    if (net_is_valid_id(id) != 0){
        printf("Id non valido: deve essere alfanumerico ed esattamente di 8 caratteri\n");
        return;
    }

    char *password_end;
    long password = strtol(pwd_str, &password_end, 10);
    if (password_end == pwd_str || *password_end != '\0' || net_is_valid_password(password) != 0){
        printf("Password non valida: deve essere compresa tra 0 e 65535\n");
        return;
    }
    strcpy(client->id, id);
    client->password = password;
    int len = registration ? build_regis(buff, id, client->udp_port, password) : build_conne(buff, id, password);
    net_send(client->tcp_fd, buff, len);
}

int client_handle_stdin_line(Client *client)
{
    char line[MSG_BUFF_MAXSIZE];
    if (fgets(line, sizeof(line), stdin) == NULL) return -1;

    line[strcspn(line, "\n")] = '\0'; //toglie \n finale

    char cmd[16] = {0};
    char arg1[ID_LENGTH + 1] = {0};
    char buff[MSG_BUFF_MAXSIZE];

    sscanf(line, "%15s", cmd);

    if (strcmp(cmd, "regis") == 0 || strcmp(cmd, "conne") == 0){
        if(client->auth) printf("Errore, non puoi fare regis/conne se sei gia autenticato\n");
        else client_handle_auth_command(client, line, (strcmp(cmd, "regis") == 0));
    }
    else if (!client->auth) printf("Errore, devi prima autenticarti per poter inviare un messaggio\n");
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
        if (client->pending_friend_reply){
            printf("Rispondi prima alla richiesta con OKIRF o NOKRF\n");
        }
        else{
            build_consu(buff);
            net_send_str(client->tcp_fd, buff);
        }
    }
    else if (strcmp(cmd, "okirf") == 0 || strcmp(cmd, "nokrf") == 0){
        if (!client->pending_friend_reply){
            printf("Nessuna richiesta EIRF> a cui rispondere\n");
        }
        else{
            if (strcmp(cmd, "okirf") == 0)
                build_okirf(buff);
            else
                build_nokrf(buff);
            if (net_send_str(client->tcp_fd, buff) >= 0){
                client->pending_friend_reply = false;
            }
        }
    }
    else if(strcmp(cmd, "iquit")==0)
    {
        build_iquit(buff);
        net_send_str(client->tcp_fd, buff);
    }
    else if(strcmp(cmd, "help")==0)
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

void client_handle_eirf(Client *client, const char *buf, int len)
{
    if(len==6+ID_LENGTH+3 && memcmp(buf, "EIRF> ", 6)==0 && memcmp(buf+6+ID_LENGTH, "+++", 3)==0){
        char id[ID_LENGTH+1];
        memcpy(id, buf + 6, ID_LENGTH);
        id[ID_LENGTH]='\0';
        if(net_is_valid_id(id)==0){
            client->pending_friend_reply=true;
            puts("Richiesta di amicizia ricevuta: rispondi con OKIRF o NOKRF");
        }
    }
    if ((len == 8 && memcmp(buf, "WELCO+++", 8) == 0) || (len == 8 && memcmp(buf, "HELLO+++", 8) == 0)){
        client->auth = true;
    }
}

void client_run(Client *client)
{
    if (client == NULL || client->tcp_fd < 0 || client->udp_fd < 0) return;

    print_menu();

while (1){
        int fdmax = client->tcp_fd > client->udp_fd ? client->tcp_fd : client->udp_fd;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client->tcp_fd, &read_fds);
        FD_SET(client->udp_fd, &read_fds);
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0){
            printf("Errore select, motivo: %s\n", strerror(errno));
            break;
        }

        if (FD_ISSET(client->tcp_fd, &read_fds)){
            char buf[MSG_BUFF_MAXSIZE];
            int r = net_recv_msg(client->tcp_fd, buf, MSG_BUFF_MAXSIZE);
            if (r <= 0){
                VERB("Il server ha chiuso la connessione.");
                break;
            }
            printf("\n[SERVER] %s\n", buf);
            client_handle_eirf(client, buf, r);
            printf("> ");
            fflush(stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)){
            if (client_handle_stdin_line(client) < 0){
                client_disconnect(client);
                break;
            }
        }

        if (FD_ISSET(client->udp_fd, &read_fds)){
            char buf[8];
            struct sockaddr_storage src_addr;
            socklen_t addr_len = sizeof(src_addr);
            ssize_t n = recvfrom(client->udp_fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src_addr, &addr_len);
            if (n > 0){
                buf[n] = '\0';
                printf("\n[NOTIFICA UDP] %s\n> ", buf);
                fflush(stdout);
            }
        }
    }
    client->pending_friend_reply = false;
}

void client_disconnect(Client *client)
{
    if (client == NULL || client->tcp_fd < 0) return;
    char buf[9];
    build_iquit(buf);
    net_send_str(client->tcp_fd, buf);
    client->auth = false;
    VERB("Cliente disconnesso correttamente");
}

void client_cleanup(Client *client)
{
    if (client == NULL) return;
    if (client->tcp_fd >= 0) close(client->tcp_fd);
    if (client->udp_fd >= 0) close(client->udp_fd);
    client->tcp_fd = -1;
    client->udp_fd = -1;
    client->auth=false;
    client->pending_friend_reply = false;
}