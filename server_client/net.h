#ifndef NET_H
#define NET_H

#include "protocol.h"
#include "msgparsing.h"

 //Crea un socket TCP su porta impostando SO_REUSEADDR
 //Ritorna il fd del socket oppure -1 in caso di errore
int net_create_tcp_server(uint16_t port);


//Crea un socket TCP e si connette a host e port
//Ritorna il fd della connessione opppure -1 in caso di errore
int net_connect_tcp(const char *host, uint16_t port);


 //Crea un socket UDP con  port
 //Ritorna fd del socket oppure -1 in caso di errore
int net_create_udp_socket(uint16_t port);


 //Accetta una connessione TCP in entrata su server_fd, scrive l'ip del cliente su ip_out e la porta su port_out
 //Ritorna il fd della connessione oppure -1 in caso di errore
int net_accept(int server_fd, char *ip_out, uint16_t *port_out);


 //Riceve su fd finche non trova il terminatore (+++) oppure buff è pieno
 //Ritorna il numero di byte ricevuti oppure -1 in caso di errore
int net_recv_msg(int fd, char *buf, int bufsize);


 //Invia buff su fd chiamando send() in loop
 //Ritorna il numero di byte inviati oppure -1 in caso di errore
int net_send(int fd, const char *buf, int len);


 //Chiama net_send con strlen(buff)
 //Ritorna il numero di byte inviati oppure -1 in caso di errore
int net_send_str(int fd, const char *buf);


 //Invia una notifica UDP [YXX] a target
 //Ritorna 0 in caso di successo, -1 in caso di errore
int net_send_udp(const User *target, StreamType type, int stream_count);


 //Chiude il fd e lo imposta a -1
void net_close(int *fd);


//Funzioni di validazione

//Ritorna 0 se id valido, -1 altrimenti
int net_is_valid_id(const char *msg);

//Ritorna 0 se 0 < port < 9999, -1 altrimenti
int net_is_valid_port(const long port);

//Ritorna 0 se 0 < password < 65535, -1 altrimenti
int net_is_valid_password(const long pwd);

//Ritorna 0 se il messaggio e' valido, -1 altrimenti
int net_is_valid_msg(const char *msg);

#endif
