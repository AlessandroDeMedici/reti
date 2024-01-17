// connessione.h
#ifndef CONNESSIONE_INCLUSA
#define CONNESSIONE_INCLUSA
#include "../lib-reti.h"
// descrittore di connessione
struct des_connection
{
	struct user * utente;
	int sd;
	natb status;		// 0: not logged
				// 1: logged
				// 2: playing
	struct des_connection * next;
};

struct des_connection * nuovaConnessione(int);
size_t chiudiConnessione(int);
size_t logoutConnessione(int);
size_t loginConnessione(struct des_connection *,struct user *);
size_t playingConnessione(int);
size_t homeConnessione(int);
struct des_connection * getConnessione(int);
size_t nessunaConnessione();
size_t logout(int);

#endif
