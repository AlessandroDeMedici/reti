#include "room.h"

// lista globale di rooms
struct des_room * room = 0;

// descrizione:
// funzione che restituisce un puntatore al descrittore di room dato l'id
// argomenti:
// id -> id della room
// ritorno:
// la funzione ritorna un puntatore al descrittore di room se presente,
// NULL altrimenti
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

// descrizione:
// funzione che crea un nuovo descrittore di room con room_id id
// e lo inserisce nella lista di rooms
// argomenti:
// id -> id della room da creare
// ritorno:
// ritorna un puntatore al descrittore di room in caso di successo,
// NULL altrimenti
struct des_room * createRoom(natl id)
{ 
	struct des_room * p = (struct des_room *)malloc(sizeof(struct des_room));
	if (!p){
		perror("createRoom - memoria insufficiente");
		return NULL;
	}

	// inizializzo la room (inserimento in testa alla lista)
	memset(p,0,sizeof(struct des_room));
	p->next = room;
	p->id = id;
	pipe(p->pf);
	pipe(p->fp);
	p->numPlayers = 0;
	p->status = CREATED;

	// inserimento in testa della nuova room
	room = p;

	return p;
}

// descrizione:
// funzione che elimina il descrittore di room con room_id id
// argomenti:
// id -> id della room da eliminare
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 nel caso in cui la room non sia stata trovata
size_t closeRoom(natl id)
{
	// elimino il des_room con la tecnica dei due puntatori
	struct des_room * p = room;
	struct des_room * q = 0;
	while (p){
		if (p->id == id){
			if (!q){
				// mi trovo in cima alla lista
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


// descrizione:
// funzione per inviare un file descriptor dalla home alla room quando padre vale TRUE
// da room a padre quando padre vale FALSE
// argomenti:
// p -> puntatore al descrittore di room
// sd -> socket descriptor da inviare
// username -> stringa che contiene lo username
// padre -> variabile logica per stabilire il verso dell'invio
// ritorno:
// ritorna 0 in caso di successo, 1 in caso di errore (ad esempio invio incompleto)
size_t sendPlayer(struct des_room * p, int sd, char * username, char padre)
{
	int invio;
	int len = strlen(username) + 1;
	int ret;

	if (!p)
		return 1;

	if (padre)
		invio = p->pf[1];
	else
		invio = p->fp[1];

	// effettua la scrittura
	ret = write(invio,&sd,sizeof(sd));
	if (ret < sizeof(sd))
		return 1;

	// se mi trovo da padre a figlio devo inviare anche username
	if (padre){
		ret = write(invio,&len,sizeof(int));
		if (ret < sizeof(int))
			return 1;
		write(invio,username,len);
		if (ret < len)
			return 1;
	}

	return 0;
}

// descrizione:
// funzione che crea il processo room
// parametri:
// p -> descrittore della room da avviare
// ritorno:
// la funzione ritorna 0 in caso di successo, 1 in caso di errore
size_t startRoom(struct des_room * p)
{
	int i;
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
	for (i = 0; i < MAX_PLAYERS; i++){
		sendPlayer(p,p->players[i],p->users[i]->username,1);
	}

	// setto la stanza a STARTED
	p->status = STARTED;
	
	return 0;
}

// descrizione:
// funzione che si occupa di inviare i player nelle room
// argomenti:
// sd -> descrittore del socket da inviare alla room
// u -> puntatore all'utente da inviare
// p -> puntatore al descrittore della room
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 se non e' riuscita ad invaire il socket sd
size_t joinRoom(int sd, struct user * u, struct des_room * p)
{
	// controllo sullo status della room
	if (p->status != CREATED)
		return 1;
	// controllo sul numero di giocatori
	if (p->numPlayers == MAX_PLAYERS)
		return 1;

	// aggiungo il player al descrittore di room
	p->players[p->numPlayers] = sd;
	p->users[p->numPlayers] = u;
	p->numPlayers++;

	// se il numero di player ha raggiunto il numero di
	// player massimi per la room allora la avvia
	if (p->numPlayers == MAX_PLAYERS){
		startRoom(p);
	}
	return 0;
}

// descrizione:
// funzione che ritorna un puntatore al descrittore di room
// se il fd e' un file descriptor della pipe di una room (fp[0])
// argomenti:
// fd -> indice del file descriptor
// ritorno:
// la funzione ritorna un puntatore al descrittore di room in caso di successo,
// NULL altrimenti
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

// descrizione:
// funzione che scrive in buffer tutte le room attive in questo momento
// argomenti:
// buffer -> array di caratteri che conterra la stringa da inviare
// ritorno:
// la funzione ritorna 0 in caso di successo,
// 1 nel caso non siano presenti room
size_t activeRooms(char * buffer)
{
	struct des_room * p = room;
	int len;
	if (!p){
		return 1;
	}
	while(p){

		char joinable = 0;
		if (p->status == CREATED)
			joinable = 1;

		sprintf(buffer,"Room %d\tnumPlayers:%d/%d\tjoinable:%d\n",p->id,p->numPlayers,MAX_PLAYERS,joinable);
		len = strlen(buffer);
		buffer += len;
		p = p->next;
	}
	buffer[len] = '\0';
	return 0;
}

#ifdef SERVER
// codice del server


// descrizione:
// funzione che si occupa di inviare i player nelle room e di aggiornare
// fd_set master e descrittore di room
// argomenti:
// sd -> descrittore del socket da inviare alla room
// max_sd -> indice del piu alto descrittore di socket in master
// master -> fd_set da aggiornare
void avviaRoom(int sd, int *max_sd, fd_set * master)
{
	struct des_connection * conn = NULL;
	int ret = 0;
	natl room_id = 0;
	natb opcode = NOK;
	struct des_room * stanza;

	// ottengo il descrittore di connessione
	conn = getConnessione(sd);

	// ricevo il room_id
	ret = recv(sd,&room_id,sizeof(room_id),0);
	if (!ret){
		perror("avviaRoom - client disconnesso");
		return;
	}
	room_id = ntohl(room_id);
	printf("(Main) (%d) vuole entrare nella room (%d)\n",sd,room_id);

	// ottengo il descrittore di room
	stanza = checkRoom(room_id);

	if (!stanza){
		// la room non era presente, quindi devo crearla
		stanza = createRoom(room_id);
		printf("(Main) La room %d non esisteva, e' stata creata\n",room_id);
		
		// se e' stata creata bisogna inserire fp[0] della pipe in master
		FD_SET(stanza->fp[0],master);
		if (stanza->fp[0] > *max_sd)
			*max_sd = stanza->fp[0];

	} else {
		// la room esisteva di gia
		printf("(Main) La room %d esisteva\n",room_id);
	}

	// invio il socket sd nella room
	ret = joinRoom(sd,conn->utente,stanza);

	if (!ret){
		// joinRoom ha ritornato 0 quindi sd e' stato inviato con successo
		playingConnessione(sd);
		printf("(Main) il socket (%d) e' entrato nella room (%d)\n",sd,room_id);
		FD_CLR(sd,master);	
	} else { 
		// joinRoom ha ritornato 1 quindi non e' stato inviato alla room
		printf("(Main) il socket (%d) non e' entrato nella room %d perche era piena\n",sd,room_id);
		// in questo caso notifichiamo al client che non e' stato possibile entrare con un NOK
		ret = send(sd,&opcode,sizeof(opcode),0);
		if (ret == -1)
			perror("avviaRoom - errore in fase di send");
	}
}

// descrizione:
// funzione che si occupa di ricevere i player che arrivano dalle room
// ed aggiornare fd_set master e descrittore di room
// argomenti:
// sd -> file descriptor pronto (associato ad una pipe)
// master -> fd_set da aggiornare
// stanza -> descrittore di room da aggiornare
void tornaIndietro(int sd, fd_set * master, struct des_room * stanza)
{
	// questo e' un fd relativo ad una pipe che ci sta inviando un socket indietro
	int ricevuti = 0, back_sd = 0, ret = 0;
	natl room_id = stanza->id;

	// modifico lo stato della stanza
	stanza->status = QUITTING;

	// ricevo il socket descriptor
	while(ricevuti < sizeof(back_sd)){
		ret = read(sd,&back_sd + ricevuti,sizeof(back_sd) - ricevuti);
		ricevuti += ret;
	}

	printf("(Main) il socket (%d) sta tornando alla home dalla room %d\n",back_sd, room_id);
	printf("(Main) giocatori nella room %d: %d -> %d\n",room_id,stanza->numPlayers,stanza->numPlayers - 1);
	
	// aggiorno il numero id player nella stanza
	stanza->numPlayers--;
	
	// se non ci sono piu giocatori nella stanza dealloca il des_room
	// e rimuovi il fd dal set master
	// (il processo room fa exit automaticamente)
	
	if (!stanza->numPlayers){
		// elimina la room
		if(closeRoom(room_id))
			// si e' verificato un errore
			printf("(Main) room %d non chiusa correttamente\n",room_id);
		printf("(Main) la room %d è stata chiusa\n",room_id);
		FD_CLR(stanza->fp[0],master);
	}

	// reinserisci sd in master
	homeConnessione(back_sd);
	FD_SET(back_sd,master);
}

// descrizione:
// funzione che ritorna 1 se non c'e' nessuna room attiva
size_t nessunaRoom()
{
	if (!room)
		return 1;
	return 0;
}
#endif


