#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>

#define MAX_USERS 100
#define ID_LENGTH 8
#define TYPE_LENGHT 5
#define MSG_TERMINATOR "+++"
#define MSG_LENGTH_MAX 200
#define MSG_BUFF_MAXSIZE 300

typedef enum{
    STREAM_FRIEND_REQ, //[0XX]
    STREAM_FRIEND_ACC, //[1XX]
    STREAM_FRIEND_REJ, //[2XX]
    STREAM_MSG,        //[3XX]
    STREAM_FLOO,       //[4XX]
} StreamType;

typedef struct { //per le notifiche UDP
    StreamType type;
    char from_id[ID_LENGTH+1];
    char msg[MSG_LENGTH_MAX+1];
    struct Stream *next;
}Stream;

typedef struct {
    char id[ID_LENGTH+1];
    uint16_t password;
    uint16_t udp_port;
    char ip[INET6_ADDRSTRLEN];
    int tcp_fd;
    int connected; //0 connesso, -1 non connesso;
    
    char friends[MAX_USERS][ID_LENGTH+1];
    int friend_count;
    bool pending_frie;

    struct Stream *streams;
    int stream_count;
} User;

// typedef struct {
//     int fd;
//     char ip[INET6_ADDRSTRLEN];
// } pendingClient;

typedef struct {
    User users[MAX_USERS];
    int client_count;
    int tcp_fd; //socket TCP listener

    fd_set master_fds;
    int fdmax;

    char pendingClients[FD_SETSIZE][INET6_ADDRSTRLEN]; //array contente l'ip dei clienti che non si sono ancora registrati
} Server;

#endif