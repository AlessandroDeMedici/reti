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
	int ret, len = strlen(string) + 1, sent = 0;
	if (string[0] == '\0' || string[0] == '\n')
		len = 0;
	natl length = htonl(len);

	while (sent < sizeof(natl)){
		ret = send(sd,&length + sent,sizeof(natl) - sent,0);
		if (!ret){
			perror("sendString - errore in fase di send");
			return 1;
		}
		sent += ret;
	}

	if (!len)
		return 0;

	sent = 0;
	while (sent < len){
		ret = send(sd,string + sent,len - sent,0);
		if (!ret){
			perror("sendString - errore in fase di send");
			return 1;
		}
		sent += ret;
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
	int len, ret, received = 0;
	natl length;
	
	while (received < sizeof(natl)){
		ret = recv(sd,&length + received,sizeof(natl) - received,0);
		if (!ret){
			perror("receiveString - errore in fase di recv");
			return 1;
		}
		received += ret;
	}

	len = ntohl(length);
	
	if (!len)
		return 0;


	received = 0;
	while (received < len){
		ret = recv(sd,string + received,len - received,0);
		if (!ret){
			perror("receiveString - errore in fase di recv");
			return 1;
		}
		received += ret;
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

