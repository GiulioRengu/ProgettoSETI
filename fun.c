#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Per isalnum()

#include "serverhandlers.h"

int checkValidData(char* id, unsigned port, unsigned password)
{

    if(strlen(id) != ID_LENGTH) return 1;
    for(int i = 0; i < ID_LENGTH; i++){
        if(isalnum((unsigned char)id[i])==0) return 2;
    }

    if(port==0 || port>9999)
        return 3;

    if(password>65535 || password<0)
        return 4;

    return 0;
}

User* createNewUser(char* id, unsigned port, unsigned password){
    User* ret=malloc(sizeof(User));
    if(ret==NULL)
        return NULL;


    strcpy(ret->id, id);
    ret->udp_port=port;
    ret->password=password;

    return ret;
}
