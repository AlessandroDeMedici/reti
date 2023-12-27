#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include "login.h"
#include "connessione.h"
#include "user.h"
#include "menu.h"


// funzione che crea il pacchetto usato in fase di login e ritorna la dimensione del pacchetto in byte
 size_t makeStringaLogin(char * buffer, char * username, char * password)
{
	int userLen = strlen(username);
	int passLen = strlen(password);
	strncpy(buffer,username,userLen);
	buffer[userLen] = '\0';
	buffer += userLen + 1;
	strncpy(buffer,password,passLen);
	buffer[passLen] = '\0';
	return userLen + passLen + 2;
}

// funzione che separa il pacchetto usato in fase di login ricevuto
 size_t demakeStringaLogin(char * buffer, char * username, char * password)
{
	int userLen = strlen(buffer);
	strcpy(username,buffer);
	buffer += userLen + 1;
	strcpy(password,buffer);
	return 0;
}

// funzione che invia la stringa di login
size_t inviaLogin(int sd,char * username, char * password)
{
	char buffer[100];
	natb len = makeStringaLogin(buffer,username,password);
	int ret, sent = 0;
	ret = send(sd,&len,sizeof(len),0);
	// controllo su invio corretto (DA AGGIUNGERE)
	if (!ret) ;
	while (sent < len){
		ret = send(sd,buffer + sent,len - sent,0);
		sent += ret;
	}
	return 0;
}

// funzione che riceve la stringa di login
size_t riceviLogin(int sd, char * username, char * password)
{
	char buffer[100];
	natb len;
	int ret, received = 0;
	ret = recv(sd,&len,sizeof(len),0);
	while (received < len){
		ret = recv(sd,buffer + received,len - received,0);
		received += ret;
	}
	demakeStringaLogin(buffer,username,password);
	return 0;
}

// funzione che svolge il login lato client (completamente automatico)
size_t loginUtente(int sd, char * username, char * password)
{
	natl ret;
	natb opcode = 0;
	for (int i = 0; i < MAX_TENTATIVI; i++){
		printf("Username: ");
		scanf("%s",username);
		printf("Password: ");
		scanf("%s",password);
		inviaLogin(sd,username,password);
		// attendo la risposta del server
		ret = recv(sd,&opcode,sizeof(opcode),0);
		if (!ret) ;
		if (opcode == OK){
			// login completato
			return 0;
			// da cambiare con exit(0) nel caso di gestione con thread
		}
		if (i < MAX_TENTATIVI - 1)
			printf("Errore, riprova...\n");
		else
			printf("Troppi tentativi, disconnessione...\n");
	}
	return 1;
	// da cambiare con exit(1) nel caso di gestione con thread
} 

// funzione che svolge il login lato server (completamente automatico)
struct user * loginServer(int sd,char * username, char * password)
{
	natb opcode = 0;
	natl ret;
	struct user * utente = NULL;
	for (int i = 0; i < MAX_TENTATIVI; i++){
		riceviLogin(sd,username,password);
		// controlla che sia affidabile
		// INIZIO-ZONA-CRITICA
		utente = controllaUtente(username,password);
		// FINE-ZONA-CRITICA
		if (utente){ 	// utente loggato correttamente
			opcode = OK;
			ret = send(sd,&opcode,sizeof(opcode),0);
			if (!ret) ;
			return utente;
		} else { 	// utente non loggato correttamente
			opcode = NOK;
			ret = send(sd,&opcode,sizeof(opcode),0);
		}
	}
	return NULL;
} 

// funzione che svolge la registrazione lato client
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
	if (!ret) ;
	if (opcode == OK){
		// procedura di registrazione andata a buon fine
		return 0;
	}
	return 1;
}

// funzione che svolge il login lato server
struct user * registerServer(int sd, char * username, char * password)
{
	natb opcode;
	natl ret;
	struct user * new_user;
	riceviLogin(sd,username,password);
	new_user = nuovoUtente(username,password);
	if (new_user){
		opcode = OK;
	} else {
		opcode = NOK;
	}
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (!ret) ;
	return new_user;
}


size_t userLogin(int sd, char * username, char * password)
{
	natl ret = 0;
	natb opcode = 0;
	printUserMenu();	
	while (1){
		scanf("%c",&opcode);
		if (opcode == '1'){
			// login branch
			opcode = LOGIN;
			break;
		} else if (opcode == '0'){
			// register branch
			opcode = REGISTER;
			break;
		}
	}
	// invio quello che voglio fare al server
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (opcode == LOGIN){
		ret = loginUtente(sd,username,password);
		if (!ret)
			printf("Login avvenuto correttamente\n");
	} else {
		ret = registerUtente(sd,username,password);
		if (!ret)
			printf("Registrazione avvenuta correttamente\n");
	}
	return 0;
}


struct user * serverLogin(int sd, char * username, char * password)
{
	struct user * utente;
	natb opcode;
	natl ret;
	// ricevo opcode dal client
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (!ret) ;
	if (opcode == LOGIN){
		// login branch
		utente = loginServer(sd,username,password);
		if (utente)
			printf("%s:logged\n",username);
	} else {
		// register branch
		utente = registerServer(sd,username,password);
		if (utente){
			printf("%s:registered\n",username);
		}
	}
	return utente;
}

