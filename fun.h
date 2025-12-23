#ifndef FUN_H
#define FUN_H

typedef struct friends{
    struct user* friend_user;
    struct friends* next;
    struct friends* prev;
} friends;

typedef friends* friendsList;

typedef struct{
    char id[8];
    unsigned password;
    unsigned port;
    int socket;
    friendsList listOfFriends;
} user;

/**
 * 
 * Controlla se gli argomenti presi in input dall'utente sono validi.
 * 
 * @param id        Nome dell'utente. (solo caratteri alfanumerici ammessi)
 * @param port      Numero della porta UDP dell utente. (0<port<=9999)
 * @param password  Password dell'utente. (0<=password<=65535)
 * 
 * @return Restituisce un intero in base a quale dei tre paramatri non rispetta le condizioni.
 * 
 * - 0: nessun errore.
 * 
 * - 1: id non valido.
 * 
 * - 2: porta UDP non valida.
 * 
 * - 3: password non valida.
 */
int checkValidData(char* id, unsigned port, unsigned password);

/**
 * 
 * Alloca un nuovo utente di tipo user che ha id, porta UDP e password.
 * 
 * @param id        Nome dell'utente.
 * @param port      Numero della porta UDP dell'utente.
 * @param password  Password dell'utente.
 * 
 * @return Puntatore al nuovo utente creato, o NULL se fallisce l'allocazione.
 */
user* createNewUser(char* id, unsigned port, unsigned password);

#endif