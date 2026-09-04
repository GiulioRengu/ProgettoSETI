#include "streamHandlers.h"

int stream_add(User *user, const char *from_id, const char *msg, StreamType type)
{
    if(user==NULL || from_id==NULL || user->stream_count>=MAX_STREAMS) return -1;
    
    Stream *new_stream=malloc(sizeof(Stream));
    new_stream->next=NULL;
    strncpy(new_stream->from_id, from_id, ID_LENGTH+1);
    new_stream->from_id[ID_LENGTH] = '\0';
    strncpy(new_stream->msg, msg, MSG_LENGTH_MAX+1);
    new_stream->msg[MSG_LENGTH_MAX] = '\0';
    new_stream->type=type;

    if(is_stream_empty(user))
    {
        user->streams=new_stream;
        user->stream_count++;
    }
    else
    {
        Stream*tmp=user->streams;
        while(tmp->next!=NULL) tmp=tmp->next;
        tmp->next=new_stream;
        user->stream_count++;
    }
    return 0;
}

int stream_remove(User *user)
{
    if(user==NULL || user->streams==NULL) return -1;
    if(user->stream_count==0) return 1;

    Stream*tmp=user->streams;
    user->streams=user->streams->next;
    free(tmp);
    user->stream_count--;
    return 0;
}

int is_stream_empty(User *user)
{
    if(user==NULL) return -1;
    return user->stream_count<=0;
}