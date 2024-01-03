#include "game.h"

struct room stanza;	// struttura globale room
struct game gioco;	// struttura globale gioco
struct oggetto oggetti[MAX_OGGETTI]; // struttura globale oggetti
struct location locazioni[MAX_LOCAZIONI]; // strutture globale locazioni
struct ricetta ricette[MAX_RICETTE]; // struttura globale ricette

// strutture per gestire i messaggi
char buffer[1024]; // buffer globale messaggio
natb buffer_id = 0xFF; // id del player che ha inviato il messaggio (0xFF se non presente)
natb sent[MAX_PLAYERS];	// giocatori a cui e' stato inviato il messaggio
natb now = 0;

#ifdef SERVER
// codice del server

// i giocatori vengono gestiti tramite una pila grande MAX_PLAYERS
// con indice la variabile indice

int indice = 0; // indice della prima locazione libera in giocatori

// pila di giocatori
struct player giocatori[MAX_PLAYERS];

// descrizione:
// funzione chiamata dal server per inserire 
// un nuovo player all'interno della pila
void addPlayer(char * username, int sd)
{
	if (indice >= MAX_PLAYERS)
		return;
	strcpy(giocatori[indice].username,username);
	giocatori[indice].sd = sd;
	giocatori[indice].p = 1;
	indice++;
}

// descrizione:
// funzione lanciata dal server
// per rispondere alla richiesta di invia messaggio
// argomenti:
// sd -> descrittore del socket
void inviaMessaggio(int id,int sd)
{
	char * temp = buffer;
	natb pid = getPlayerId(sd);
	natb opcode = NOK;
	int i, ret;

	// controllo se posso inviare, altrimenti invio un NOK
	// questo e' il raro caso in cui il server soddisfi due richieste di tell allo stesso
	// ciclo, in questo caso il messaggio dalla seconda richiesta in poi vengono scartati
	if (buffer_id != 0xFF){
		ret = send(sd,&opcode,sizeof(opcode),0);
		if (!ret)
			perror("inviaMessaggio - errore in fase di send");
		return;
	}

	// invio un OK
	opcode = OK;
	ret = send(sd,&opcode,sizeof(opcode),0);

	// adesso attendo la stringa
	sprintf(temp,"%s: ",giocatori[pid].username);
	temp += strlen(buffer);
	receiveString(sd,temp);
	printf("(%d) il socket (%d) ha inviato il messaggio: %s\n",id,sd,buffer);

	// inizializzo le strutture
	buffer_id = pid;
	for (i = 0; i < MAX_PLAYERS; i++){
		sent[i] = 0;
	}
	sent[pid] = 1;
}

// descrizione:
// funzione lanciata dal server
// per rispondere alla richiesta di ricevi messaggio
// argomenti:
// sd -> descrittore del socket
void riceviMessaggio(int sd)
{
	natl len = 0;
	int ret = 0;
	int i;
	natb pid = getPlayerId(sd);
	if (buffer[0] != '\0' && (!sent[pid])){
		// deve essere presente una stringa e non deve essere gia stata inviata a questo player
		sendString(sd,buffer);
		sent[pid] = 1;
	} else {
		// se non e' presente alcuna stringa
		len = 0;
		ret = send(sd,&len,sizeof(len),0);
		if (!ret)
			perror("riceviMessaggio - errore in fase di invio");
	}
	// libero buffer_id
	for (i = 0; i < MAX_PLAYERS; i++){
		if (!sent[i])
			break;
		if (i == MAX_PLAYERS - 1)
			buffer_id = 0xFF;
	}
}

// descrizione:
// funzione chiamata dal server per rispondere 
// alla richiesta di aggiornamento dell'oggetto
// argomenti:
// sd -> descrittore del socket
void aggiornaOggetto(int sd)
{
	natb id = 0;
	struct oggetto * o;
	// ricevo id dell'oggetto
	recv(sd,&id,sizeof(id),0);
	o = &oggetti[id];
	// invia lo status aggiornato dell'oggetto
	send(sd,&o->status,sizeof(o->status), 0);
}

// descrizione:
// funzione chiamata dal server per rispondere
// alla richiesta di UNLOCK di un oggetto
// argomenti:
// sd -> descrittore del socket
void sbloccaOggetto(int sd)
{
	natb id = 0;
	struct oggetto * o = NULL;
	// ricevo id
	recv(sd,&id,sizeof(id),0);
	// sblocco l'oggetto
	o = &oggetti[id];
	o->status = FREE;
}

// descrizione:
// funzione chiamata dal server per rispondere
// alla richiesta di TAKE
// argomenti:
// sd -> descrittore del socket
void ottieniOggetto(int sd)
{ 
	natb id = 0;
	// devo ottenere l'indice del player
	natb pid = getPlayerId(sd);

	recv(sd,&id,sizeof(id),0);

	struct oggetto * o = &oggetti[id];

	// devo metterlo nel suo inventario
	aggiungiInventario(pid,o);
	// devo cambiare lo status dell'oggetto
	// in TAKEN
	o->status = TAKEN;
}

// descrizione:
// funzione che ritorna l'indice del player dato il suo
// socket descriptor
// argomenti:
// sd -> socket descriptor del player
// ritorno:
// la funzione ritorna l'id del player in caso di successo,
// 0xFF altrimenti
natb getPlayerId(int sd)
{
	natb id = 0;
	for (;id < MAX_PLAYERS;id++){
		if (!giocatori[id].p)
			continue;
		if (giocatori[id].sd == sd)
			return id;
	}

	printf("getPlayerId - player not found\n");
	return 0xFF;
}

// descrizione:
// trova nell'inventario del player id l'oggetto di nome c
// e ritorna un puntatore all'oggetto
// argomenti:
// id -> id del player
// c -> stringa che contiene il nome dell'oggetto
// ritorno:
// la funzione ritorna un puntatore all'oggetto in caso di successo,
// NULL altrimenti
struct oggetto * findOggettoInventario(natl id,char * c)
{
	int i;
	if (!c)
		return NULL;
	for (i = 0; i < INVENTARIO; i++){
		if (!giocatori[id].inventario[i])
			continue;
		if (!strcmp(giocatori[id].inventario[i]->nome,c)){
			return giocatori[id].inventario[i];
		}
	}
	return NULL;
}


// descrizione:
// funzione chiamata dal server per
// rimuovere l'oggetto o dall'inventario del giocatore id
// argomenti:
// id -> id del player
// o -> puntatore all'oggetto da rimuovere
void rimuoviInventario(natb id,struct oggetto * o)
{
	int i;
	if (!o)
		return;
	for (i = 0; i < INVENTARIO; i++){
		if (giocatori[id].inventario[i] == o)
			giocatori[id].inventario[i] = NULL;
	}
 }


// descrizione:
// funzione chiamata dal server per
// aggiungere l'oggetto o all'inventario del giocatore id
// argomenti:
// id -> id del player
// o -> puntatore all'oggetto da aggiungere
size_t aggiungiInventario(natb id,struct oggetto * o)
{
	int i;
	if (!o)
		return 0;

	for (i = 0; i < INVENTARIO; i++){
		if (!giocatori[id].inventario[i]){
			giocatori[id].inventario[i] = o;
			return 0;
		}
	}
	return 1;
} 

// descrizione:
// funzione chiamata dal server per rispondere
// alla richiesta di use
// argomenti:
// sd -> descrittore del socket
void usaOggetto(int sd)
{
	natb id1, id2 = 0xFF, pid;
	struct ricetta * r = NULL;
	struct oggetto * o1, * o2 = NULL;
	
	// ricevo gli id degli oggetti
	recv(sd,&id1,sizeof(id1),0);
	recv(sd,&id2,sizeof(id2),0);
	
	// ottengo i puntatori agli oggetti
	o1 = &oggetti[id1];
	if (id2 != 0xFF)
		// alcune ricette prevedono che ci sia un solo oggetto come argomento
		o2 = &oggetti[id2];

	// ottengo il player id
	pid = getPlayerId(sd);
	
	r = findRicetta(o1,o2);
	// compio l'azione
	// non viene inviato un messaggio di OK o NOK perche' l'azione viene validata
	// prima di essere inviata
	switch(r->action){
		case (GIVE):
			rimuoviInventario(pid,o1);
			rimuoviInventario(pid,o2);
			aggiungiInventario(pid,r->dest);
			r->dest->status = TAKEN;
			break;
		case (UNLOCK):
			r->dest->status = FREE;
		case (TOKEN):
			rimuoviInventario(pid,o1);
			gioco.token++;
			break;
	}
}

// descrizione:
// funzione per stampare tutti gli oggetti
// nell'inventario di un giocatore
// argomenti:
// sd -> descrittore del socket del player
void objs(int sd)
{
	
	int i;
	natb id = getPlayerId(sd);

	for (i = 0; i < INVENTARIO; i++){
		printf("%d: ", i + 1);
		if (giocatori[id].inventario[i])
			printf("%s\n",giocatori[id].inventario[i]->nome);
		else
			printf("vuoto\n");
	}
}

// descrizione:
// funzione che dato l'opcode elabora la richiesta del client
// argomenti:
// id -> descrittore del socket
// opcode -> opcode ricevuto
void game(int id, int sd, natb opcode)
{
	switch (opcode){
		case (UPDATE_OBJECT):
			printf("(%d) il socket (%d) ha fatto aggiornaOggetto()\n",id,sd);
			aggiornaOggetto(sd);
			break;
		case (UNLOCK):
			printf("(%d) il socket (%d) ha fatto sbloccaOggetto()\n",id,sd);
			sbloccaOggetto(sd);
			break;
		case (TAKE):
			printf("(%d) il socket (%d) ha fatto ottieniOggetto()\n",id,sd);
			ottieniOggetto(sd);
			break;
		case (USE):
			printf("(%d) il socket (%d) ha fatto usaOggetto()\n",id,sd);
			usaOggetto(sd);
			break;
		case (UPDATE_TOKEN):
			printf("(%d) il socket (%d) ha fatto getToken()\n",id,sd);
			getToken(sd);
			break;
		case (INC_TOKEN):
			printf("(%d) il socket (%d) ha fatto token()\n",id,sd);
			token();
			break;
		case (GET_TIME):
			printf("(%d) il socket (%d) ha fatto ottieniTempo()\n",id,sd);
			ottieniTempo(sd);
			break;
		case (RICEVI_MESSAGGIO):
			printf("(%d) il socket (%d) ha fatto riceviMessaggio()\n",id,sd);
			riceviMessaggio(sd);
			break;
		case (INVIA_MESSAGGIO):
			printf("(%d) il socket (%d) ha fatto inviaMessaggio()\n",id,sd);
			inviaMessaggio(id,sd);
			break;
	}	
}

// descrizione:
// funzione per sbloccare i player per iniziare a giocare
void sbloccaPlayers()
{
	int i;
	natb opcode = OK;

	// ricordiamo che quando i client fanno avviaRoom rimangono
	// in attesa di un messaggio
	// quando la room e' piena tutti i giocatori ricevono un messaggio di OK
	for (i = 0; i < MAX_PLAYERS; i++)
		if (giocatori[i].p)
			send(giocatori[i].sd,&opcode,sizeof(opcode),0);
}

// descrizione:
// funzione lanciata dal server per rispondere alla richiesta di UPDATE_TOKEN
// argomenti:
// sd -> descrittore del socket
void getToken(int sd)
{
	send(sd,&gioco.token,sizeof(gioco.token),0);
}

// descrizione:
// funzione lanciata dal server per rispondere alla richiesta di INC_TOKEN
void token()
{
	gioco.token++;
}

// descrizione:
// funzione lanciata dal server per impostare lo start time del gioco
void startTime()
{
	time(&gioco.start_time);
}

// descrizione:
// funzione lanciata dal server per rispondere alla richiesta di GET_TIME
// argomenti:
// sd -> descrittore del socket
void ottieniTempo(int sd)
{
	time_t start_time = gioco.start_time;
	
	start_time = htonl(start_time);
	
	send(sd,&start_time,sizeof(start_time),0);
}


// descrizione:
// funzione che libera tutti gli oggetti nell'inventario di un giocatore
// che sta uscendo dalla room
// argomenti:
// sd -> descrittore del socket
void quitRoom(int sd)
{
	int i;
	// ottengo l'id del player
	natb id = getPlayerId(sd);

	// libero tutti gli oggetti
	for (i = 0; i < INVENTARIO; i++){
		if (giocatori[id].inventario[i])
			giocatori[id].inventario[i]->status = FREE;
	}
}

#else
// codice del client

int sd;			// descrittore del server (globale)

// descrizione:
// funzione chiamata dal client per inizializzare il descrittore del server
// argomenti:
// sockd -> descrittore del socket del server
void initsd(int sockd)
{
	sd = sockd;
}

// struttura globale del player (lato client)
struct player giocatore;

// descrizione:
// funzione chiamata dal client per inviare una richiesta di INVIA_MESSAGGIO
// argomenti:
// arg -> stringa che contiene il messaggio da inviare
void inviaMessaggio(char * arg)
{
	// invio opcode
	natb opcode = INVIA_MESSAGGIO;
	int ret = send(sd,&opcode,sizeof(opcode),0);
	
	// attendo per un ok o un nok
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (ret <= 0){
		perror("Inviamessaggio - errore in fase di receive");
	}
	if (opcode == NOK){
		printf("Non e' stato possibile inviare il messaggio\n");
		return;
	}

	if (ret <= 0) {
		perror("inviaMessaggio - errore in fase di send");
	}

	// invio la stringa
	sendString(sd,arg);
}


// descrizione:
// funzione chiamata dal client per inviare una richiesta di RICEVI_MESSAGGIO
void riceviMessaggio()
{
	// invio opcode
	natb opcode = RICEVI_MESSAGGIO;
	int ret = send(sd,&opcode,sizeof(opcode),0);
	natl len = 0;
	if (ret <= 0) {
		perror("riceviMessaggio - errore in fase di send");
	}

	// ricevo la lunghezza della stringa
	ret = recv(sd,&len,sizeof(len),0);
	len = ntohl(len);
	if (!len){
		// se la lunghezza e' nulla ritorno
		return;
	}
	
	// ricevo la stringa
	ret = recv(sd,buffer,len,0);
	printf("\t\t");
	stampaAnimata(buffer);
	printf("\n");
}

// descrizione:
// funzione chiamata dal client per inviare una richiesta di UPDATE_OBJECT
// argomenti:
// o -> puntatore all'oggetto da aggiornare
void aggiornaOggetto(struct oggetto * o)
{
	int ret;
	natb status = 0;
	// si richiede lo status dell'oggetto
	natb opcode = UPDATE_OBJECT;
	natb id = getObjectId(o);
	
	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);
	
	// invio id dell'oggetto
	send(sd,&id,sizeof(id),0);
	
	// ricezione dello status
	ret = recv(sd,&o->status,sizeof(status),0);
	
	if (!ret) // disconnessione
		printf("aggiornaOggetto - server disconnesso\n");
} 

// descrizione:
// funzione chiamata dal client per inviare una richiesta di UNLOCK
// argomenti:
// o -> puntatore all'oggetto da sbloccare
void sbloccaOggetto(struct oggetto * o)
{
	natb id = getObjectId(o);
	natb opcode = UNLOCK;
	int ret;

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (!ret){
		perror("sbloccaOggetto - errore in fase di send");
		return;
	}
	// invio id
	ret = send(sd,&id,sizeof(id),0);
	if (!ret){
		perror("sbloccaOggetto - errore in fase di send");
		return;
	}
}

// descrizione:
// funzione chiamata dal client per inviare una richiesta di TAKE
// argomenti:
// o -> puntatore all'oggetto da ottenere
void ottieniOggetto(struct oggetto * o)
{
	natb id = getObjectId(o);
	natb opcode = TAKE;

	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);
	// invio id
	send(sd,&id,sizeof(id),0);
}


// descrizione:
// funzione chiamata dal client per trovare all'interno dell'inventario
// l'oggetto di nome c
// argomenti:
// c -> stringa che contiene il nome dell'oggetto
// ritorno:
// la funzione ritorna un puntatore all'oggetto in caso di successo,
// NULL altrimenti
struct oggetto * findOggettoInventario(char * c)
{
	int i;
	if (!c)
		return NULL;
	for (i = 0; i < INVENTARIO; i++){
		if (!giocatore.inventario[i])
			continue;
		if (!strcmp(giocatore.inventario[i]->nome,c)){
			return giocatore.inventario[i];
		}
	}
	return NULL;
}

// descrizione:
// funzione chiamata dal client per elaborare il comando di look
// argomenti:
// c -> stringa che contiene l'argomento (NULL, nome di location o di oggetto)
void look(char * c)
{
	struct location * l;
	struct oggetto * o;

	// look da sola stampa la descrizione della room
	if (!c){
		stampaRoom();
		return;
	}

	// cerchiamo un nome di location
	l = findLocation(c);
	if (l){
		stampaLocation(l);
		return;
	}

	// cerchiamo infine un nome di oggetto, diamo precedenza agli oggetti
	// nell'inventario (non ho bisogno di aggiornarli prima di stamparli)
	o = findOggettoInventario(c);
	if (o){
		stampaOggetto(o);
		return;
	}

	// devo cercare l'oggetto fra tutti gli oggetti
	o = findOggetto(c);
	if (!o){
		printf("look - no object found\n");
		return;
	}
	
	// aggiorno lo stato dell'oggetto
	aggiornaOggetto(o);

	// se l'oggetto e' HIDDEN l'oggetto non viene trovato
	if (o->status == HIDDEN)
		o = NULL;

	if (o){
		// stampo la descrizione dell'oggetto
		stampaOggetto(o);
		return;
	}

	printf("look - no object found\n");

}

// descrizione:
// funzione chiamata dal client per elaborare il comando di take
// argomenti:
// c -> stringa che contiene l'argomento (nome di oggetto)
void take(char *c)
{
	// controllo sugli input
	if (!c){
		printf("take - bad argument\n");
		return;
	}

	// provo ad ottenere l'oggetto
	struct oggetto * o = findOggetto(c);
	if (!o){
		// oggetto non trovato
		printf("take - non-existing object\n");
		return;
	}
	
	// se trovo l'oggetto devo prima aggiornarne lo status
	aggiornaOggetto(o);
	
	// se e' HIDDEN non posso trovarlo
	if (o->status == HIDDEN)
		o = NULL;
	
	if (!o){
		// oggetto non trovato
		printf("take - non-existing object\n");
		return;
	}

	// se l'oggetto e' gia' stato preso non posso 
	// prenderlo nuovamente (questo funziona anche se il player
	// prova a prendere un oggetto che gia possiede)
	if (o->status == TAKEN)
	{
		printf("L'oggetto e' gia stato preso\n");
		return;
	}
	
	// se l'oggetto e' bloccato provo a sbloccarlo
	if (o->status == BLOCCATO){
		// se l'oggetto e' stato sbloccato
		if (!sblocca(o))
			printf("Hai sbloccato %s\n",o->nome);
		// l'utente ha fatto exit
	} else if (o->status == FREE) {
		ottieni(o);
		printf("Hai ottenuto %s\n",o->nome);
	}

}


// descrizione:
// funzione chiamata dal client che si occupa di gestire la risoluzione
// dell'enigma
// argomenti:
// o -> puntatore all'oggetto da sbloccare
// ritorno:
// la funzione ritorna 0 se l'utente e' riuscito a risolvere l'enigma
// e sbloccare l'oggetto
// 1 altrimenti
size_t sblocca(struct oggetto * o)
{
	char buffer[64];
	printf("%s\n",o->enigma);
	while(1){
		int i;
		fgets(buffer,63,stdin);
		for (i = 0; i < 63; i++){
			if (buffer[i] == '\n')
			{
				buffer[i] = '\0';
				break;
			}
		}
		if (!strcmp(buffer,o->risposta)){ // devo sbloccare l'oggetto
			printf("\r\033[KCorretto!\n");
			o->status = FREE;
			sbloccaOggetto(o);
			return 0;
		} else if (!strcmp(buffer,"exit")){ // il player non vuole piu indovinare per ora
			return 1;
		} else { // errore (aggiungere tentativi--)
			printf("\r\033[KSbagliato\n");
		}
	}
}

// descrizione:
// funzione chiamata dal client che si occupa di gestire l'ottenimento di un oggetto
// argomenti:
// o -> puntatore all'oggetto che si vuole ottenere
void ottieni(struct oggetto * o)
{
	int i;	
	for (i = 0; i < INVENTARIO; i++){
		// se trovo uno slot libero nell'inventario
		// allora ci inserisco l'oggetto
		if (giocatore.inventario[i])
			continue;
		giocatore.inventario[i] = o;
		o->status = TAKEN;
		ottieniOggetto(o);
		return;
	}
	// non c'erano slot liberi
	printf("Inventario pieno!\n");
}


// descrizione:
// funzione chiamata dal client per rimuovere un oggetto
// dal proprio inventario
// argomenti:
// o -> puntatore all'oggetto da rimuovere dall'inventario
void rimuoviInventario(struct oggetto * o)
{
	int i;
	if (!o)
		return;
	for (i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i] == o)
			giocatore.inventario[i] = NULL;
	}
}

// descrizione:
// funzione chiamata dal client per aggiungere un oggetto
// al proprio inventario
// argomenti:
// o -> puntatore all'oggetto da aggiungere all'inventario
size_t aggiungiInventario(struct oggetto * o)
{
	int i;
	if (!o)
		return 0;

	for (i = 0; i < INVENTARIO; i++){
		if (!giocatore.inventario[i]){
			giocatore.inventario[i] = o;
			return 0;
		}
	}
	return 1;
} 

// descrizione:
// funzione chiamata dal client per inviare una richiesta di USE al server
// (questa viene chiamata solo dopo che e' stata trovata una ricetta adeguata)
// argomenti:
// o1 -> puntatore al primo oggetto della ricetta
// o2 -> puntatore al secondo oggetto della ricetta
void usaOggetto(struct oggetto * o1, struct oggetto * o2)
{
	natb id1, id2 = 0xFF, opcode = TAKE;
	// ottengo gli id degli oggetti
	id1 = getObjectId(o1);
	if (o2)
		id2 = getObjectId(o2);

	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);

	// invio id degli oggetti
	send(sd,&id1,sizeof(id1),0);
	send(sd,&id2,sizeof(id2),0);
}

// descrizione:
// funzione chiamata dal client per elaborare il comando di use
// argomenti:
// obj1 -> stringa che contiene il nome del primo oggetto
// obj2 -> stringa che contiene il nome del secondo oggetto
void use(char * obj1, char * obj2)
{
	struct oggetto * o1 = NULL, *o2 = NULL;
	struct ricetta * r;
	
	if (!obj1)
	{
		printf("use - need at least 1 argument\n");
		return;
	}

	// ottengo i puntatori agli oggetti
	o1 = findOggettoInventario(obj1);
	if (!o1){
		printf("use - object 1 not in inventory\n");
		return;
	}
	if (obj2)
		o2 = findOggettoInventario(obj2);

	// cerco la ricetta
	r = findRicetta(o1,o2);
	if (!r){
		printf("use - no recipe found\n");
		return;
	}

	// a questo punto sono sicuro che la ricetta esiste
	
	// segnalo il comando al server
	usaOggetto(o1,o2);

	// compio l'azione della ricetta
	switch(r->action){
		case (GIVE):
			rimuoviInventario(o1);
			rimuoviInventario(o2);
			aggiungiInventario(r->dest);
			r->dest->status = TAKEN;
			printf("Hai ottenuto %s\n",r->dest->nome);
			break;
		case (UNLOCK):
			r->dest->status = FREE;
			printf("Hai sbloccato %s\n",r->dest->nome);
		case (TOKEN):
			rimuoviInventario(o1);
			gioco.token++;
			printf("Hai ottenuto un token!\n");
			token();
			break;
	}
}

// descrizione:
// funzione lanciata dal client per stampare tutti gli oggetti nel
// proprio inventario
void objs()
{
	int i;
	for (i = 0; i < INVENTARIO; i++){
		printf("%d: ", i + 1);
		if (giocatore.inventario[i])
			printf("%s\n",giocatore.inventario[i]->nome);
		else
			printf("vuoto\n");
	}
}

// descrizione:
// funzione lanciata dal client per inviare un messaggio
// di QUIT_ROOM
void quitRoom()
{
	natb opcode = QUIT_ROOM;
	send(sd,&opcode,sizeof(opcode),0);
	
	printf("Sto tornando alla home...\n");
	printHome();
}


// descrizione:
// funzione lanciata dal client per gestire
// i comandi inviati durante il gioco
void game()
{
	char buffer[1024];
	char * command;
	char * arg1;
	char * arg2;
	
	// inizializzo il game
	gioco.status = STARTED;
	gioco.token = 0;
	ottieniTempo();

	// stampo il menu di gioco
	printHelp();

	// eseguo un loop finche il gioco termina o il giocatore esce
	while(1){
		int i;
		printf("> ");
		fgets(buffer,127,stdin);

		// prima di eseguire il comando controllo il tempo
		// controllo sul tempo
		if (controllaTempo()){
			lose();
			quitRoom();
			break;
		}
		
		// controllo se sono presenti messaggi
		riceviMessaggio();
		
		for (i = 0; i < 128; i++){
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		}
		
		command = strtok(buffer," ");
	
		if (!command){
			; // non compio nessuna azione
		}
		else if (!strcmp(command,"exit")){
			quitRoom();
			break;
		}
		else if (!strcmp(command,"objs")){
			objs();
		}
		else if (!strcmp(command,"use")){
			arg1 = strtok(NULL," ");
			arg2 = strtok(NULL,"");
			use(arg1,arg2);
		}
		else if (!strcmp(command,"take")){
			arg1 = strtok(NULL," ");
			take(arg1);
		}
		else if (!strcmp(command,"look")){
			arg1 = strtok(NULL," ");
			look(arg1);
		}
		else if (!strcmp(command,"time")){
			stampaTempo();
		}
		else if (!strcmp(command,"tell")){
			arg1 = strtok(NULL,"");
			inviaMessaggio(arg1);
		} else if (!strcmp(command,"help")){
			printHelp();
		}
		else 
			printf("\033[A\r\033[K");
		
		// controllo sui token (vinto)
		getToken();
		if (gioco.token == MAX_TOKEN){
			win();
			quitRoom();
			break;
		}
	}
}

// descrizione:
// funzione che stampa una stringa di sconfitta
void lose()
{
	printf("Tempo scaduto, hai perso!\n");
}

// descrizione:
// funzione lanciata dal client per elaborare il comando
// di time e stampare una stringa con il tempo rimanente
void stampaTempo()
{
	time_t current_time;
	int remaining;
	time(&current_time);
	remaining = MINUTES * 60 - difftime(current_time,gioco.start_time);
	if (remaining <= 0)
		printf("Tempo rimanente: 00:00\n");
	printf("Tempo rimanente: %02d:%02d\n",remaining/60,remaining%60);
}

// descrizione:
// funzione lanciata dal client per ottenere il numero aggiornato di token
void getToken()
{
	natb opcode = UPDATE_TOKEN;
	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);
	// ricezione numero di token
	recv(sd,&gioco.token,sizeof(gioco.token),0);
}

// descrizione:
// funzione lanciata dal client per inviare una richiesta di INC_TOKEN al server
void token()
{
	natb opcode = INC_TOKEN;
	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);
}

// descrizione:
// funzione chiamata dal client per ottenere lo start_time
// del gioco
void ottieniTempo()
{
	natb opcode = GET_TIME;
	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);

	// ottengo lo start_time
	recv(sd,&gioco.start_time,sizeof(gioco.start_time),0);
	
	// lo converto nel formato giusto
	gioco.start_time = ntohl(gioco.start_time);
}

#endif
// codice comune

// descrizione:
// funzione che dato il nome di un oggetto ritorna un puntatore all'oggetto
// argomenti:
// c -> stringa contenente il nome dell'oggetto
// ritorno:
// la funzione ritorna un puntatore all'oggetto in caso di sucesso,
// NULL altrimenti
struct oggetto * findOggetto(char * c)
{
	int i;
	if (!c)
		return NULL;
	for (i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(oggetti[i].nome,c)){
			return &oggetti[i];
		}
	}
	return NULL;
}

// descrizione:
// funzione che dato un puntatore ad un oggetto ne ritorna il suo id
// argomenti:
// o -> puntatore ad un oggetto
// ritorno:
// in caso di successo la funzione ritorna l'id dell'oggetto puntato da o,
// NULL altrimenti
natb getObjectId(struct oggetto * o)
{
	natb id = 0;
	for (;id < MAX_OGGETTI;id++){
		if (&oggetti[id] == o)
			return id;
	}
	printf("getObjectId - object not found\n");
	return 0xFF;
}

// descrizione:
// funzione che ritorna un puntatore alla location dato il nome della location
// argomenti:
// c -> stringa contenente il nome della location
// ritorno:
// la funzione ritorna un puntatore alla location in caso di successo,
// NULL altrimenti
struct location * findLocation(char * c)
{
	int i;
	if (!c)
		return NULL;
	for (i = 0; i < MAX_LOCAZIONI; i++){
		if (!strcmp(locazioni[i].nome,c))
			return &locazioni[i];
	}
	return NULL;	
}

// descrizione:
// funzione che dati due oggetti ritorna la ricetta che contiene questi due argomenti
// argomenti:
// o1 -> puntatore al primo oggetto
// o2 -> puntatore al secondo oggetto
// ritorno:
// la funzione ritorna un puntatore alla ricetta se e' presente una ricetta con argomenti
// gli oggetti o1 ed o2 (ATTENZIONE: non conta l'ordine degli oggetti)
struct ricetta * findRicetta(struct oggetto * o1, struct oggetto * o2)
{
	int i;
	if (!o1)
		return NULL;
	for (i = 0; i < MAX_RICETTE; i++){
		// nelle ricette non conta l'ordine degli oggetti usati
		if (	(o1 == ricette[i].oggetto1 && o2 == ricette[i].oggetto2) ||
			(o1 == ricette[i].oggetto2 && o2 == ricette[i].oggetto1)
		   )
			return &ricette[i];
	}
	return NULL;
}

// descrizione:
// funzione per stampare la descrizione della room
void stampaRoom()
{
	char buffer[512];
	sprintf(buffer,"%s\n",stanza.descrizione);
	stampaAnimata(buffer);
}

// descrizione:
// funzione per stampare la descrizione della location l
// argomenti:
// l -> puntatore alla location
void stampaLocation(struct location * l)
{
	char buffer[512];
	sprintf(buffer,"%s\n",l->descrizione);
	stampaAnimata(buffer);
} 

// descrizione:
// funzione per stampare la descrizione dell'oggetto o
// argomenti:
// o -> puntatore all'oggetto
void stampaOggetto(struct oggetto * o)
{
	char buffer[512];
	if (o->status == BLOCCATO){
		// se l'oggetto e' bloccato devo stampare la sua descrizione da bloccato
		sprintf(buffer,"%s\n",o->descrizioneBloccato);
		stampaAnimata(buffer);
	} else {
		// se l'oggetto e' libero devo stampare la sua descrizione
		sprintf(buffer,"%s\n",o->descrizione);
		stampaAnimata(buffer);
	}
} 

// descrizione:
// funzione chiamata dal client per stampare un piccolo messaggio di vittoria
// ed attendere un input da parte del giocatore
void win()
{
	printf("Hai vinto!\n");
	printf("Premi invio per continuare per continuare...\n");
	getchar();
}

// descrizione:
// funzione usata in fase di inizializzazione del game per aggiungere una ricetta
// o1 -> puntatore al primo oggetto
// o2 -> puntatore al secondo oggetto
// dst -> puntatore all'oggetto di destinazione
// action -> azione da compiere
void aggiungiRicetta(struct oggetto * o1, struct oggetto * o2, struct oggetto * dst, char action)
{
	int i;
	for (i = 0; i < MAX_RICETTE; i++){
		if (!ricette[i].oggetto1){ // mi fermo alla prima ricetta libera
			ricette[i].oggetto1 = o1;
			ricette[i].oggetto2 = o2;
			ricette[i].dest = dst;
			ricette[i].action = action;
			return;
		}
	}
}


// descrizione:
// funzione usata per controllare il tempo rimanente nel gioco
// ritorno:
// la funzione ritorna 0 se rimane ancora tempo per giocare, 0 altrimenti
int controllaTempo()
{
	time_t current_time;
	int remaining;
	time(&current_time);
	remaining = MINUTES * 60 - difftime(current_time,gioco.start_time);
	if (remaining <= 0)
		return 1;
	return 0;
}


// per semplicita in questo progetto si ha un solo scenario di gioco inizializzato tramite funzione
// e non caricato da file
// descrizione:
// funzione usata in fase di inizializzazione per inizializzare l'unico scenario di gioco
void init()
{ 
	memset(oggetti,0,sizeof(struct oggetto) * MAX_OGGETTI);
	memset(locazioni,0,sizeof(struct location) *  MAX_LOCAZIONI);
	memset(ricette,0,sizeof(struct ricetta) * MAX_RICETTE);

	// INIZIALIZZAZIONE OGGETTI
	// pistola
	strcpy(oggetti[0].nome, "pistola");
	strcpy(oggetti[0].descrizione, "Una vecchia pistola western.");
	strcpy(oggetti[0].descrizioneBloccato, "La pistola è bloccata da un lucchetto.");
	strcpy(oggetti[0].enigma, "Quante stelle ci sono nel cielo?");
	strcpy(oggetti[0].risposta, "infinite");
	oggetti[0].status = BLOCCATO;

	// cassaforte
	strcpy(oggetti[1].nome, "cassaforte");
	strcpy(oggetti[1].descrizione, "Una robusta cassaforte in metallo.");
	strcpy(oggetti[1].descrizioneBloccato, "La cassaforte è bloccata saldamente.");
	strcpy(oggetti[1].enigma, "Inserisci la combinazione");
	strcpy(oggetti[1].risposta, "3141");
	oggetti[1].status = BLOCCATO;

	// bottiglia
	strcpy(oggetti[2].nome, "bottiglia");
	strcpy(oggetti[2].descrizione, "La bottiglia sembra avere qualcosa al suo interno...");
	strcpy(oggetti[2].descrizioneBloccato, "Una bottiglia apparentemente vuota.");
	strcpy(oggetti[2].enigma, "E' il nome di un famoso alcolico ottenuto da bacche di ginepro, il suo nome ha 3 lettere");
	strcpy(oggetti[2].risposta, "gin");
	oggetti[2].status = BLOCCATO;

	// biglietto
	strcpy(oggetti[3].nome, "biglietto");
	strcpy(oggetti[3].descrizione, "Sul biglietto c'e' scritto: La combinazione della cassaforte sono le prime 4 cifre di pi greco");
	oggetti[3].status = HIDDEN;
	
	// chiave
	strcpy(oggetti[4].nome, "chiave");
	strcpy(oggetti[4].descrizione, "Hai trovato la chiave per uscire dal saloon!");
	oggetti[4].status = HIDDEN;
	


	// INIZIALIZZAZIONE LOCATION
	// bancone
	strcpy(locazioni[0].nome, "bancone");
	strcpy(locazioni[0].descrizione, "Dietro al ++bancone++, puoi vedere qualche bicchiere sporco ed una **bottiglia**.");
	// tavolo
	strcpy(locazioni[1].nome, "tavolo");
	strcpy(locazioni[1].descrizione, "Sul ++tavolo++ c'è una mappa del deserto e qualche sedia rovesciata, sopra al tavolo sembra esserci una piccola **cassaforte**.");

	// pianoforte
	strcpy(locazioni[2].nome, "pianoforte");
	strcpy(locazioni[2].descrizione, "Il ++pianoforte++ sembra essere fuori uso da molto tempo.");

	// mucca
	strcpy(locazioni[3].nome, "mucca");
	strcpy(locazioni[3].descrizione, "Una ++mucca++ solitaria pascola tranquillamente nel locale.");


	// INIZIALIZZAZIONE ROOM
	strcpy(stanza.nome, "saloon");
	strcpy(stanza.descrizione, "Benvenuto nel saloon. Qui puoi trovare il ++bancone++, il ++tavolo++, il ++pianoforte++ e la ++mucca++.");
	
	// INIZIALIZZAZIONE RICETTE
	// se uso la bottiglia posso raccogliere il biglietto
	aggiungiRicetta(&oggetti[2],NULL,&oggetti[3],GIVE);
	// se uso la cassaforte ottengo la chiave
	aggiungiRicetta(&oggetti[1],NULL,&oggetti[4],GIVE);
	// se uso la chiave ottengo un token
	aggiungiRicetta(&oggetti[4],NULL,NULL,TOKEN);

}
