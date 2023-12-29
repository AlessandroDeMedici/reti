// user.c
#include "user.h"
#include <string.h>
#include <stdlib.h>

// lista globale di utenti
struct user * utenti = 0;

// funzione che aggiunge una nuova connessione
struct user * nuovoUtente(char * username, char * password)
{
	// devo controllare che non sia gia presente lo stesso user
	// posso aggiungere il nuovo utente
	struct user * new_user = (struct user *)malloc(sizeof(struct user));
	// eventuale gestione dell'errore
	if (!new_user){
		;
	}

	// inizializzazione della nuova struttura
	new_user->next = utenti;
	strcpy(new_user->username,username);
	strcpy(new_user->password,password);

	// inserisco l'utente in cima alla lista
	utenti = new_user;
	return new_user;
 }

// funzione che controlla che le credenziali siano corrette
struct user * controllaUtente(char * username, char * password)
{
	struct user * p = utenti;
	while (p){
		if (!strcmp(p->username,username)){
			if (!strcmp(p->password,password)){
				// login corretto
				return p;
			} else {
				// password scorretta
				return 0;
			}
		}
		p = p->next;
	}
	return 0;		// username non trovato
}



