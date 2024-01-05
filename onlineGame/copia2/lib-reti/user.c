// user.c
#include "user.h"

// lista globale di utenti
struct user * utenti = 0;

// descrizione:
// funzione che aggiunge un nuovo utente e ritorna un puntatore a questo
// argomenti:
// username -> array di caratteri che contiene lo username
// password -> array di caratteri cheh contiene la password
struct user * nuovoUtente(char * username, char * password)
{
	// devo controllare che non sia gia presente lo stesso user
	// posso aggiungere il nuovo utente
	struct user * new_user = (struct user *)malloc(sizeof(struct user));
	// eventuale gestione dell'errore
	if (!new_user){
		return NULL;
	}

	// inizializzazione della nuova struttura
	new_user->next = utenti;
	strcpy(new_user->username,username);
	strcpy(new_user->password,password);

	// inserisco l'utente in cima alla lista
	utenti = new_user;
	return new_user;
 }

// descrizione:
// funzione che ritorna un puntatore ad un utente se username e password
// coincidono, null altrimenti
// argomenti:
// username -> array di caratteri che contiene lo username
// password -> array di caratteri che contiene la password
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

// descrizione:
// funzione che ritorna un puntatore ad un utente se solo lo username
// coincide, null altrimenti
// argomenti:
// username -> array di caratteri che contiene lo username
struct user * controllaUsername(char * username)
{
	struct user * p = utenti;
	while (p){
		if (!strcmp(p->username,username)){
			return p;
		}
		p = p->next;
	}
	return 0;
}




