#include "utility.h"


// descrizione:
// funzione generica per l'invio di stringhe
// il protocollo di invio e':
// 	1. invio la dimensione come naturale su 4 byte
// 	2. invio la stringa
// argomenti:
// sd -> descrittore del socket
// string -> stringa da inviare
// ritorno:
// la funzione ritorna 0 in caso di successo 1 altrimenti
size_t sendString(int sd, char * string)
{ 
	int ret, len = strlen(string) + 1;
	if (string[0] == '\0' || string[0] == '\n')
		len = 0;
	natl length = htonl(len);
	ret = send(sd,&length,sizeof(natl),0);
	if (!ret){
		perror("sendString - errore in fase di recv");
		return 1;
	}

	if (!len)
		return 0;
	ret = send(sd,string,len,0);
	if (!ret){
		perror("sendString - errore in fase di send");
		return 1;
	}
	return 0;
}

// descrizione:
// funzione generica per la ricezione di stringhe
// il protocollo di ricezione e':
// 	1. ricevo la dimensione come naturale su 4 byte
// 	2. ricevo la stringa
// argomenti:
// sd -> descrittore del socket
// string -> stringa da ricevere
// ritorno:
// la funzione ritorna 0 in caso di successo
size_t receiveString(int sd, char * string)
{ 
	int len, ret;
	natl length;
	ret = recv(sd,&length,sizeof(natl),0);
	if (!ret){
		perror("receiveString - errore in fase di recv");
		return 1;
	}
	len = ntohl(length);
	if (!len)
		return 0;
	ret = 0;
	ret = recv(sd,string,len,0);
	if (!ret){
		perror("receiveString - errore in fase di recv");
		return 1;
	}
	return 0;
}


// descrizione:
// funzione che stampa in modo animato il testo
// l'animazione consiste nell'emulare il il typing
// argomenti:
// buffer -> stringa da stampare
void stampaAnimata(char * buffer)
{
	int i;
	for (i = 0; buffer[i] != '\0'; i++){
		printf("%c",buffer[i]);
		fflush(stdout);
		usleep(10000);
	}
}

