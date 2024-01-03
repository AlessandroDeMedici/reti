#include "login.h"

// descrizione:
// funzione che dati username e password inserisce la stringa "username;password\0" in buffer.
// Questa stringa verra usata in fase di registrazione
// argomenti:
// buffer -> array di caratteri dove verra scritto "username;password\0"
// username -> array di caratteri dove verra scritto lo username
// password -> array di caratteri dove verra scritta la password
// ritorno:
// la funzione ritorna la lunghezza della stringa "username;password\0" compreso di marca di fine stringa
size_t makeStringaLogin(char * buffer, char * username, char * password)
{
	int i = 0, j = 0;
	for (; username[i] != '\0'; i++)
		buffer[i] = username[i];
	buffer[i++] = ';';
	for (; password[j] != '\0'; j++)
		buffer[j + i] = password[j];
	buffer[i + j++] = '\0';
	return strlen(buffer) + 1;
} 

// descrizione:
// funzione che data la stringa "username;password\0" la divide in username e password
// argomenti:
// buffer -> array di caratteri che contiene "username;password\0"
// username -> array di caratteri dove verra scritto lo username
// password -> arary di caratteri dove verra scritta la password
void demakeStringaLogin(char * buffer, char * username, char * password)
{
	int i = 0, j = 0;
	for (; buffer[i] != ';'; i++)
		username[i] = buffer[i];
	username[i++] = '\0';
	for (;buffer[j] != '\0';j++)
		password[j] = buffer[i + j];
	password[j] = '\0';
}

// descrizione:
// funzione che invia la stringa usata in fase di login ("username;password\0")
// argomenti:
// sd -> descrittore sel socket
// username -> stringa che contiene lo username
// password -> stringa che contiene la password
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 in caso di disconnessione o di errore
size_t inviaLogin(int sd,char * username, char * password)
{
	char buffer[100];
	natb len = makeStringaLogin(buffer,username,password);
	int ret;
	ret = send(sd,&len,sizeof(len),0);
	if (ret <= 0)
		return 1;
	ret = send(sd,buffer,len,0);
	if (!ret)
		return 1;
	return 0;
}

// descrizione:
// funzione che riceve la stringa usata in fase di login ("username;password\0")
// argomenti:
// sd -> descrittore del socket
// username -> array dove verra scritto lo username
// password -> array dove verra scritta la password
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 in caso di disconnessione o di errore
size_t riceviLogin(int sd, char * username, char * password)
{
	char buffer[100];
	natb len;
	int ret, received = 0;
	ret = recv(sd,&len,sizeof(len),0);
	while (received < len){
		ret = recv(sd,buffer + received,len - received,0);
		if (!ret){
			return 1;
		}
		received += ret;
	}
	demakeStringaLogin(buffer,username,password);
	return 0;
}

// descrizione:
// funzione che svolge la fase di login lato client
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri dove verra scritto lo username
// password -> array di caratteri dove verra scritta la password
// ritorno:
// la funziona ritorna 0 in caso di successo,
// 1 in caso di disconnessione o di errore
size_t loginUtente(int sd, char * username, char * password)
{
	natl ret;
	natb opcode = 0;
	int i;
	// per un numero di volte pari a MAX TENTATIVI vengono richiesti username e password
	for (i = 0; i < MAX_TENTATIVI; i++){
		printf("> Username: ");
		scanf("%s",username);
		printf("> Password: ");
		scanf("%s",password);
		// invio la stringa di login
		inviaLogin(sd,username,password);
		// attendo la risposta del server
		ret = recv(sd,&opcode,sizeof(opcode),0);
		if (!ret) // il server ha chiuso la connessione
			return 1;
		if (opcode == OK){
			// login completato
			return 0;
		}
		// il login non e' stato completato correttamente (ad esempio si e' ricevuto NOK al posto di OK)
		if (i < MAX_TENTATIVI - 1)
			// Ho ancora altri tentativi
			printf("Errore, riprova...\n");
		else
			// Non ho altri tentativi
			printf("Troppi tentativi, disconnessione...\n");
	}
	return 1;
} 

// descrizione:
// funzione che svolge la fase di login lato server 
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri nel quale verra scritto lo username
// password -> array di caratteri nel quale verra scritta la password
// ritorno:
// la funzione ritorna un puntatore all'utente collegato in caso di successo,
// NULL in caso di errore
struct user * loginServer(int sd,char * username, char * password)
{
	natb opcode = 0;
	natl ret;
	struct user * utente = NULL;
	int i;
	// ricevo la stringa di login per numero massimo di volte MAX_TENTATIVI
	for (i = 0; i < MAX_TENTATIVI; i++){	
		// il client si e' disconnesso
		if (riceviLogin(sd,username,password))
			return NULL;
		// controllo se e' presente un utente con tali username e password
		utente = controllaUtente(username,password);
		if (utente){ 	
			// l'utente e' presente
			opcode = OK;
			ret = send(sd,&opcode,sizeof(opcode),0);
			if (ret <= 0)
				// si e' verificato un errore
				return NULL;
			return utente;
		} else { 	
			// utente non loggato correttamente
			opcode = NOK;
			ret = send(sd,&opcode,sizeof(opcode),0);
		}
	}
	// ho superato il numero massimo di tentativi
	return NULL;
} 

// descrizione:
// funzione che svolge la fase di registrazione lato client
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri nel quale verra scritto lo username
// password -> array di caratteri nel quale verra scritta la password
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 in caso di errore o disconnessione del server
size_t registerUtente(int sd, char * username, char * password)
{	
	printf("Username: ");
	scanf("%s",username);
	printf("Password: ");
	scanf("%s",password);
	natb opcode;
	natl ret;
	inviaLogin(sd,username,password);
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (ret <= 0)
		// il server si e' disconnesso o si e' verificato un errore
		return 1;
	if (opcode == OK){
		// procedura di registrazione andata a buon fine
		return 0;
	}
	printf("E' gia' presente un account con queste credenziali\n");
	return 1;
}

// descrizione
// funzione che svolge la fase di registrazione lato server
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri nel quale verra scritto lo username
// password -> array di caratteri nel quale verra scritta la password
// ritorno:
// la funzione ritorna un puntatore all'utente colleggato (appena registrato)
// in caso di successo, NULL in caso di errore o di disconnessione del client
struct user * registerServer(int sd, char * username, char * password)
{
	natb opcode = OK;
	natl ret;
	struct user * new_user;
	// ricevo la stringa di login
	if (riceviLogin(sd,username,password))
		return NULL; 	// il client si e' disconnesso
	// creo un nuovo utente con queste credenziali
	new_user = controllaUsername(username);
	if (new_user){		// era gia presente un utente con lo stesso username
		opcode = NOK;
		ret = send(sd,&opcode,sizeof(opcode),0); // invio il messaggio di NOK
		if (!ret)
			perror("registerServer - errore in fase di invio\n");
		return NULL;
	}
	// posso procedere a creare un nuovo utente
	new_user = nuovoUtente(username,password);
	if (new_user)
		opcode = OK;
	
	// invio il messaggio di OK
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (ret <= 0)
		return NULL;
	
	return new_user;
}

// descrizione:
// funzione che svolge la fase di registrazione o di login per il client
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri nel quale verra scritto lo username
// passowrd -> array di caratteri nel quale verra scritta la password
// ritorno:
// la funzione ritorna 0 in caso di successo
// 1 in caso di errore o disconnessione del server
size_t userLogin(int sd, char * username, char * password)
{
	natl ret = 0;
	natb opcode = 0;
	char buffer[64];
	// stampo il menu di login
	printMenu();
	// ottengo il comando (login o register)
	while (1){
		int i;
		fgets(buffer,63,stdin);
		for (i = 0; i < 64; i++){
			if (buffer[i] == '\n')
				buffer[i] = '\0';
		}

		if (!strcmp(buffer,"login") || !strcmp(buffer,"log")){
			// login branch
			opcode = LOGIN;
			break;
		} else if (!strcmp(buffer,"register") || !strcmp(buffer,"reg")){
			// register branch
			opcode = REGISTER;
			break;
		} else
			printf("\033[A\r\033[K> ");
	}

	// invio quello che voglio fare al server (LOGIN o REGISTER)
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (!ret)
		return 1;


	if (opcode == LOGIN){
		// fase di login
		ret = loginUtente(sd,username,password);
		if (!ret)
			printf("Login avvenuto correttamente\n");
		else
			return 1;
	} else {
		// fase di registrazione
		ret = registerUtente(sd,username,password);
		if (!ret)
			printf("Registrazione avvenuta correttamente\n");
		else {
			return 1;
		}
	}

	return 0;
}

// descrizione:
// funzione che svolge la fase di login o di registrazione per il server
// argomenti:
// sd -> descrittore del socket
// username -> array di caratteri nel quale verra scritto lo username
// password -> array di caratteri nel quale verra scritta la password
// ritorno:
// la funzione ritorna un puntatore all'utente collegato in caso di successo
// NULL in caso di errore o disconnessione del client
struct user * serverLogin(int sd, char * username, char * password)
{ 
	struct user * utente;
	natb opcode;
	natl ret;

	// ricevo opcode dal client (LOGIN O REGISTER)
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (!ret){ // connessione chiusa
		return NULL;
	}

	if (opcode == LOGIN){
		// fase di login
		utente = loginServer(sd,username,password);
		if (utente)
			// log
			printf("(Main) (%d) %s e' acceduto\n",sd,username);
		else {
			printf("(Main) %d ha provato ad accedere\n",sd);
			return NULL;
		}
	} else {
		// fase di registrazione
		utente = registerServer(sd,username,password);
		if (utente){
			// log
			printf("(Main) (%d) %s si e' registrato\n",sd,username);
		} else {
			printf("(Main) %d ha provato a registarsi\n",sd);
			return NULL;
		}
	}
	return utente;
}




