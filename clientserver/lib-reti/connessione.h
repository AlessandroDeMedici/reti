// connessione.h
#ifndef CONNESSIONE_INCLUSA
#define CONNESSIONE_INCLUSA

#include "user.h"
#include "costanti.h"


// descrittore di connessione
struct des_connection
{
	struct user * utente;
	int sd;
	natl chiave;		// chiave di sessione
	natb status;		// 0: not logged
				// 1: logged
				// 2: playing
	struct des_connection * next;
};

natl generaChiaveSessione();
natl getChiaveSessione();
struct des_connection * nuovaConnessione(int sd);
size_t chiudiConnessione(int sd);
size_t loginConnessione(struct des_connection *,struct user *);
size_t logoutConnessione(int);
size_t playingConnessione(int);
size_t homeConnessione(int);
struct des_connection * getConnessione(int);
#endif
