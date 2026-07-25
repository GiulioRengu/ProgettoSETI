#include "msgparsing.h"


/*------------------------
*   parsing of messages
-------------------------*/

int has_delim(char* buff, int offset){
    int plus_count = 0;
    int max_len = strlen(buff);
    for(unsigned i = offset; max_len; i++){
        if(buff[i] == ' ') return 0;
        if(buff[i] == '+') {
            plus_count++;
            if(plus_count == 3) return i-2;
        }
        else plus_count = 0;
    }
    return 0;
}

void parse(char *src, char* dest, int offset){
    if(src == NULL || dest == NULL) return;
    int len = strlen(src);

    if(offset >= len){
        dest[0] = '\0';
        return;
    }

    char* aux = src+offset;

    int delim_index = has_delim(src, offset);
    if(delim_index != 0){
        int to_copy = delim_index - offset;
        strncpy(dest, aux, to_copy);
        dest[to_copy] = '\0';
    }

    else sscanf(aux, "%s", dest);
}

int get_type(char* buff, char* type){
    parse(buff, type, 0);
    if (strncmp(type, "REGIS", 5) == 0) return MSG_REGIS;
    if (strncmp(type, "CONNE", 5) == 0) return MSG_CONNE;
    if (strncmp(type, "FRIE?", 5) == 0) return MSG_FRIE_REQ;
    if (strncmp(type, "MESS?", 5) == 0) return MSG_MESS_REQ;
    if (strncmp(type, "FLOO?", 5) == 0) return MSG_FLOO_REQ;
    if (strncmp(type, "LIST?", 5) == 0) return MSG_LIST_REQ;
    if (strncmp(type, "CONSU", 5) == 0) return MSG_CONSU;
    if (strncmp(type, "OKIRF", 5) == 0) return MSG_OKIRF;
    if (strncmp(type, "NOKRF", 5) == 0) return MSG_NOKRF;
    if (strncmp(type, "IQUIT", 5) == 0) return MSG_IQUIT;
    if (strncmp(type, "WELCO", 5) == 0) return MSG_WELCO;
    if (strncmp(type, "GOBYE", 5) == 0) return MSG_GOBYE;
    if (strncmp(type, "HELLO", 5) == 0) return MSG_HELLO;
    if (strncmp(type, "FRIE>", 5) == 0) return MSG_FRIE_OK;
    if (strncmp(type, "FRIE<", 5) == 0) return MSG_FRIE_KO;
    if (strncmp(type, "MESS>", 5) == 0) return MSG_MESS_OK;
    if (strncmp(type, "MESS<", 5) == 0) return MSG_MESS_KO;
    if (strncmp(type, "FLOO>", 5) == 0) return MSG_FLOO_OK;
    if (strncmp(type, "RLIST", 5) == 0) return MSG_RLIST;
    if (strncmp(type, "LINUM", 5) == 0) return MSG_LINUM;
    if (strncmp(type, "SSEM>", 5) == 0) return MSG_SSEM;
    if (strncmp(type, "OOLF>", 5) == 0) return MSG_OOLF;
    if (strncmp(type, "EIRF>", 5) == 0) return MSG_EIRF;
    if (strncmp(type, "ACKRF", 5) == 0) return MSG_ACKRF;
    if (strncmp(type, "FRIEN", 5) == 0) return MSG_FRIEN;
    if (strncmp(type, "NOFRI", 5) == 0) return MSG_NOFRI;
    if (strncmp(type, "NOCON", 5) == 0) return MSG_NOCON;

    return MSG_UNKNOWN;
}

void get_id(char* buff, char* id){
    parse(buff, id, 6);
}

void get_port(char* buff, char* port, int offset){
    parse(buff, port, offset);
}

void get_msg(char*buff, char* msg, int offset){
    parse(buff, msg, offset);
}

/*------------------------
*builder client to server
-------------------------*/

//[REGIS id port password]
void build_regis(char *buff, const char *id, uint16_t port, uint16_t password)
{
    int offset = sprintf(buff, "REGIS %s %04u", id, port);
    buff[offset] = password & 0xFF;            // isolo i primi 8 bit
    buff[offset + 1] = (password >> 8) & 0xFF; // shifto di 8 bit e isolo i restanti
    offset += 2;
    memcpy(buff + offset, "+++", 4);
}

//[CONNE id password]
void build_conne(char *buff, const char *id, uint16_t password)
{
    int offset = sprintf(buff, "CONNE %s", id);
    buff[offset] = password & 0xFF;
    buff[offset + 1] = (password >> 8) & 0xFF;
    offset += 2;
    memcpy(buff + offset, "+++", 4);
}

// [FRIE? id]
void build_frie_req(char *buff, const char *id) { sprintf(buff, "FRIE? %s+++", id); }

// [MESS? id mess]
void build_mess_req(char *buff, const char *id, const char *mess) { sprintf(buff, "MESS? %s %s+++", id, mess); }

// [FLOO? mess]
void build_floo_req(char *buff, const char *mess) { sprintf(buff, "FLOO? %s+++", mess); }

// [LIST?]
void build_list_req(char *buff) { strcpy(buff, "LIST?+++"); }

// [CONSU]
void build_consu(char *buff) { strcpy(buff, "CONSU+++"); }

// [OKIRF]
void build_okirf(char *buff) { strcpy(buff, "OKIRF+++"); }

// [NOKRF]
void build_nokrf(char *buff) { strcpy(buff, "NOKRF+++"); }

// [IQUIT]
void build_iquit(char *buff) { strcpy(buff, "IQUIT+++"); }

/*------------------------
*builder server to client
-------------------------*/

// [WELCO]
void build_welco(char *buff) { strcpy(buff, "WELCO+++"); }

// [GOBYE]
void build_gobye(char *buff) { strcpy(buff, "GOBYE+++"); }

// [HELLO]
void build_hello(char *buff) { strcpy(buff, "HELLO+++"); }

// [FRIE>]
void build_frie_ok(char *buff) { strcpy(buff, "FRIE>+++"); }

// [FRIE<]
void build_frie_ko(char *buff) { strcpy(buff, "FRIE<+++"); }

// [MESS>]
void build_mess_ok(char *buff) { strcpy(buff, "MESS>+++"); }

// [MESS<]
void build_mess_ko(char *buff) { strcpy(buff, "MESS<+++"); }

// [FLOO>]
void build_floo_ok(char *buff) { strcpy(buff, "FLOO>+++"); }

// [RLIST num-item] //num item è int così uso %03d
void build_rlist(char *buff, int num_item) { sprintf(buff, "RLIST %03d+++", num_item); }

// [LINUM id]
void build_linum(char *buff, const char *id) { sprintf(buff, "LINUM %s+++", id); }

// [SSEM> id mess]
void build_ssem(char *buff, const char *id, const char *mess) { sprintf(buff, "SSEM> %s %s+++", id, mess); }

// [OOLF> id mess]
void build_oolf(char *buff, const char *id, const char *mess) { sprintf(buff, "OOLF> %s %s+++", id, mess); }

// [EIRF> id]
void build_eirf(char *buff, const char *id) { sprintf(buff, "EIRF> %s+++", id); }

// [ACKRF]
void build_ackrf(char *buff) { strcpy(buff, "ACKRF>+++"); }

// [FRIEN id]
void build_frien(char *buff, const char *id) { sprintf(buff, "FRIEN %s+++", id); }

// [NOFRI id]
void build_nofri(char *buff, const char *id) { sprintf(buff, "NOFRI> %s+++", id); }

// [NOCON]
void build_nocon(char *buff) { strcpy(buff, "NOCON+++"); }

/*------------------------
 * builder udp notif
 * -----------------------*/

// [YXX]
void build_udp_notif(char *buff, StreamType type, int stream_count)
{
    int count = stream_count & 0xFF;
    int low_bit = count & 0x0F;         // isolo i 4 bit bassi
    int high_bit = (count >> 4) & 0x0F; // isolo i 4 bit alti
    sprintf(buff, "%d%X%X", type, low_bit, high_bit);
}
