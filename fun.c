#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Per isalnum()

#include "fun.h"

int checkValidData(char* id, unsigned port, unsigned password)
{
    if(id==NULL || id[0]=='\0') //da fare il controllo SUBITO non puoi mettere numeri come nome utente 
        return 1;
        
        for(int i=0; id[i]!='\0'; i++)
            if(!isalnum(id[i]))
                return 1;

    if(strlen(id)>=8)    
        return 1;
    

    if(port==0 || port>9999)
        return 2;

    if(password>65535 || password<0)
        return 3;

    return 0;
}

user* createNewUser(char* id, unsigned port, unsigned password){
    user* ret=malloc(sizeof(user));
    if(ret==NULL)
        return NULL;

    strcpy(ret->id, id);
    ret->port=port;
    ret->password=password;
    ret->listOfFriends=NULL;

    return ret;
}
