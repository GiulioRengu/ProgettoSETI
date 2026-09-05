#ifndef STREAMHANDLERS_H
#define STREAMHANDLERS_H

#include "../server_client/protocol.h"
#include "../server_client/msgparsing.h"
#include "../server_client/net.h"
#include "server.h"

/**
 * Adding a stream to user->streams linked list;
 * Fails if user is NULL or from_id is NULL or MAX_STREAMS of user is reached
 * 
 * @return 0 success,
 * @return -1 failure.
 */
int stream_add(User *user, const char *from_id, const char *msg, StreamType type);

/**
 * Removes the oldest Stream in the streams linked list
 * Fails when list is empty
 * 
 */
Stream *stream_remove(User *user);

/**
 * self explanatory
 * @return 0 success,
 * @return -1 failure.
 */
int is_stream_empty(User *user);

#endif