#ifndef NET_H
#define NET_H

/*
 * net.h
 *
 * Tutte le operazioni di rete del protocollo IPortbook.
 * Usato sia dal server che dal client (sta in server_client/).
 *
 * Dipende da protocol.h per: User, Server, StreamType, RECV_BUF_SIZE.
 * Dipende da msgparsing.h per: MSG_UDP_BUF.
 *
 * Responsabilità:
 *   - creazione e configurazione socket TCP e UDP
 *   - accettazione connessioni in entrata (server)
 *   - invio e ricezione messaggi TCP
 *   - invio notifiche UDP
 */

#include "protocol.h"
#include "msgparsing.h"

/* ═══════════════════════════════════════════════════════════
 * SETUP SOCKET
 * ═══════════════════════════════════════════════════════════ */

/*
 * Crea, configura e mette in ascolto un socket TCP sulla porta
 * indicata. Imposta SO_REUSEADDR per evitare "Address already
 * in use" al riavvio del server.
 *
 * Ritorna il fd del socket in ascolto, o -1 in caso di errore.
 */
int net_create_tcp_server(uint16_t port);

/*
 * Crea un socket TCP e si connette all'host:port indicati.
 * host è una stringa IPv4 (es. "127.0.0.1").
 *
 * Ritorna il fd della connessione, o -1 in caso di errore.
 */
int net_connect_tcp(const char *host, uint16_t port);

/*
 * Crea un socket UDP da usare per inviare notifiche ai client.
 * Il socket è non bloccante e non è legato a nessuna porta fissa
 * (il SO sceglie una porta effimera).
 *
 * Ritorna il fd del socket UDP, o -1 in caso di errore.
 */
int net_create_udp_socket(void);

/* ═══════════════════════════════════════════════════════════
 * ACCETTAZIONE CONNESSIONI (server)
 * ═══════════════════════════════════════════════════════════ */

/*
 * Accetta una nuova connessione TCP in entrata su server_fd.
 * Scrive l'IP del client in ip_out (buffer di almeno INET_ADDRSTRLEN
 * byte) e la porta sorgente del client in port_out.
 *
 * Ritorna il fd della nuova connessione, o -1 in caso di errore.
 *
 * NOTA: chiamare dopo che select() segnala server_fd come leggibile.
 */
int net_accept(int server_fd, char *ip_out, uint16_t *port_out);

/* ═══════════════════════════════════════════════════════════
 * RICEZIONE TCP
 * ═══════════════════════════════════════════════════════════ */

/*
 * Legge da fd accumulando in buf (bufsize byte) finché non trova
 * MSG_TERMINATOR ("+++") oppure il buffer è pieno.
 *
 * buf viene null-terminato.
 * Ritorna il numero di byte letti (incluso +++), o -1 in caso
 * di errore o connessione chiusa dal peer.
 *
 * ATTENZIONE: TCP è un flusso — un singolo recv() può restituire
 * meno di un messaggio completo, o più di uno. Questa funzione
 * gestisce entrambi i casi chiamando recv() in loop.
 */
int net_recv_msg(int fd, char *buf, int bufsize);

/* ═══════════════════════════════════════════════════════════
 * INVIO TCP
 * ═══════════════════════════════════════════════════════════ */

/*
 * Invia tutti i byte di buf (di lunghezza len) su fd.
 * Chiama send() in loop finché non ha inviato tutto.
 *
 * Ritorna il numero totale di byte inviati, o -1 in caso di errore.
 *
 * NOTA: non usare direttamente send() nel server/client —
 * send() può inviare meno byte del richiesto senza essere un errore.
 */
int net_send(int fd, const char *buf, int len);

/*
 * Comodità: chiama net_send con strlen(buf) come lunghezza.
 * Usare per tutti i messaggi TCP del protocollo (sono stringhe).
 *
 * Ritorna il numero di byte inviati, o -1 in caso di errore.
 */
int net_send_str(int fd, const char *buf);

/* ═══════════════════════════════════════════════════════════
 * INVIO UDP (notifiche server → client)
 * ═══════════════════════════════════════════════════════════ */

/*
 * Invia una notifica UDP di 3 byte [YXX] al client target.
 *
 * udp_fd      : socket UDP del server (da net_create_udp_socket)
 * target      : puntatore all'utente destinatario — usa target->ip
 *               e target->udp_port per costruire l'indirizzo
 * type        : StreamType della notifica (0-4)
 * stream_count: numero di stream non letti del client
 *
 * Ritorna 0 se ok, -1 in caso di errore.
 *
 * NOTA: UDP è fire-and-forget — il client potrebbe non riceverla.
 * Il server non deve bloccarsi ad aspettare conferma.
 */
int net_send_udp(int udp_fd, const User *target, StreamType type, int stream_count);

/* ═══════════════════════════════════════════════════════════
 * CHIUSURA
 * ═══════════════════════════════════════════════════════════ */

/*
 * Chiude il fd e lo imposta a -1 tramite il puntatore.
 * Uso tipico: net_close(&user->tcp_fd);
 * Dopo la chiamata user->tcp_fd == -1 (disconnesso).
 */
void net_close(int *fd);

int net_is_valid_id(char *msg);

int net_is_valid_port(char *msg);

int net_is_valid_password(char *msg);

/* controlla che ci sia una sequenza di +++*/
int net_is_valid_msg(char *msg);

#endif