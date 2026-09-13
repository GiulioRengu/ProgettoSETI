#ifndef FLOODHANDLER_H
#define FLOODHANDLER_H

#include "../server_client/protocol.h"

//Controlla se esiste un utente con username id nella lista di utenti del server
//Ritorna il suo indice se lo trova, -1 altrimenti
int get_user_index(Server* server, const char* id);

//Esegue la BFS per propagare il flood a tutti gli amici degli amici, quindi invia mess a tutti gli utenti idonei
void execute_flood_bfs(Server* server, User* sender, const char* mess);


#endif