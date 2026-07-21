#include "msgparsing.h"

void parse_msg(char *src, char* dest, int offset){
    if(src == NULL || dest == NULL) return;
    int len = strlen(src);

    if(offset >= len){
        dest[0] = '\0';
        return;
    }

    char* aux = src+offset;
    sscanf(aux, "%s", dest);
}

  /*------------------------
  *builder client to server
  -------------------------*/

  //[REGIS id port password]
  void build_regis(char *buff, const char *id, uint16_t port, uint16_t password);

  //[CONNE id password]
  void build_conne(char *buff, const char *id, uint16_t password);

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

