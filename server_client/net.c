#include "net.h"

/* ═══════════════════════════════════════════════════════════
 * SETUP SOCKET
 * ═══════════════════════════════════════════════════════════ */

int net_create_tcp_server(uint16_t port)
{
    int fd;
    struct sockaddr_in6 server_addr;

    fd=socket(AF_INET6, SOCK_STREAM, 0);
    if(fd<0)
    {
        printf("Errore nella creazione del socket TCP | port %u Reason: %s", port, strerror(errno));
        return -1;
    }

    int opt=1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0)
    {
        printf("Errore nella setsockopt SO_REUSEADDR | fd: %d, port: %u Reason: %s", fd, port, strerror(errno));
        return -1;
    }

    int v6only=0;
    bool dual_stack=true;
    if(setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only))<0)
    {
        printf("Supporto dual-stack non abilitato | fd: %d, port: %u Reason: %s",fd,port,strerror(errno));
        dual_stack=false;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family=AF_INET6;
    server_addr.sin6_port=htons(port);
    server_addr.sin6_addr=in6addr_any;

    if(bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
    {
        printf("Errore bind TCP | fd: %d, port: %u Reason: %s", fd, port, strerror(errno));
        close(fd);
        return -1;
    }

    if(listen(fd, SOMAXCONN)<0)
    {
        printf("Errore listen TCP | fd: %d, port: %u Reason: %s", fd, port, strerror(errno));
        close(fd);
        return -1;
    }

    printf("TCP server in ascolto | port: %u, protocol: %s", port, dual_stack ? "IPv4/IPv6" : "IPv6");

    return fd;
}


int net_connect_tcp(const char *host, uint16_t port)
{
    int fd;
    struct sockaddr_in6 server_addr;

    fd=socket(AF_INET6, SOCK_STREAM, 0);
    if(fd<0)
    {
        printf("Errore nella creazione del socket TCP | Port: %d Reason: %s", port, strerror(errno));
        close(fd);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family=AF_INET6;
    server_addr.sin6_port=htons(port);

    int ret=inet_pton(AF_INET6, host, &server_addr.sin6_addr);
    if(ret==0)
    {
        printf("Indirizzo invalido | host %s", host);
        close(fd);
        return -1;
    }

    if(ret<0)
    {
        printf("Errore pton Reason: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
   {
        printf("Errore connect | fd: %d, host: %s Reason %s", fd, host, strerror(errno));
        close(fd);
        return -1;
   }

    return fd;

}


int net_create_udp_socket(uint16_t port){
    //controllo porta prima della funzione oppure da inserire in funzione
    struct sockaddr_in6 server_addr;
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0){
        printf("Errore net_create_udp_socket | port: %u Reason: %s", port,strerror(errno));
        return -1;
    }

    int f = 0, r = 0;
    if ((r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &f, sizeof(f))) < 0){ //provo a modificare IPV6_V6ONLY a false per attivare il dual-stack
        printf("Avviso net_create_udp_socket: Dual-Stack non attivo\n");
        f = 1; //imposto f = 1 per ricordarmi che il dual stack non è attivo
    }

    memset(&server_addr, 0, sizeof(server_addr)); //imposto tutti i byte a 0
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);

    if((r = bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr))) < 0){
        printf("Errore net_create_udp_socket: bind\n");
        close(sock);
        return -1;
    }

    return sock;
}


/* ═══════════════════════════════════════════════════════════
 * ACCETTAZIONE CONNESSIONI (server)
 * ═══════════════════════════════════════════════════════════ */

int net_accept(int server_fd, char *ip_out, uint16_t *port_out){
    struct sockaddr_in6 saddr;
    socklen_t size = sizeof(struct sockaddr_in6);

    int new_fd = accept(server_fd, (struct sockaddr*) &saddr, &size);
    if (new_fd < 0){
        printf("Errore net_accept: accept  Reason: %s\n", strerror(errno));
        return -1;
    }

    const char* ntop_res;
    if(IN6_IS_ADDR_V4MAPPED(&saddr.sin6_addr)){ //controlla se indirizzo ip è mappato ipv4 o ipv6
        // se ipv4 estraggo ultimi 4 byte di saddr.sin6_addr
        struct in_addr v4_addr;
        memcpy(&v4_addr, &saddr.sin6_addr.s6_addr[12], 4); //copio ultimi 4 byte da [12] a [15] in v4_addr
        ntop_res = inet_ntop(AF_INET, &v4_addr, ip_out, INET_ADDRSTRLEN);
    }
    else{
        //se ipv6 copio semplicemente da saddr.sin6_addr a ip_out
        ntop_res = inet_ntop(AF_INET6, &saddr.sin6_addr, ip_out, INET6_ADDRSTRLEN);
    }

    if (ntop_res == NULL){
        printf("Errore net_accept: inet_ntop\n");
        close(new_fd);
        return -1;
    }

    printf("Accettata connessione a %s su fd %d\n", ip_out, server_fd);
    *port_out = ntohs(saddr.sin6_port);
    return new_fd;
}


/* ═══════════════════════════════════════════════════════════
 * RICEZIONE TCP
 * ═══════════════════════════════════════════════════════════ */

int net_recv_msg(int fd, char *buf, int bufsize){
    if (buf == NULL || bufsize <= 0) return -1;
    int plus_count = 0, received = 0;
    ssize_t r = 0;

    while(received < bufsize-1){
        r = recv(fd, buf+received, 1, 0); //scrivo in posizione buf[received]
        if (r<0){
            printf("Errore net_recv_msg: recv  Reason: %s\n", strerror(errno));
            close(fd); //forse da rimuovere perchè se ne occupa la select (gemini)
            return -1;
        }

        if(r==0)
        {
            printf("Client disconnesso");
            break;
        }

        if (buf[received++] == '+'){ //incremento received
            plus_count++;
            if (plus_count == 3) break;
        }
        else plus_count = 0;
    }
    buf[received] = '\0';
    return received;
}


/* ═══════════════════════════════════════════════════════════
 * INVIO TCP
 * ═══════════════════════════════════════════════════════════ */

int net_send(int fd, const char *buf, int len)
{
    int bytes_to_send=len, bytes_sent=0, b;
    while(bytes_sent<len)
    {
        b=send(fd, buf+bytes_sent, bytes_to_send, 0);
        if(b==-1)
        {
            printf("Errore durante la send | fd: %d Reason: %s", fd, strerror(errno));
            return -1;
        }
        bytes_sent+=b;
        bytes_to_send-=b;
    }
    return bytes_sent;
}


int net_send_str(int fd, const char *buf)
{
    return net_send(fd, buf, strlen(buf));
}

/* ═══════════════════════════════════════════════════════════
 * INVIO UDP (notifiche server -> client)
 * ═══════════════════════════════════════════════════════════ */

int net_send_udp(/*int udp_fd,*/ const User *target, StreamType type, int stream_count){ //forse da togliere udp_fd
    struct sockaddr_storage dest_addr; //creo indirizzo generico destinatario con sockaddr_storage
    socklen_t ip_len;                  // in modo da poter contenere sia eventuale ipv6 che ipv4
    memset(&dest_addr, 0, sizeof(dest_addr));
    int fd = 0;

    if(strchr(target->ip, ':')){ // controlla se ci sono ':' in ip, se ci sono è mappato ipv6
        struct sockaddr_in6* v6 = (struct sockaddr_in6*) &dest_addr; // casto dest_addr a sockaddr_in6 e modifico i suoi valori utilizzando puntatore v6
        v6->sin6_family = AF_INET6;
        v6->sin6_port = htons(target->udp_port);

        if(inet_pton(AF_INET6, target->ip, &v6->sin6_addr) < 1){
            printf("Errore net_send_udp | Reason: %s", strerror(errno));
            return -1;
        }
        ip_len = sizeof(struct sockaddr_in6);
        fd = socket(AF_INET6, SOCK_DGRAM, 0);
    }

    else{ //indirizzo mappato ipv4, uso sockaddr_in
        struct sockaddr_in* v4 = (struct sockaddr_in*) &dest_addr;// casto dest_addr a sockaddr_in e modifico i suoi valori utilizzando puntatore v4
        v4->sin_family = AF_INET;
        v4->sin_port = htons(target->udp_port);
        
        if(inet_pton(AF_INET, target->ip, &v4->sin_addr) < 1){
            printf("Errore net_send_udp | Reason: %s", strerror(errno));
            return -1;   
        }
        ip_len = sizeof(struct sockaddr_in);
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (fd < 0){
        printf("Errore net_send_udp  Reason: %s", strerror(errno));
        return -1;
    }

    char tosend[4];
    build_udp_notif(tosend, type, stream_count);
    ssize_t sent = sendto(fd, tosend, 3, 0, (struct sockaddr*) &dest_addr, ip_len);
    if (sent != 3){
        printf("Errore net_send_udp | Reason: %s", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

/* ═══════════════════════════════════════════════════════════
 * CONTROLLI
 * ═══════════════════════════════════════════════════════════ */

/**
 * @retval 0 va bene
 * @retval -1 id troppo lungo (max 8 char)
 * @retval 1 id contiene caratteri non alfanumerici
 */
int net_is_valid_id(char *msg){
    if (strlen(msg) != 8) return -1;
    for(unsigned i = 0; i<8; i++) if (!isalnum(msg[i])) return 1;
    return 0;
}

/**
 * @retval 0 va bene
 * @retval -1 non va bene
 */
int net_is_valid_port(uint16_t port){
    return (port>0 && port<9999) ? 0 : -1;
}

/**
 * @retval 0 va bene
 * @retval 1 non va bene
 */
int net_is_valid_password(int pwd){
    return (pwd>0 && pwd<65536) ? 0 : -1;
}

/**
 * @retval -1 se troppo lungo
 * @retval 0 ha già il terminatore
 * @retval 1 va bene
 * 
 */
int net_is_valid_msg(char *msg){
    if (strlen(msg) > MSG_LENGTH_MAX) return -1;

    return (strstr(msg, "+++") != NULL) ? 1 : 0; //se msg ha +++ ritorna -1
}