#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#include "server_client/protocol.h"
#include "server_client/msgparsing.h"
 
int main(){

    char *msg="CONSU 12345678 1010 999";
    // char buff[256];
    char *type=malloc(sizeof(char)*16);
    printf("%d\n", get_type(msg, type));
    get_id(msg, type);
    printf("%s\n", type);
    get_port(msg, type, 15);
    printf("%s\n", type);
    printf("%d\n", get_type(msg, type));
}