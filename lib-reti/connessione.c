// connessione.c
#include "connessione.h"

// lista globale delle connessioni attualmente attive
struct des_connection * connessioni = 0;

// descrizione:
// funzione che aggiunge una nuova connessione alla lista
// argomenti:
// sd -> descrittore del socket della connessione
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

// descrizione:
// restituisce il puntatore al descrittore di connessione dato il socket descriptor
// argomenti:
// sd -> descrittore del socket della connessione
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

// descrizione:
// funzione che chiude la connessione dato il socket
// argomenti:
// sd -> descrittore del socket della connessione
// ritorno:
// ritorna 0 in caso di successo
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

// descrizione:
// modifica lo stato della connessione in logged e lega la connessione ad un utente
// argomenti:
// connessione -> puntatore al descrittore di connessione da modificare
// utente -> puntatore all'utente che e' appena acceduto
// ritorno:
// ritorna 0 in caso di successo, 1 altrimenti
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

// descrizione:
// modifica lo status della connessione in playing (il player si trova in una room)
// argomenti:
// sd -> socket descriptor della connessione
// ritorno:
// ritorna 0 in caso di successo, 1 altrimenti
size_t playingConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = PLAYING;
	return 0;
}


// descrizione:
// modifica lo status della connessione in NOT_LOGGED (il giocatore ha fatto logout o non e' ancora collegato)
size_t logoutConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = NOT_LOGGED;
	return 0;
}

// descrizione:
// modifica lo status della connessione in logged (il player si trova nella home)
// argomenti:
// sd -> socket descriptor della connessione
// ritorno:
// ritorna 0 in caso di successo, 1 altrimenti
size_t homeConnessione(int sd)
{
	struct des_connection * p = getConnessione(sd);
	if (!p)
		return 1;
	p->status = LOGGED;
	return 0;
}

// descrizione:
// funzione che ritorna 1 nel caso in cui non ci sia nessuna connessione attiva, 0 altrimenti
size_t nessunaConnessione()
{
	// non ci sono connessioni quando la lista di connessioni attive e' vuota
	if (!connessioni)
		return 1;
	return 0;
}


// descrizione:
// funzione usata dal client per inviare una richiesta di logout
// argomenti:
// sd -> descrittore del socket
// ritorno:
// la funzione ritorna 0 in caso di successo, 1 altrimenti
size_t logout(int sd)
{
	natb opcode = LOGOUT;
	int ret = send(sd,&opcode,sizeof(opcode),0);
	if (ret <= 0){
		perror("logout - errore in fase di send");
		return 1;
	}
	return 0;
}




