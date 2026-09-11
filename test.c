#include <assert.h>
#include <sys/time.h>
#include "server_client/net.h"

static void check_auth_message(uint16_t password, bool registration)
{
    char message[MSG_BUFF_MAXSIZE];
    char received[MSG_BUFF_MAXSIZE];
    char expected[MSG_BUFF_MAXSIZE];
    const char *header = registration ? "REGIS testuser 4321 " : "CONNE testuser ";
    int offset = registration ? 20 : 15;
    int expected_len = offset + 2 + 3;
    memcpy(expected, header, offset);
    expected[offset] = password & 0xff;
    expected[offset + 1] = (password >> 8) & 0xff;
    memcpy(expected + offset + 2, "+++", 3);

    int len = registration ? build_regis(message, "testuser", 4321, password)
                           : build_conne(message, "testuser", password);
    assert(len == expected_len);
    assert(memcmp(message, expected, len) == 0);

    int sockets[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    for (int i = 0; i < 2; i++){
        assert(setsockopt(sockets[i], SOL_SOCKET, SO_RCVTIMEO,
                          &timeout, sizeof(timeout)) == 0);
    }

    assert(net_send(sockets[0], message, len) == len);
    // La ricezione deve terminare senza aspettare un secondo comando.
    assert(net_recv_msg(sockets[1], received, sizeof(received)) == expected_len);
    assert(memcmp(received, expected, expected_len) == 0);
    assert(received[expected_len] == '\0');
    build_welco(message);
    assert(net_send_str(sockets[1], message) == 8);
    assert(net_recv_msg(sockets[0], received, sizeof(received)) == 8);
    assert(strcmp(received, "WELCO+++") == 0);

    // Una password contenente '+' non deve lasciare terminatori nel flusso.
    assert(net_send_str(sockets[0], "LIST?+++") == 8);
    assert(net_recv_msg(sockets[1], received, sizeof(received)) == 8);
    assert(strcmp(received, "LIST?+++") == 0);
    close(sockets[0]);
    close(sockets[1]);
}
aa
int main(void)
{
    const uint16_t passwords[] = {
        0, 1, 9, 10, 43, 99, 100, 255, 256, 257, 999, 1000,
        1024, 0x2b00, 0x2b2b, 0x2bff, 65535
    };
    for (size_t i = 0; i < sizeof(passwords) / sizeof(passwords[0]); i++){
        check_auth_message(passwords[i], true);
        check_auth_message(passwords[i], false);
    }
    puts("REGIS/CONNE: password binarie e framing verificati.");
    return 0;
}
