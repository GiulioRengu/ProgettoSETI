#ifndef STREAMHANDLERS_H
#define STREAMHANDLERS_H

#include "../server_client/protocol.h"
#include "../server_client/msgparsing.h"
#include "../server_client/net.h"
#include "server.h"

//Aggiunge un flusso alla lista di flussi di user
//Ritorna 0 se aggiunge il flusso correttamente, -1 se la lista flussi e' piena o non completata correttamente
int stream_add(User *user, const char *from_id, const char *msg, StreamType type);

//Rimuove il flusso in testa dalla lista di flussi di user e lo ritorna
//Ritorna NULL in caso di errore o se non ci sono flussi da rimuovere 
Stream *stream_remove(User *user);

//Controlla se la lista di flussi di user e' vuota
//Ritorna user->stream_count <= 0
int is_stream_empty(User *user);

#endif