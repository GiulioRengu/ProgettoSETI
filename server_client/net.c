#include <net.h>

/* ═══════════════════════════════════════════════════════════
 * SETUP SOCKET
 * ═══════════════════════════════════════════════════════════ */

int net_create_tcp_server(uint16_t port);


int net_connect_tcp(const char *host, uint16_t port);


int net_create_udp_socket(void);


/* ═══════════════════════════════════════════════════════════
 * ACCETTAZIONE CONNESSIONI (server)
 * ═══════════════════════════════════════════════════════════ */

int net_accept(int server_fd, char *ip_out, uint16_t *port_out);


/* ═══════════════════════════════════════════════════════════
 * RICEZIONE TCP
 * ═══════════════════════════════════════════════════════════ */

int net_recv_msg(int fd, char *buf, int bufsize);


/* ═══════════════════════════════════════════════════════════
 * INVIO TCP
 * ═══════════════════════════════════════════════════════════ */

int net_send(int fd, const char *buf, int len);


int net_send_str(int fd, const char *buf);


/* ═══════════════════════════════════════════════════════════
 * INVIO UDP (notifiche server → client)
 * ═══════════════════════════════════════════════════════════ */

int net_send_udp(int udp_fd, const User *target, StreamType type, int stream_count);


/* ═══════════════════════════════════════════════════════════
 * CHIUSURA
 * ═══════════════════════════════════════════════════════════ */

void net_close(int *fd);


/* ═══════════════════════════════════════════════════════════
 * CONTROLLI
 * ═══════════════════════════════════════════════════════════ */

int net_is_valid_id(char *msg);

int net_is_valid_port(char *msg);

int net_is_valid_password(char *msg);

int net_is_valid_msg(char *msg);