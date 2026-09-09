#ifndef FLOODHANDLER_H
#define FLOODHANDLER_H

#include "../server_client/protocol.h"
// #include "../server_client/msgparsing.h"
// #include "../server_client/net.h"

/*
 * Funzione di utilità per trovare l'indice numerico di un utente 
 * nell'array server->users (utile per l'array dei "visitati").
 */
int get_user_index(Server* server, const char* id);

/*
 * Esegue la visita in ampiezza (BFS) del grafo delle amicizie 
 * per recapitare il messaggio 'mess' a tutti gli utenti connessi indirettamente.
 */
void execute_flood_bfs(Server* server, User* sender, const char* mess);


#endif