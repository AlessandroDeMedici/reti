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
	printf("Login: 1\nRegister: 0\n");
	printf("*********************************\n");
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
	int ret;
	ret = send(sd,&opcode,sizeof(opcode),0);
	receiveString(sd,buffer);
	printf("LISTA DELLE ROOMS:\n%s",buffer);
}

int avviaRoom(int sd, char * arg1)
{
	natb opcode = START_ROOM;
	natl room;
	int ret;
	// acquisisco il numero di room
	sscanf(arg1,"%d",&room);

	// invio il messaggio di START_ROOM
	ret = send(sd,&opcode,sizeof(opcode),0);

	// invio il numero di room
	room = htonl(room);
	ret = send(sd,&room,sizeof(room),0);

	// attendo il messaggio di OK
	printf("In attesa di entrare nella room %d...\n",htonl(room));
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (opcode == OK){
		printf("Benvenuto nella room!\n");
		return 0;
	} else
		return 1;
}




