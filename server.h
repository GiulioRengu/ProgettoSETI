#ifndef SERVER_H
#define SERVER_H
#include <stdint.h>


typedef struct User{
    char id[9];
    uint16_t password;
    uint16_t udp_port;
    int tcp_fd;
    int connected; //0 connesso, -1 non connesso;
    char friends[100][9]; 
    Stream *streams;
} User;

typedef enum{
    STREAM_FRIEND_REQ, //[0XX]
    STREAM_FRIEND_ACC, //[1XX]
    STREAM_FRIEND_REJ, //[2XX]
    STREAM_MSG,        //[3XX]
    STREAM_FLOO,       //[4XX]
} StreamType;

typedef struct Stream{ //per le notifiche UDP
    StreamType type;
    char from_id[9];
    char msg[201];
}Stream;

typedef struct {
    User users[100];
    int client_count;
    int tcp_fd;
} Server;

#endif