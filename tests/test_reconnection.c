#include <assert.h>
#include <sys/time.h>

/* Exercise the actual command parser together with the server handlers. */
#define main client_program_main
#include "../client/mainClient.c"
#undef main
#include "../server/server.h"

static void command(Client *client, const char *line)
{
    FILE *input = tmpfile();
    assert(input != NULL);
    assert(fputs(line, input) >= 0);
    rewind(input);
    int saved_stdin = dup(STDIN_FILENO);
    assert(saved_stdin >= 0);
    assert(dup2(fileno(input), STDIN_FILENO) >= 0);
    clearerr(stdin);
    assert(handle_stdin_line(client) == 0);
    assert(dup2(saved_stdin, STDIN_FILENO) >= 0);
    clearerr(stdin);
    close(saved_stdin);
    fclose(input);
}

static int connect_pair(Server *server, Client *client)
{
    int sockets[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    struct timeval timeout = {.tv_sec = 1};
    for (int i = 0; i < 2; i++)
        assert(setsockopt(sockets[i], SOL_SOCKET, SO_RCVTIMEO,
                          &timeout, sizeof(timeout)) == 0);
    client->tcp_fd = sockets[0];
    strcpy(server->pendingClients[sockets[1]], "::1");
    FD_SET(sockets[1], &server->master_fds);
    server->fdmax = sockets[1];
    return sockets[1];
}

static void expect_reply(Client *client, const char *expected)
{
    char response[MSG_BUFF_MAXSIZE];
    assert(net_recv_msg(client->tcp_fd, response, sizeof(response)) == 8);
    assert(strcmp(response, expected) == 0);
}

static int bind_test_udp(uint16_t *port)
{
    for (uint16_t candidate = 7800; candidate < 9999; candidate++){
        int fd = net_create_udp_socket(candidate);
        if (fd >= 0){
            *port = candidate;
            return fd;
        }
    }
    assert(!"No available UDP test port");
    return -1;
}

static void check_reconnect(unsigned password, bool abrupt)
{
    Server *server = calloc(1, sizeof(*server));
    assert(server != NULL);
    Client client = {0};
    uint16_t registration_port;
    int probe = bind_test_udp(&registration_port);
    close(probe);
    assert(client_start(&client, registration_port) == 0);
    int initial_udp_fd = client.udp_fd;
    int fd = connect_pair(server, &client);
    char line[80];
    snprintf(line, sizeof(line), "regis bob12345 %u\n", password);
    command(&client, line);
    server_handle_client_msg(server, fd);
    expect_reply(&client, "WELCO+++");
    assert(server->users[0].password == password);
    assert(server->users[0].udp_port == registration_port);
    assert(client.udp_port == registration_port);
    assert(client.udp_fd == initial_udp_fd);
    struct sockaddr_in6 bound;
    socklen_t bound_len = sizeof(bound);
    assert(getsockname(client.udp_fd, (struct sockaddr *)&bound, &bound_len) == 0);
    assert(ntohs(bound.sin6_port) == registration_port);

    if (abrupt){
        close(client.tcp_fd);
        server_handle_client_msg(server, fd);
    }
    else{
        client_disconnect(&client);
        server_handle_client_msg(server, fd);
        expect_reply(&client, "GOBYE+++");
        close(client.tcp_fd);
    }
    assert(server->users[0].connected == -1);
    assert(server->users[0].tcp_fd == -1);

    fd = connect_pair(server, &client);
    snprintf(line, sizeof(line), "conne bob12345 %u\n", password ^ 1);
    command(&client, line);
    server_handle_client_msg(server, fd);
    expect_reply(&client, "GOBYE+++");
    close(client.tcp_fd);

    fd = connect_pair(server, &client);
    snprintf(line, sizeof(line), "conne bob12345 %u\n", password);
    command(&client, line);
    server_handle_client_msg(server, fd);
    expect_reply(&client, "HELLO+++");
    assert(server->users[0].password == password);
    assert(server->users[0].connected == 0);
    server_disconnect(server, fd);
    close(client.tcp_fd);
    close(client.udp_fd);
    free(server);
}

int main(void)
{
    /* Reject the old port argument, missing fields and invalid credentials. */
    const char *invalid[] = {
        "regis bob12345 4321 12345 extra\n", "regis bob12345 4321 12345\n",
        "regis bob12345\n", "regis bob123456 12345\n",
        "conne bob123456 12345\n", "regis bob12345 123abc\n",
        "conne bob12345 123abc\n", "regis bob12345 4294967297\n",
        "conne bob12345 4294967297\n",
        "regis bob12345 99999999999999999999999999999999\n",
        "conne bob12345 12345 extra\n", "regis bob12345 -1\n",
        "regis bob12345 65536\n",
        "conne bob12345 65536\n", "regis bob12345 0 12345\n",
        "regis bob12345 9999 12345\n", "regis bob12345 65537 12345\n",
        "regis bob12345 -1 12345\n", "regis bob12345 4321abc 12345\n"
    };
    Server *server = calloc(1, sizeof(*server));
    assert(server != NULL);
    Client client = {0};
    int fd = connect_pair(server, &client);
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++){
        command(&client, invalid[i]);
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(fd, &readable);
        struct timeval timeout = {0};
        assert(select(fd + 1, &readable, NULL, NULL, &timeout) == 0);
    }
    server_disconnect(server, fd);
    close(client.tcp_fd);
    free(server);

    const unsigned passwords[] = {0, 1, 43, 255, 256, 12345, 0x2b00, 0x2b2b, 0x2bff, 65535};
    for (size_t i = 0; i < sizeof(passwords) / sizeof(passwords[0]); i++){
        check_reconnect(passwords[i], false);
        check_reconnect(passwords[i], true);
    }
    puts("Registration validation and reconnection tests passed.");
    return 0;
}
