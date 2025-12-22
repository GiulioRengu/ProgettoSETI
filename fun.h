#ifndef FUN_H
#define FUN_H

typedef struct friends{
    char* name;
    struct friends* next;
    struct friends* prev;
} friends;

typedef friends* friendsList;

typedef struct{
    char* id;
    unsigned password;
    unsigned port;
    friendsList listOfFriends;
} user;

int checkValidData(char* id, unsigned port, unsigned pass);

user* createNewUser(char* id, unsigned port, unsigned pass);

#endif