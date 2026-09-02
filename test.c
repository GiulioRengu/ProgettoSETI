#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#include "server_client/protocol.h"
#include "server_client/msgparsing.h"
 
int main(){


    char* msg =  "1010";
    uint16_t password = (unsigned char)msg[0] | ((unsigned char)msg[1] << 8);
    printf("%d", password);
}