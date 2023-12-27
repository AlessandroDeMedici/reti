// connessione.c
#include "connessione.h"
#include <stdio.h>
#include <stdlib.h>
#include "login.h"


// lista delle connessioni attualmente attive
struct des_connection * connessioni = 0;

// chiave di sessione
natl KEY_SEED = 0;

natl generaChiaveSessione()
{
	return KEY_SEED++;
}

natl getChiaveSessione()
{
	return KEY_SEED;
}

// funzione che aggiunge una nuova connessione
struct des_connection * nuovaConnessione(int sd)
{
	struct des_connection * new_conn = (struct des_connection *)malloc(sizeof(struct des_connection));
	// eventuale gestione dell'errore
	if (!new_conn){
		;
	}
	
	// inizializzazione della nuova struttura
	new_conn->next = 0;
	new_conn->sd = sd;
	new_conn->status = NOT_LOGGED;
	new_conn->chiave = generaChiaveSessione();
	new_conn->utente = 0;

	// inserimento in fondo
	// INIZIO-ZONA-CRITICA
	struct des_connection * p = connessioni;
	struct des_connection * q = 0;
	while(p){
		q = p;
		p = p->next;
	}
	if (!q){
		// primo elemento
		connessioni = new_conn;
	} else {
		q->next = new_conn;
	}
	printf("Connessione aperta\n");
	return new_conn;
}

// restituisce il puntatore al descrittore di connessione dato
// il socket descriptor
struct des_connection * getConnessione(int sd)
{
	struct des_connection * p = connessioni;
	while (p){
		if (p->sd == sd){
			return p;
		}
		p = p->next;
	}
	return NULL;
}

// funzione che chiude la connessione dato il socket
size_t chiudiConnessione(int sd)
{
	struct des_connection * p = connessioni;
	struct des_connection * q = 0;
	while (p){
		if (p->sd == sd){ // devo liberare quest'area
			if (!q){
				connessioni = p->next;
			} else {
				q->next = p->next;
			}
			break;
		}
		p = p->next;
	}
	free(p);
	printf("Connessione chiusa\n");
	return 0;
 }

// modifica lo stato della connessione in logged e lega la connessione ad un utente
size_t loginConnessione(struct des_connection *connessione,struct user *utente)
{
	// controllo sugli input
	if (!connessione || !utente){
		return 1;
	}
	connessione->utente = utente;
	connessione->status = LOGGED;
	return 0;
}

// modifica lo status della connessione in playing
size_t playingConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = PLAYING;
	return 0;
}

// modifica lo status della connessione in logged
size_t homeConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = LOGGED;
	return 0;
}

// modifica lo status della connessione in not_logged
size_t logoutConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = NOT_LOGGED;
	return 0;
}



