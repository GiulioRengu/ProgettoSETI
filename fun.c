#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fun.h"

/*
    Crea un nuovo utente (user) che ha id, password e porta UDP.
 * 
@param id Nome dell'utente.
@param password password dell'utente.
@param port numero della porta UDP dell'utente.

@return Un puntatore ad un nuovo oggetto user allocato.
 */
user* createNewUser(char* id, unsigned password, unsigned port){
    user* ret = malloc(sizeof(user));
    if(ret==NULL) return NULL;

    ret->id=malloc(strlen(id)+1);
    strcpy(ret->id, id);
    ret->port=port;
    ret->password=password;
    ret->listOfFriends=NULL;
    return ret;
}

