#ifndef LOGIN_INCLUSA
#define LOGIN_INCLUSA

#include "user.h"
#include "costanti.h"

// funzione per creare il pacchetto partendo da username e password
size_t makeStringaLogin(char *, char *, char *);
// funzione per ottenere username e password dal pacchetto di login
size_t demakeStringaLogin(char *, char *, char *);
// funzione per inviare il pacchetto con la stringa
size_t inviaLogin(int,char*,char*);
// funzione per ricevere il pacchetto con la stringa
size_t riceviLogin(int, char *, char *);
// funzione per fare automaticamente i cicli di login sul client
size_t loginUtente(int, char *, char *);
// funzione per fare automaticamente i cicli di login sul server
struct user * loginServer(int, char *, char *);
// funzione per fare automaticamente i cicli di register sul client
size_t registerUtente(int,char *,char *);
// funzione per fare automaticamente i cicli di register sul server
struct user * registerServer(int,char*,char*);
// funzione per fare automaticamente la procedura di login o di registazione sul client
size_t userLogin(int,char *,char *);
// funzione per fare automaticamente la procedura di login o di registrazione sul server
struct user * serverLogin(int,char*,char*);

#endif
