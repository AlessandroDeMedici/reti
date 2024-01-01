#include "menu.h"
#include "costanti.h"
#include <stdio.h>
#include "utility.h"
#include "game/game.h"

// funzione che stampa il menu di login
// per il server o per il client
void printUserMenu()
{
	printf("********** ESCAPE ROOM **********\n");
	printf("Benvenuto sul server\n");
	printf("login: fai login\nregister: registati\n");
	printf("*********************************\n");
	printf("> ");
}

void printHome()
{
	printf("********** ESCAPE ROOM **********\n");
	printf("Comandi:\n");
	printf("start [room]: avvia la room con ID [room]\n");
	printf("list: controlla tutte le rooma attive\n");
	printf("end: esci e chiudi la connessione\n");
	printf("*********************************\n");
}

void roomList(int sd)
{
	natb opcode = ROOM_LIST;
	char buffer[256];
	send(sd,&opcode,sizeof(opcode),0);
	receiveString(sd,buffer);
	printf("LISTA DELLE ROOMS:\n%s",buffer);
}

int avviaRoom(int sd, char * arg1)
{
	natb opcode = START_ROOM;
	natl room;
	// acquisisco il numero di room
	sscanf(arg1,"%d",&room);

	// invio il messaggio di START_ROOM
	send(sd,&opcode,sizeof(opcode),0);

	// invio il numero di room
	room = htonl(room);
	send(sd,&room,sizeof(room),0);

	// attendo il messaggio di OK
	printf("In attesa che gli altri giocatori entrino nella room %d...\n",htonl(room));
	recv(sd,&opcode,sizeof(opcode),0);
	if (opcode == OK){
		printf("Benvenuto nella room!\n");
		return 0;
	} else
		return 1;
}

void printHelp()
{
	printf("******************* Buona partita ******************\n");
	printf("Comandi:\nlook [oggetto|location]\t\tstampa la descrizione di un oggetto o location\n");
	printf("take [oggetto]\t\t\tsblocca o ottieni un oggetto\n");
	printf("use [oggetto1] [oggetto2]\tusa un oggetto\n");
	printf("time\t\t\t\tottieni il tempo rimanente\n");
	printf("exit\t\t\t\tesci dal gioco\n");
	printf("****************************************************\n");
}



