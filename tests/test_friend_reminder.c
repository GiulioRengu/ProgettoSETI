#include <assert.h>
#include <sys/time.h>
#include "../server/server.h"
#include "../server/streamHandlers.h"

static int connect_peer(Server *server, int *fd)
{
    int pair[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    struct timeval timeout = {.tv_sec = 1};
    assert(setsockopt(pair[0], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0);
    *fd = pair[1];
    FD_SET(*fd, &server->master_fds);
    server->fdmax = *fd;
    strcpy(server->pendingClients[*fd], "::1");
    return pair[0];
}

static void expect_frame(int peer, const char *expected)
{
    char frame[MSG_BUFF_MAXSIZE];
    assert(net_recv_msg(peer, frame, sizeof(frame)) == (int)strlen(expected));
    assert(strcmp(frame, expected) == 0);
}

static void reconnect(Server *server, int fd, int peer, uint16_t password)
{
    char frame[MSG_BUFF_MAXSIZE];
    int len = build_conne(frame, "bob12345", password);
    assert(net_send(peer, frame, len) == len);
    server_handle_client_msg(server, fd);
}

static void check_reminder(bool accepted)
{
    Server *server = calloc(1, sizeof(*server));
    assert(server != NULL);
    server->tcp_fd = -1;
    server->client_count = 2;
    User *bob = &server->users[0], *alice = &server->users[1];
    strcpy(bob->id, "bob12345");
    bob->password = 12345;
    strcpy(alice->id, "alice123");
    strcpy(alice->ip, "::1");
    alice->udp_port = 4321;
    alice->tcp_fd = -1;
    alice->connected = -1;
    int fd;
    int peer = connect_peer(server, &fd);
    bob->tcp_fd = fd;
    bob->connected = 0;
    assert(stream_add(bob, alice->id, NULL, STREAM_FRIEND_REQ) == 0);
    handle_consu(server, fd);
    expect_frame(peer, "EIRF> alice123+++");
    assert(bob->pending_frie_req);
    assert(bob->stream_count == 0);
    server_disconnect(server, fd);
    close(peer);

    // A failed login must not reveal or discard the unanswered request.
    peer = connect_peer(server, &fd);
    reconnect(server, fd, peer, 12346);
    expect_frame(peer, "GOBYE+++");
    char byte;
    assert(recv(peer, &byte, 1, 0) == 0);
    assert(bob->pending_frie_req);
    close(peer);

    // Repeat a dropped connection: reminders must not consume the request.
    for (int attempt = 0; attempt < 2; attempt++){
        peer = connect_peer(server, &fd);
        reconnect(server, fd, peer, 12345);
        expect_frame(peer, "HELLO+++");
        expect_frame(peer, "EIRF> alice123+++");
        assert(bob->pending_frie_req);
        assert(strcmp(bob->pending_frie_id, "alice123") == 0);
        assert(bob->stream_count == 0);
        if (attempt == 0){
            close(peer);
            server_handle_client_msg(server, fd);
            assert(bob->connected == -1);
        }
    }
    handle_friend_reply(server, fd, accepted);
    expect_frame(peer, "ACKRF+++");
    assert(!bob->pending_frie_req);
    assert(bob->friend_count == (accepted ? 1 : 0));
    assert(alice->streams->type == (accepted ? STREAM_FRIEND_ACC : STREAM_FRIEND_REJ));
    server_disconnect(server, fd);
    close(peer);

    // Once answered, a later login sends HELLO without an EIRF reminder.
    peer = connect_peer(server, &fd);
    reconnect(server, fd, peer, 12345);
    expect_frame(peer, "HELLO+++");
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(peer, &readable);
    struct timeval timeout = {0};
    assert(select(peer + 1, &readable, NULL, NULL, &timeout) == 0);
    server_disconnect(server, fd);
    close(peer);
    server_cleanup(server);
    free(server);
}

int main(void)
{
    check_reminder(true);
    check_reminder(false);
    puts("Friend reminder: reconnect, repeat disconnect, wrong password, accept and reject passed.");
    return 0;
}
