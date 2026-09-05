#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#include "server_client/protocol.h"
#include "server_client/msgparsing.h"
 
int main(){


    char* msg = "messs unoduetr abcdef";
    char buff[32];
        get_id(msg, buff);
    get_msg(msg, buff, 15);
    printf("%s\n", buff);
}