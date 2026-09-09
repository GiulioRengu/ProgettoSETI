#ifndef FLOODHANDLER_H
#define FLOODHANDLER_H

#include "../server_client/protocol.h"
// #include "../server_client/msgparsing.h"
// #include "../server_client/net.h"

/*
 * Utility function, returns idx of id in server->users
 * @retval -1 if user.id is not found. 
 * @retval index of user in server->users else
 */
int get_user_index(Server* server, const char* id);

/*
 * Execution of flood using BFS through sender->friends.
 */
void execute_flood_bfs(Server* server, User* sender, const char* mess);


#endif