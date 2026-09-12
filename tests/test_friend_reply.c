#include <assert.h>
#include <sys/time.h>

#define main client_program_main
#include "../client/mainClient.c"
#undef main
#include "../server/serverHandlers.h"

static void command(Client *client, const char *line)
{
    FILE *input = tmpfile();
    assert(input != NULL);
    fputs(line, input);
    rewind(input);
    int saved = dup(STDIN_FILENO);
    assert(saved >= 0);
    assert(dup2(fileno(input), STDIN_FILENO) >= 0);
    clearerr(stdin);
    assert(handle_stdin_line(client) == 0);
    assert(dup2(saved, STDIN_FILENO) >= 0);
    clearerr(stdin);
    close(saved);
    fclose(input);
}

static void expect_no_wire(int fd)
{
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(fd, &readable);
    struct timeval timeout = {0};
    assert(select(fd + 1, &readable, NULL, NULL, &timeout) == 0);
}

static void receive_frame(Client *client, int peer, const char *frame)
{
    assert(net_send_str(peer, frame) == (int)strlen(frame));
    char buf[MSG_BUFF_MAXSIZE];
    int len = net_recv_msg(client->tcp_fd, buf, sizeof(buf));
    assert(len == (int)strlen(frame));
    handle_server_message(client, buf, len);
}

int main(void)
{
    int sockets[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    struct timeval timeout = {.tv_sec = 1};
    for (int i = 0; i < 2; i++)
        assert(setsockopt(sockets[i], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0);
    Client client = {.tcp_fd = sockets[0], .udp_fd = -1};
    int peer = sockets[1];
    const char *replies[] = {"okirf\n", "nokrf\n", "OKIRF\n", "NOKRF\n"};
    for (size_t i = 0; i < 4; i++){
        command(&client, replies[i]);
        expect_no_wire(peer);
    }
    const char *unrelated[] = {"FRIE>+++", "ACKRF+++", "NOCON+++", "EIRF> bad+++", "EIRF> abc!1234+++"};
    for (size_t i = 0; i < sizeof(unrelated) / sizeof(unrelated[0]); i++){
        receive_frame(&client, peer, unrelated[i]);
        command(&client, "okirf\n");
        expect_no_wire(peer);
    }
    for (size_t i = 0; i < 4; i++){
        receive_frame(&client, peer, "EIRF> alice123+++");
        command(&client, "consu\n");
        expect_no_wire(peer);
        command(&client, replies[i]);
        char wire[MSG_BUFF_MAXSIZE];
        assert(net_recv_msg(peer, wire, sizeof(wire)) == 8);
        assert(strcmp(wire, i % 2 == 0 ? "OKIRF+++" : "NOKRF+++") == 0);
        command(&client, "okirf\n");
        command(&client, "nokrf\n");
        command(&client, "consu\n");
        expect_no_wire(peer);
        receive_frame(&client, peer, "ACKRF+++");
        command(&client, "okirf\n");
        expect_no_wire(peer);
        command(&client, "consu\n");
        assert(net_recv_msg(peer, wire, sizeof(wire)) == 8);
        assert(strcmp(wire, "CONSU+++") == 0);
    }
    receive_frame(&client, peer, "EIRF> alice123+++");
    receive_frame(&client, peer, "GOBYE+++");
    command(&client, "nokrf\n");
    expect_no_wire(peer);

    /* Invalid replies must also be rejected by the server without using
     * an uninitialized response buffer or dereferencing a missing user. */
    Server *server = calloc(1, sizeof(*server));
    assert(server != NULL);
    handle_friend_reply(server, peer, true);
    expect_no_wire(client.tcp_fd);
    server->client_count = 1;
    strcpy(server->users[0].id, "bob12345");
    server->users[0].tcp_fd = peer;
    handle_friend_reply(server, peer, false);
    expect_no_wire(client.tcp_fd);
    free(server);
    client_cleanup(&client);
    close(peer);
    puts("Friend replies: EIRF gating, one reply per request, ACKRF and server rejection passed.");
    return 0;
}
