// connessione.c
#include "connessione.h"

// lista delle connessioni attualmente attive
struct des_connection * connessioni = 0;

// funzione che aggiunge una nuova connessione
struct des_connection * nuovaConnessione(int sd)
{
	struct des_connection * new_conn = (struct des_connection *)malloc(sizeof(struct des_connection));
	// eventuale gestione dell'errore
	if (!new_conn){
		return NULL;
	}
	
	// inizializzazione della nuova struttura
	new_conn->next = 0;
	new_conn->sd = sd;
	new_conn->status = NOT_LOGGED;
	new_conn->utente = 0;

	// inserimento in fondo alla lista
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
	printf("(Main) Connessione aperta con il socket %d\n",sd);
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
	// elimino un elemento dalla lista con la tecnica dei due puntatori
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
	printf("(Main) Connessione chiusa con il socket %d\n",sd);
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

size_t nessunaConnessione()
{
	if (!connessioni)
		return 1;
	return 0;
}

