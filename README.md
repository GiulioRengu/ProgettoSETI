# ProgettoSETI
Progetto di SETI 25/26 Biglieri - Rengucci

Questa reposiotory è dedicata al progetto di SETI "IPortBook", un sistema simile ad un socialnetwork dove però gli utenti comunicano tramite datagrammi UDP.

Avvia il client scegliendo la porta UDP per le notifiche:

```sh
./client_exe ::1 5000 1
```

Nel terminale del client, registra l'utente con `regis abcdefgh 1234`.
Il client usa automaticamente la porta UDP `1`, salvata e inizializzata
all'avvio, e la include nel messaggio `REGIS` inviato al server.

DA FARE:
-mainClient.c da rivedere
-verbose da aggiungere
