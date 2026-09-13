#include "floodHandler.h"
#include "streamHandlers.h"
#include "server.h"

int get_user_index(Server* server, const char* id)
{
    for(int i=0; i<MAX_USERS; i++)
    {
        if((server->users[i].id[0]!='\0') && strcmp(server->users[i].id,id)==0) return i;
    }
    return -1;
}

void execute_flood_bfs(Server* server, User* sender, const char* mess)
{
    bool visited[MAX_USERS]={false};

    User *queue[MAX_USERS];
    int head=0, tail=0;

    int sender_index=get_user_index(server, sender->id);
    if(sender_index<0) return;

    visited[sender_index]=true;

    queue[tail++]=sender;

    while(head<tail)
    {
        User *current_user=queue[head++];
        for(int i=0; i<MAX_USERS; i++)
        {
            if(current_user->friends[i][0]!='\0')
            {
                char* friend_id=current_user->friends[i];
                int friend_index=get_user_index(server, friend_id);

                if(friend_index!=-1 && !visited[friend_index])
                {
                    User*friend_ptr=&server->users[friend_index];
                    queue[tail++]=friend_ptr;
                    visited[friend_index]=true;
                    
                    if (stream_add(friend_ptr, sender->id, mess, STREAM_FLOO)==0)
                    {
                        server_send_udp_notification(server, friend_ptr, STREAM_FLOO);
                        VERB("Flood propagato da %s a %s", sender->id, friend_ptr->id);
                    }
                }
            }
        }
    }
}