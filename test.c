#include <stdio.h>
#include<string.h>

#include "fun.h"

int main(){
    user* u = createNewUser("failopps", 6741, 16164);
    printf("%s, %ld, %d\n", u->id, strlen("failopps"), u->password);
    friendsList FL;
    FL->name = "rengu";
    FL->next->name = "gianko";
    FL->next->next = NULL;
    while(FL){
        printf("%s, ", FL->name);
        FL = FL->next;
    }
}