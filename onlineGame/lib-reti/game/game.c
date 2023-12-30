#include "game.h"
#include "../menu.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct room stanza;	// struttura globale room
struct game gioco;	// struttura globale oggetti
struct oggetto oggetti[MAX_OGGETTI];
struct location locazioni[MAX_LOCAZIONI];
struct ricetta ricette[MAX_RICETTE];

#ifdef SERVER
// codice del server
int indice = 0;

struct player giocatori[MAX_PLAYERS];

void addPlayer(char * username, int sd)
{
	if (indice >= MAX_PLAYERS)
		return;
	strcpy(giocatori[indice].username,username);
	giocatori[indice].sd = sd;
}

// funzione chiamata dal server per rispondere alla richiesta di aggiornamento dell'oggetto
void aggiornaOggetto(int sd)
{
	natb id = 0;
	int ret = 0;
	struct oggetto * o;
	// ricevo id dell'oggetto
	ret = recv(sd,&id,sizeof(id),0);
	o = &oggetti[id];
	// invia lo status aggiornato dell'oggetto
	ret = send(sd,&o->status,sizeof(o->status), 0);
}

// funzione chiamata dal server per sbloccare l'oggetto o
void sbloccaOggetto(int sd)
{
	natb id = 0;
	struct oggetto * o = NULL;
	int ret;
	// ricevo id
	ret = recv(sd,&id,sizeof(id),0);
	// sblocco l'oggetto
	o = &oggetti[id];
	o->status = FREE;
}

// funzione chiamata dal server per ottenere la modifica di
// un oggetto
void ottieniOggetto(int sd)
{ 
	natb id = 0;
	// devo ottenere l'indice del player
	natb pid = getPlayerId(sd);

	int ret = recv(sd,&id,sizeof(id),0);

	struct oggetto * o = &oggetti[id];

	// devo metterlo nel suo inventario
	aggiungiInventario(pid,o);
	// devo cambiare lo status dell'oggetto
	// in TAKEN
	o->status = TAKEN;
}

// funzione che ritorna l'indice del player dato il suo
// socket descriptor
natb getPlayerId(int sd)
{
	natb id = 0;
	for (;id < MAX_PLAYERS;id++)
		if (giocatori[id].sd == sd)
			return id;

	printf("getPlayerId - player not found\n");
	return 0xFF;
}

// trova nell'inventario del player id l'oggetto di nome c
struct oggetto * findOggettoInventario(natl id,char * c)
{
	if (!c)
		return NULL;
	for (int i = 0; i < INVENTARIO; i++){
		if (!giocatori[id].inventario[i])
			continue;
		if (!strcmp(giocatori[id].inventario[i]->nome,c)){
			return giocatori[id].inventario[i];
		}
	}
	return NULL;
}

// rimuove l'oggetto o dall'inventario del giocatore id
void rimuoviInventario(natb id,struct oggetto * o)
{
	if (!o)
		return;
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatori[id].inventario[i] == o)
			giocatori[id].inventario[i] = NULL;
	}
 }

// aggiunge l'oggetto o all'inventario del giocatore id
size_t aggiungiInventario(natb id,struct oggetto * o)
{
	if (!o)
		return 0;

	for (int i = 0; i < INVENTARIO; i++){
		if (!giocatori[id].inventario[i]){
			giocatori[id].inventario[i] = o;
			return 0;
		}
	}
	return 1;
} 

void usaOggetto(int sd)
{
	natb id1, id2 = 0xFF, pid;
	int ret;
	struct ricetta * r = NULL;
	struct oggetto * o1, * o2 = NULL;
	// ricevo gli oggetti
	ret = recv(sd,&id1,sizeof(id1),0);
	ret = recv(sd,&id2,sizeof(id2),0);
	
	o1 = &oggetti[id1];
	if (o2 != 0xFF)
		o2 = &oggetti[id2];

	// ottengo il player id
	pid = getPlayerId(sd);
	
	// supponiamo che la use abbia funzionato correttamente
	r = findRicetta(o1,o2);
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

// stampa tutti gli oggetti nell'inventario del giocatore
void objs(int sd)
{
	
	natb id = getPlayerId(sd);

	for (int i = 0; i < INVENTARIO; i++){
		printf("%d: ", i + 1);
		if (giocatori[id].inventario[i])
			printf("%s\n",giocatori[id].inventario[i]->nome);
		else
			printf("vuoto\n");
	}
}

// funzione che elabora le richieste dei client
void game(int sd, natb opcode)
{
	switch (opcode){
		case (UPDATE_OBJECT):
			printf("%d ha fatto aggiornaOggetto()\n",sd);
			aggiornaOggetto(sd);
			break;
		case (UNLOCK):
			printf("%d ha fatto sbloccaOggetto()\n",sd);
			sbloccaOggetto(sd);
			break;
		case (TAKE):
			printf("%d ha fatto ottieniOggetto()\n",sd);
			ottieniOggetto(sd);
			break;
		case (USE):
			printf("%d ha fatto usaOggetto()\n",sd);
			usaOggetto(sd);
			break;
		case (UPDATE_TOKEN):
			printf("%d ha fatto getToken()\n",sd);
			getToken(sd);
			break;
		case (INC_TOKEN):
			printf("%d ha fatto token()",sd);
			token();
			break;
	}	
}

// funzione per sbloccare i player per iniziare a giocare
void sbloccaPlayers()
{
	natb opcode = 250;
	int ret;
	for (int i = 0; i < MAX_PLAYERS; i++)
		ret = send(giocatori[i].sd,&opcode,sizeof(opcode),0);
}

// funzione lanciata dal server per inviare il numero
// di token attuali
void getToken(int sd)
{
	int ret = send(sd,&gioco.token,sizeof(gioco.token),0);
}

// funzione lanciata dal server per incrementare il numero
// di token
void token()
{
	gioco.token++;
}

#else
// codice del client

int sd;			// descrittore del server

void initsd(int sockd)
{
	sd = sockd;
}

struct player giocatore;

// funzione chiamata dal client per richiedere di aggiornare l'oggetto
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

// funzione che notifica il server che l'oggetto o e' stato 
// sbloccato
void sbloccaOggetto(struct oggetto * o)
{
	natb id = getObjectId(o);
	natb opcode = UNLOCK;
	int ret;

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);
	// invio id
	ret = send(sd,&id,sizeof(id),0);
}

// notifica al server di aver preso l'oggetto di id id
void ottieniOggetto(struct oggetto * o)
{
	natb id = getObjectId(o);
	natb opcode = TAKE;
	int ret;

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);
	// invio id
	ret = send(sd,&id,sizeof(id),0);
}

// trova nell'inventario del player l'oggetto di nome c
struct oggetto * findOggettoInventario(char * c)
{
	if (!c)
		return NULL;
	for (int i = 0; i < INVENTARIO; i++){
		if (!giocatore.inventario[i])
			continue;
		if (!strcmp(giocatore.inventario[i]->nome,c)){
			return giocatore.inventario[i];
		}
	}
	return NULL;
}

void look(char * c)
{
	struct location * l;
	struct oggetto * o;

	// look da sola stampa la descrizione della room
	if (!c){
		stampaRoom();
		return;
	}

	// find location
	
	l = findLocation(c);
	if (l){
		stampaLocation(l);
		return;
	}

	// se l'oggetto e' nell'inventario non ho bisogno di aggiornare niente
	o = findOggettoInventario(c);
	if (o){
		stampaOggetto(o);
		return;
	}

	// devo cercare l'oggetto fra tutti gli oggetti
	o = findOggetto(c);
	if (o)
		aggiornaOggetto(o);

	// se l'oggetto e' HIDDEN l'oggetto non viene trovato
	if (o->status == HIDDEN)
		o = NULL;

	if (o){
		stampaOggetto(o);
		return;
	}

	printf("look - bad argument\n");

}

void take(char *c)
{
	if (!c){
		printf("take - bad argument\n");
		return;
	}

	// provo ad ottenere l'oggetto
	struct oggetto * o = findOggetto(c);
	if (!o){
		printf("take - non-existing object\n");
		return;
	}
	aggiornaOggetto(o);
	
	if (o->status == HIDDEN)
		o = NULL;
	
	if (!o){ // oggetto non trovato
		printf("take - non-existing object\n");
		return;
	}

	// se l'oggetto e' gia' stato preso non posso 
	// prenderlo
	if (o->status == TAKEN)
	{
		printf("L'oggetto e' gia stato preso\n");
		return;
	}
	
	// se l'oggetto e' bloccato provo a sbloccarlo
	if (o->status == BLOCCATO){
		sblocca(o);
		printf("Hai sbloccato %s\n",o->nome);
	} else {
		ottieni(o);
		printf("Hai ottenuto %s\n",o->nome);
	}

}

void sblocca(struct oggetto * o)
{
	char buffer[64];
	printf("%s\n",o->enigma);
	while(1){
		fgets(buffer,63,stdin);
		for (int i = 0; i < 63; i++){
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
			return;
		} else if (!strcmp(buffer,"exit")){ // il player non vuole piu indovinare per ora
			return;
		} else { // errore (aggiungere tentativi--)
			printf("\r\033[KSbagliato\n");
		}
	}
}

void ottieni(struct oggetto * o)
{
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i])
			continue;
		giocatore.inventario[i] = o;
		o->status = TAKEN;
		ottieniOggetto(o);
		return;
	}
	printf("Inventario pieno!\n");
}

void rimuoviInventario(struct oggetto * o)
{
	if (!o)
		return;
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i] == o)
			giocatore.inventario[i] = NULL;
	}
}

size_t aggiungiInventario(struct oggetto * o)
{
	if (!o)
		return 0;

	for (int i = 0; i < INVENTARIO; i++){
		if (!giocatore.inventario[i]){
			giocatore.inventario[i] = o;
			return 0;
		}
	}
	return 1;
} 

void usaOggetto(struct oggetto * o1, struct oggetto * o2)
{
	natb id1, id2 = 0xFF, opcode = TAKE;
	int ret;
	id1 = getObjectId(o1);
	if (o2)
		id2 = getObjectId(o2);

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);

	// invio oggetti
	ret = send(sd,&id1,sizeof(id1),0);
	ret = send(sd,&id2,sizeof(id2),0);
}

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

	// a questo punto sono sicuro che la ricetta esista
	
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

// stampa tutti gli oggetti nell'inventario
void objs()
{
	for (int i = 0; i < INVENTARIO; i++){
		printf("%d: ", i + 1);
		if (giocatore.inventario[i])
			printf("%s\n",giocatore.inventario[i]->nome);
		else
			printf("vuoto\n");
	}
}

void startRoomID(char * room)
{
	int rid, ret;
	natb opcode = START_ROOM;
	sscanf(room,"%d",&rid);
	rid = htonl(rid);
	
	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);

	// invio room id
	ret = send(sd,&rid,sizeof(rid),0);
}

void quitRoom()
{
	natb opcode = QUIT_ROOM;
	int ret = send(sd,&opcode,sizeof(opcode),0);
	
	// aspetto un messaggio di ok
	ret = recv(sd,&opcode,sizeof(opcode),0);
	printHome();
}

// funzione lanciata dal client per gestire il gioco
void game()
{
	char buffer[128];
	char * command;
	char * arg1;
	char * arg2;
	gioco.status = STARTED;
	gioco.token = 0;
	while(1){
		printf("> ");
		getToken();
		
		if (gioco.token == MAX_TOKEN){
			win();
			quitRoom();
			break;
		}

		fgets(buffer,127,stdin);

		for (int i =0; i < 128; i++){
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		}
		command = strtok(buffer," ");
		arg1 = strtok(NULL," ");
		arg2 = strtok(NULL," ");
		if (!command)
			continue;
		else if (!strcmp(command,"end")){
			quitRoom();
			break;
		}
		else if (!strcmp(command,"objs")){
			objs();
		}
		else if (!strcmp(command,"use")){
			use(arg1,arg2);
		}
		else if (!strcmp(command,"take")){
			take(arg1);
		}
		else if (!strcmp(command,"look")){
			look(arg1);
		}
		else if (!strcmp(command,"start")){
			startRoomID(arg1);
		}
		else
			;
	}
}

// funzione lanciata dal client per ottenere il numero aggiornato di token
void getToken()
{
	natb opcode = UPDATE_TOKEN;
	int ret = send(sd,&opcode,sizeof(opcode),0);

	ret = recv(sd,&gioco.token,sizeof(gioco.token),0);
}

// funzione lanciata dal client per incrementare il numero di token del game
void token()
{
	natb opcode = INC_TOKEN;
	int ret = send(sd,&opcode,sizeof(opcode),0);
}

#endif

// ritorna l'oggetto di nome c
struct oggetto * findOggetto(char * c)
{
	if (!c)
		return NULL;
	for (int i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(oggetti[i].nome,c)){
			return &oggetti[i];
		}
	}
	return NULL;
}

// ritorna l'id dell'oggetto puntato da o
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

// trova la location
struct location * findLocation(char * c)
{
	if (!c)
		return NULL;
	for (int i = 0; i < MAX_LOCAZIONI; i++){
		if (!strcmp(locazioni[i].nome,c))
			return &locazioni[i];
	}
	return NULL;	
}

// trova la ricetta dati gli oggetti
struct ricetta * findRicetta(struct oggetto * o1, struct oggetto * o2)
{
	if (!o1)
		return NULL;
	for (int i = 0; i < MAX_RICETTE; i++){
		// nelle ricette non conta l'ordine degli oggetti usati
		if (	(o1 == ricette[i].oggetto1 && o2 == ricette[i].oggetto2) ||
			(o1 == ricette[i].oggetto2 && o2 == ricette[i].oggetto1)
		   )
			return &ricette[i];
	}
	return NULL;
}

// funzione per stampare la descrizione della room
void stampaRoom()
{
	printf("%s\n",stanza.descrizione);
}

// funzione per stampare la descrizione della location l
void stampaLocation(struct location * l)
{
	printf("%s\n",l->descrizione);
}

// funzione per stampare la descrizione dell'oggetto o
void stampaOggetto(struct oggetto * o)
{
	// devo controllare se l'oggetto e' bloccato
	if (o->status == BLOCCATO){
		printf("%s\n",o->descrizioneBloccato);
	} else {
		printf("%s\n",o->descrizione);
	}
}

void win()
{
	printf("Hai vinto!\n");
}

// funzione per aggiungere una ricetta
void aggiungiRicetta(struct oggetto * o1, struct oggetto * o2, struct oggetto * dst, char action)
{
	for (int i = 0; i < MAX_RICETTE; i++){
		if (!ricette[i].oggetto1){ // mi fermo alla prima ricetta libera
			ricette[i].oggetto1 = o1;
			ricette[i].oggetto2 = o2;
			ricette[i].dest = dst;
			ricette[i].action = action;
			return;
		}
	}
}


// funzione per inizializzare la prima room
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
	strcpy(oggetti[1].enigma, "Combinazione");
	strcpy(oggetti[1].risposta, "3141");
	oggetti[1].status = BLOCCATO;

	// bottiglia
	strcpy(oggetti[2].nome, "bottiglia");
	strcpy(oggetti[2].descrizione, "La bottiglia sembra avere qualcosa al suo interno...");
	strcpy(oggetti[2].descrizioneBloccato, "Una bottiglia apparentemente vuota.");
	strcpy(oggetti[2].enigma, "Un alcolico con 3 lettere");
	strcpy(oggetti[2].risposta, "gin");
	oggetti[2].status = BLOCCATO;

	// biglietto
	strcpy(oggetti[3].nome, "biglietto");
	strcpy(oggetti[3].descrizione, "Sul biglietto c'e' scritto: La combinazione della cassaforte sono le prime 4 cifre di pi");
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
	aggiungiRicetta(&oggetti[1],NULL,&oggetti[4],GIVE);
	aggiungiRicetta(&oggetti[4],NULL,NULL,TOKEN);
}