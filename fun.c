#include "fun.h"
#include <stdlib.h>

user* createNewUser(char* id, unsigned port, unsigned pass){
    user* ret = malloc(sizeof(user));
    ret->id = id;
    ret->port = port;
    ret->password = pass;
    ret->listOfFriends = NULL;
    return ret;
}

