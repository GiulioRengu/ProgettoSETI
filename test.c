#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#include "server_client/protocol.h"
#include "server_client/msgparsing.h"
 
int main(){


    char* msg = "+++al+n++c+a++em+f+opa";
    char buff[32];
    get_msg(msg, buff, 0);
    printf("%s\n", buff);
}