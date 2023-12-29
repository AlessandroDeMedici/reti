#include "room.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// lista di rooms;
struct des_room * room = 0;

extern fd_set master;

// id corrisponde all'id della room
struct des_room * checkRoom(natl id)
{
	struct des_room * p = room;
	while(p){
		if (p->id == id)
			return p;
		p = p->next;
	}
	return NULL;			// room non presente
}

struct des_room * createRoom(natl id)
{ 
	// inserimento in testa della nuova room
	struct des_room * p = (struct des_room *)malloc(sizeof(struct des_room));
	
	// inizializzo la room (inserimento in testa alla lista)
	memset(p,0,sizeof(struct des_room));
	p->next = room;
	p->id = id;
	pipe(p->pf);
	pipe(p->fp);
	p->numPlayers = 0;
	p->status = CREATED;

	room = p;

	return p;
}

size_t closeRoom(natl id)
{
	struct des_room * p = room;
	struct des_room * q = 0;
	while (p){
		if (p->id == id){
			if (p == room){
				room = p->next;
			} else { 
				q->next = p->next;
			}
			free(p);
			return 0;
		}
		q = p;
		p = p->next;
	}
	return 1;
}

// ATTENZIONE: dopo aver inviato sd alla room target bisogna
// comunque rimuovere sd dall'fd_set master
//size_t joinRoom(int sd, struct des_room * targetRoom)
//{
//	// invia sd alla room target
//	// ritorna 0 in caso di successo
//	// 1 altrimenti
//
//	int pf = targetRoom->pf[1];
//	int ret, inviati = 0;
//
//	// controllo sul numero di numPlayers
//	if (targetRoom->numPlayers == MAX_PLAYERS)
//		return 1;
//	while(inviati < sizeof(sd)){
//		ret = write(pf,&sd,sizeof(sd));
//		inviati += ret;
//	}
//	targetRoom->numPlayers++;
//	return 0;
//}


// INIZIO-MODIFICHE:
// queste modifiche sono state fatte perche' e' stato
// riscontrato un problema nel codice:
// 	la tabella dei file attivi di un processo
// 	e' relativa al processo, i suoi sd non sono
// 	gli stessi del processo padre, anche se ottiene
// 	l'int relativo al socket descriptor.
// 	per questo motivo PRIMA SI APRONO TUTTI I SOCKET 
// 	DEI PLAYER DELLA ROOM e successivamente si fa la
// 	fork().
// 	NUOVI PLAYER NON POSSONO FARE JOIN DELLA ROOM
// 	I PLAYER POSSONO COMUNQUE QUITTARE IN GAME 
// 	(NON E' UN BUG)
// nuova versione della join room che modifica soltanto il
// descrittore, appena sono raggiunti i MAX_PLAYER player crea la room

// funzione per inviare i player dal processo padre al processo figlio TRUE o dal processo figlio al processo padre FALSE, di default e' dal padre al figlio
size_t sendPlayer(struct des_room * p, int sd, char * username, char padre)
{
	int invio;
	int len = strlen(username);
	if (padre)
		invio = p->pf[1];
	else
		invio = p->fp[1];
	// effettua la scrittura
	write(invio,&sd,sizeof(sd));
	
	// se mi trovo da padre a figlio devo inviare anche username
	if (padre){
		write(invio,&len,sizeof(int));
		write(invio,username,len);
	}

	return 0;
}

int receivePlayer(struct des_room * p, char * username, char padre)
{
	int ricezione, sd, len;
	if (padre)
		ricezione = p->fp[0];
	else
		ricezione = p->pf[0];
	// effettua la ricezione
	read(ricezione,&sd,sizeof(sd));

	// se sono il figlio devo anche ricevere lo username
	if (!padre){
		read(ricezione,&len,sizeof(int));
		read(ricezione,username,len);
	}

	return sd;
}

// abbiamo raggiunto il numero di players necessari per avviare la room
size_t startRoom(struct des_room * p)
{
	// strutture per creare il nuovo processo
	pid_t pid;
	char stringa1[10];
	char stringa2[10];
	char stringa3[10];
	char stringa4[10];
	char stringa5[10];	// id della room

	// preparo gli argomenti da passare
	sprintf(stringa1,"%d",p->pf[0]);
	sprintf(stringa2,"%d",p->pf[1]);
	sprintf(stringa3,"%d",p->fp[0]);
	sprintf(stringa4,"%d",p->fp[1]);
	sprintf(stringa5,"%d",p->id);

	pid = fork();
	
	if (!pid){
		// processo figlio
		execl("./room","./room",stringa1,stringa2,stringa3,stringa4,stringa5,NULL);
		printf("non e' stato possibile creare il processo figlio\n");
		return 0;
	}
	// processo padre
	// deve inviare i MAX_PLAYERS player 
	for (int i = 0; i < MAX_PLAYERS; i++){
		sendPlayer(p,p->players[i],p->users[i]->username,1);
	}
	return 0;
}

size_t joinRoom(int sd, struct user * u, struct des_room * p)
{
	// controllo sullo status della room
	if (p->status != CREATED)
		return 1;
	// controllo sul numero di giocatori
	if (p->numPlayers == MAX_PLAYERS)
		return 1;
	p->players[p->numPlayers] = sd;
	p->users[p->numPlayers] = u;
	p->numPlayers++;
	// se il numero di player ha raggiunto il numero di
	// player massimi per la room allora la avvia
	if (p->numPlayers == MAX_PLAYERS){
		startRoom(p);
		p->status = STARTED;
	}
	return 0;
}

// funzione che restituisce il sd uscito dalla room e da reinserire in fd_set master
// per il momento supponiamo che se questo sd e' 0
// allora il processo non ha piu giocatori e vuole chiudersi
int backRoom(struct des_room * targetRoom)
{
	int sd, ret, ricevuti = 0;
	int fp = targetRoom->fp[0];
	while (ricevuti < sizeof(sd)){
		ret = read(fp,&sd,sizeof(sd));
		ricevuti += ret;
	}
	targetRoom->numPlayers--;
	return sd;
}

// funzione che ritorna un puntatore al descrittore di room se il file descriptor e' di una pipe di una room
struct des_room * getRoom(natl fd)
{
	struct des_room * p = room;
	while(p){
		if (p->fp[0] == fd)
			return p;
		p = p->next;
	}
	return NULL;
}

// funzione che scrive in buffer tutte le room attive in questo momento
size_t activeRooms(char * buffer)
{
	struct des_room * p = room;
	int len;
	if (!p){
		sprintf(buffer,"No active rooms\n");
		return 0;
	}
	while(p){
		sprintf(buffer,"Room %d\tnumPlayers:%d/%d\n",p->id,p->numPlayers,MAX_PLAYERS);
		len = strlen(buffer);
		buffer += len;
		p = p->next;
	}
	buffer[len] = '\0';
	return 0;
}





