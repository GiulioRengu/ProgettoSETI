#ifndef MSGPARSING_H
#define MSGPARSING_H
#include "protocol.h"

typedef enum {
    MSG_REGIS,          /* REGIS id port mdp+++ */
    MSG_CONNE,          /* CONNE id mdp+++      */
    MSG_FRIE_REQ,       /* FRIE? id+++          */
    MSG_MESS_REQ,       /* MESS? id mess+++     */
    MSG_FLOO_REQ,       /* FLOO? mess+++        */
    MSG_LIST_REQ,       /* LIST?+++             */
    MSG_CONSU,          /* CONSU+++             */
    MSG_OKIRF,          /* OKIRF+++             */
    MSG_NOKRF,          /* NOKRF+++             */
    MSG_IQUIT,          /* IQUIT+++             */

    MSG_WELCO,          /* WELCO+++             */
    MSG_GOBYE,          /* GOBYE+++             */
    MSG_HELLO,          /* HELLO+++             */
    MSG_FRIE_OK,        /* FRIE>+++             */
    MSG_FRIE_KO,        /* FRIE<+++             */
    MSG_MESS_OK,        /* MESS>+++             */
    MSG_MESS_KO,        /* MESS<+++             */
    MSG_FLOO_OK,        /* FLOO>+++             */
    MSG_RLIST,          /* RLIST num-item+++    */
    MSG_LINUM,          /* LINUM id+++          */
    MSG_SSEM,           /* SSEM> id mess+++     */
    MSG_OOLF,           /* OOLF> id mess+++     */
    MSG_EIRF,           /* EIRF> id+++          */
    MSG_ACKRF,          /* ACKRF+++             */
    MSG_FRIEN,          /* FRIEN id+++          */
    MSG_NOFRI,          /* NOFRI id+++          */
    MSG_NOCON,          /* NOCON+++             */
} MsgType;


//controlla se la stringa ha delimitatore +++, se lo ha restituisce il suo indice, altrimenti -1
int has_delim(char* buff, int offset);

//Legge un token da src(+offset) finche non trova uno spazio o +++ e mette il risultato in dest
void parse(char *src, char* dest, int offset);

//Estrae un messaggio da src e lo mette in dest. Diversa da parse perche non si deve fermare se incontra spazi
void extract_msg(char* src, char* dest, int offset);

//Ritorna il tipo del messaggio oppure -1 in caso di errore
int get_type(char* buff);

//Chiama parse per estrarre l'id dal messaggio (offset=6)
void get_id(char* buff, char* id);

//Chiama parse per estrarre la porta dal messaggio (offset variabile)
void get_port(char* buff, char* port, int offset);

//Chiama extract_msg per estrarre il messaggio da inviare (offset variabile)
void get_msg(char*buff, char* msg, int offset);


//[REGIS id port password]
// Ritorna la lunghezza in byte, escluso il NULL finale
int build_regis(char *buff, const char *id, uint16_t port, uint16_t password);

//[CONNE id password]
// Ritorna la lunghezza in byte, escluso il NULL finale
int build_conne(char *buff, const char *id, uint16_t password);

// [FRIE? id]
void build_frie_req(char *buff, const char *id);

// [MESS? id mess]
void build_mess_req(char *buff, const char *id, const char *mess);

// [FLOO? mess]
void build_floo_req(char *buff, const char *mess);

// [LIST?]
void build_list_req(char *buff);

// [CONSU]
void build_consu(char *buff);

// [OKIRF]
void build_okirf(char *buff);

// [NOKRF]
void build_nokrf(char *buff);

// [IQUIT]
void build_iquit(char *buff);

// [WELCO]
void build_welco(char *buff);

// [GOBYE]
void build_gobye(char *buff);

// [HELLO]
void build_hello(char *buff);

// [FRIE>]
void build_frie_ok(char *buff);

// [FRIE<]
void build_frie_ko(char *buff);

// [MESS>]
void build_mess_ok(char *buff);

// [MESS<]
void build_mess_ko(char *buff);

// [FLOO>]
void build_floo_ok(char *buff);

// [RLIST num-item] num item è int così uso %03d
void build_rlist(char *buff, int num_item);

// [LINUM id]
void build_linum(char *buff, const char *id);

// [SSEM> id mess]
void build_ssem(char *buff, const char *id, const char *mess);

// [OOLF> id mess]
void build_oolf(char *buff, const char *id, const char *mess);

// [EIRF> id]
void build_eirf(char *buff, const char *id);

// [ACKRF]
void build_ackrf(char *buff);

// [FRIEN id]
void build_frien(char *buff, const char *id);

// [NOFRI id]
void build_nofri(char *buff, const char *id);

// [NOCON]
void build_nocon(char *buff);

//Costruisce una notifica udp [YXX] con Y = type e XX = stream_count
void build_udp_notif(char *buff, StreamType type, int stream_count);

#endif
