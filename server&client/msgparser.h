#ifndef MSGPARSER_H
#define MSGPARSER_H
#include "protocol.h"

/**
 * messaggio:
 * read type + data extraction
 */
 /**
  * REGIS, WELCO, GOBYE, CONNE, HELLO, FRIE?, FRIE>, FRIE<,
  * MESS?, MESS>, MESS<, FLOO>, FLOO?, LIST?, RLIST, LINUM,
  * CONSU, SSEM>,OOLF>, EIRF>, OKIRF, NOKRF, ACKRF, FRIEN, NOFRI, NOCON
  */
    // primi 5 byte del messaggio, per capire di che tipo è
    void parse_msg(const char *msg);

    /*------------------------
    *builder client to server
    -------------------------*/
    
    //[REGIS id port password]
    void build_regis(char *buff, size_t buff_size, const char *id, uint16_t port, uint16_t password);

    //[CONNE id password]
    void build_conne(char *buff, size_t buff_size, const char *id, uint16_t password);

    // [FRIE? id]
    void build_frie_req(char *buff, size_t buff_size, const char *id);

    // [MESS? id mess]
    void build_mess_req(char *buff, size_t buff_size, const char *id, const char *mess);

    // [FLOO? mess]
    void build_floo_req(char *buff, size_t buff_size, const char *mess);

    // [LIST?]
    void build_list_req(char *buff, size_t buff_size);

    // [CONSU]
    void build_consu(char *buff, size_t buff_size);

    // [OKIRF]
    void build_okirf(char *buff, size_t buff_size);

    // [NOKRF]
    void build_nokrf(char *buff, size_t buff_size);

    // [IQUIT]
    void build_iquit(char *buff, size_t buff_size);

    /*------------------------
    *builder server to client
    -------------------------*/
    
// [WELCO]
void build_welco(char *buff, size_t buff_size);

// [GOBYE]
void build_gobye(char *buff, size_t buff_size);

// [HELLO]
void build_hello(char *buff, size_t buff_size);

// [FRIE>]
void build_frie_ok(char *buff, size_t buff_size);

// [FRIE<]
void build_frie_ko(char *buff, size_t buff_size);

// [MESS>]
void build_mess_ok(char *buff, size_t buff_size);

// [MESS<]
void build_mess_ko(char *buff, size_t buff_size);

// [FLOO>]
void build_floo_ok(char *buff, size_t buff_size);

// [RLIST num-item] //num item è int così uso %03d
void build_rlist(char *buff, size_t buff_size, int num_item);

// [LINUM id]
void build_linum(char *buff, size_t buff_size, const char *id);

// [SSEM> id mess]
void build_ssem(char *buff, size_t buff_size, const char *id, const char *mess);

// [OOLF> id mess]
void build_oolf(char *buff, size_t buff_size, const char *id, const char *mess);

// [EIRF> id]
void build_eirf(char *buff, size_t buff_size, const char *id);

// [ACKRF]
void build_ackrf(char *buff, size_t buff_size);

// [FRIEN id]
void build_frien(char *buff, size_t buff_size, const char *id);

// [NOFRI id]
void build_nofri(char *buff, size_t buff_size, const char *id);

// [NOCON]
void build_nocon(char *buff, size_t buff_size);


/* ==========================================================================
 * BUILDER: NOTIFICHE UDP
 * ========================================================================== */

// [YXX]
void build_udp_notif(char *buff, size_t buff_size, StreamType type, int stream_count);

#endif