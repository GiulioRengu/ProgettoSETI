#ifndef MSGPARSING_H
#define MSGPARSING_H
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
  void parse_msg(char *src, char* dest, int offset);

  /*------------------------
  *builder client to server
  -------------------------*/

  //[REGIS id port password]
  void build_regis(char *buff, const char *id, uint16_t port, uint16_t password);

  //[CONNE id password]
  void build_conne(char *buff, const char *id, uint16_t password);

  // [FRIE? id]
  void build_frie_req(char *buff, const char *id){ sprintf(buff, "FRIE? %s+++", id) }

  // [MESS? id mess]
  void build_mess_req(char *buff, const char *id, const char *mess){ sprintf(buff, "")}

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

  /*------------------------
  *builder server to client
  -------------------------*/

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

  // [RLIST num-item] //num item è int così uso %03d
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


  /* ==========================================================================
  * BUILDER: NOTIFICHE UDP
  * ========================================================================== */

  // [YXX]
  void build_udp_notif(char *buff, StreamType type, int stream_count);

#endif