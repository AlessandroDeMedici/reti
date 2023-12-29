#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "costanti.h"

// di ogni funzione che si ritiene che il suo
// risultato possa evolvere nel corso della
// partita si ha una versione per il client
// ed una versione per il server
//
//
// il resto delle funzioni come
// stampaRoom -> stampa la descrizione della room
// stampaLocation -> stampa la descrizione della location
// sono presenti in unica versione

// descrittore del server
int sd;

// struttura globale per la room
struct room stanza;

// struttura globale per il giocatore
struct player giocatore;

// struttura globale per il gioco
struct game gioco;

// strutture che contengono gli oggetti (globale)
struct oggetto oggetti[MAX_OGGETTI];

// strutture che contengono le locazioni (globale)
struct location locazioni[MAX_LOCAZIONI];

// struttura che contiene le ricette (globale)
struct ricetta ricette[MAX_RICETTE];

// struttura che contiene i giocatori (globale server)
struct player giocatori[MAX_PLAYERS];

// struttura che contiene il giocatore (globale client)
struct player giocatore;

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


// funzione di utilita per ottenere l'id dell'oggetto
// dato un puntatore all'oggetto
natb getId(struct oggetto * o)
{
	natb id = 0;
	for (;id < MAX_OGGETTI;id++){
		if (&oggetti[id] == o)
			return id;
	}
	printf("getId - object not found\n");
	return 0xFF;
}

// funzione chiamata dal client per richiedere di aggiornare l'oggetto
void aggiornaOggetto(struct oggetto * o)
{
	int ret;
	natb status = 0;
	// si richiede lo status dell'oggetto
	natb opcode = UPDATE_OBJECT;
	natb id = getId(o);
	// invio opcode
	send(sd,&opcode,sizeof(opcode),0);
	// invio id dell'oggetto
	send(sd,&id,sizeof(id),0);
	// ricezione dello status
	ret = recv(sd,&o->status,sizeof(status),0);
	if (!ret) // disconnessione
		printf("aggiornaOggetto - server disconnesso\n");
} 

// funzione chiamata dal server per inviare lo status dell'oggetto al client
void aggiornaOggettoServer(int sd)
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

// funzione che sblocca l'oggetto
void sbloccaOggetto(struct oggetto * o)
{
	natb id = getId(o);
	natb opcode = UNLOCK;

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode),0);
	// invio id
	ret = send(sd,&od,sizeof(id),0);
}

void sbloccaOggettoServer(int sd)
{
	natb id = 0;
	struct oggetto * o = NULL;
	int ret;
	// ricevo id
	ret = recv(sd,&id,sizeof(id),0);
	// sblocco l'oggetto
	o = &oggetti[id];
	o->STATUS = FREE;
}

// funzione che sblocca l'oggetto
void ottieniOggetto(struct oggetto * o)
{
	natb id = getId(o);
	natb opcode = TAKE;

	// invio opcode
	ret = send(sd,&opcode,sizeof(opcode,0));
	// invio id
	ret = send(sd,&od,sizeof(id),0);
}

// funzione di utilita che ritorna l'indice
// del player dato il suo socket descriptor
natb getPlayerId(int sd)
{
	natb id = 0;
	for (;id < MAX_PLAYERS;id++)
		if (giocatori[id].sd == sd)
			return id;

	printf("getPlayerId - player not found\n");
	return 0xFF;
}

void ottieniOggettoServer(int sd)
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

// trova nell'inventario l'oggetto di nome c
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


// per rispondere alla look il server deve soltanto inviare lo stato aggiornato dell'oggetto al client
void look(char * c)
{
	// se non ci sono argomenti specificati devo stampare
	// la descrizione della room
	if (!c){
		stampaRoom();
		return;
	}
	
	// find location e stampa location
	struct location * l = findLocation(c);
	if (l){
		stampaLocation(l);
		return;
	}
	
	// find oggetto e stampa oggetto
	struct oggetto * o = findOggetto(c);
	aggiornaOggetto(o);
	
	// se HIDDEN non viene trovato
	if (o->status == HIDDEN)
		o = NULL:
	if (o){
		stampaOggetto(o);
		return;
	}
	// argomento sbagliato
	printf("look - bad argument\n");
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

// funzione per inserire l'oggetto all'interno dell'inventario

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

// funzione che inizializza la room e le sue strutture
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

void stampaRoom()
{
	printf("%s\n",stanza.descrizione);
}

void stampaLocation(struct location * l)
{
	printf("%s\n",l->descrizione);
}

// funzione che restituisce un puntatore all'oggetto dato il nome
void stampaOggetto(struct oggetto * o)
{
	// devo aggiornare lo status dell'oggetto
	aggiornaOggetto(o);
	// devo controllare se l'oggetto e' bloccato
	if (o->status == BLOCCATO){
		printf("%s\n",o->descrizioneBloccato);
	} else {
		printf("%s\n",o->descrizione);
	}
}

// acquisisce l'oggetto
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

// funzione per ottenere l'oggetto
void take(char * c)
{
	if (!c)
	{
		printf("take - bad argument\n");
		return;
	}
	
	// provo ad ottenere l'oggetto
	struct oggetto * o = findOggetto(c);
	aggiornaOggetto(o);
	if (o->HIDDEN)
		o = NULL;
	if (!o){ // oggetto non trovato
		printf("take - non-existing object\n");
		return;
	}
	// se l'oggetto e' gia' stato preso non posso prenderlo
	// di nuovo
	if (o->status == TAKEN)
	{
		printf("L'oggetto e' gia stato preso\n");
		return;
	}
	// se l'oggetto e' bloccato provo a sbloccarlo
	if (o->status == BLOCCATO)
		sblocca(o);
	// se l'oggetto non e' bloccato lo ottengo
	else
		ottieni(o);
}

// funzione di utilita per rimuovere un oggetto dall'inventario
// MODIFICARE LA FUNZIONE IN MODO CHE POSSA
// FUNZIONARE ANCHE PER IL SERVER CHE GESTISCE
// UN ARRAY DI GIOCATORI
void rimuoviInventario(struct oggetto * o)
{
	if (!o)
		return;
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i] == o)
			giocatore.inventario[i] = NULL;
	}
}

// funzione di utilita per aggiungere un oggetto all'inventario
// MODIFICARE QUESTA FUNZIONE AGGIUNGENDO 
// L'INVENTARIO DEL PLAYER CORRETTO
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

// funzione per usare l'oggetto o gli oggetti
void use(char * obj1, char * obj2)
{
	struct oggetto * o1 = 0,* o2 = 0;
	struct ricetta * r;
	if (!obj1)
	{
		printf("use - need at least 1 arugment\n");
		return;
	}
	// ottengo i puntatori agli oggetti
	o1 = findOggettoInventario(obj1);

	if (!o1){
		printf("use - oject not in inventory\n");
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

	// una volta trovata la ricetta devo compiere l'azione specificata
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

void win(){
	printf("Hai vinto!\n");
}

// funzione che si occupa di far runnare il gioco
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
		else if (!strcmp(command,"end"))
			break;
		else if (!strcmp(command,"objs")){
			objs();
		}
		else if (strstr(command,"use")){
			use(arg1,arg2);
		}
		else if (strstr(command,"take")){
			take(arg1);
		}
		else if (strstr(command,"look")){
			look(arg1);
		}
		else
			;
		if (gioco.token == MAX_TOKEN){
			win();
			break;
		}
	}
}

// funzione che elabora l'input dell'utente
void elab(char * stringa)
{
	char comando[50];
	char arg1[50];
	char arg2[50];
	char risposta[256];
	command = strtok(buffer," ");
	arg1 = strtok(NULL," ");
	arg2 = strtok(NULL," ");
	if (!command)
		continue;
	else if (!strcmp(command,"end"))
		break;
	else if (!strcmp(command,"objs")){
		objs();
	}
	else if (strstr(command,"use")){
		use(arg1,arg2);
	}
	else if (strstr(command,"take")){
		take(arg1);
	}
	else if (strstr(command,"look")){
		look(arg1);
	}
	else
		;
	if (gioco.token == MAX_TOKEN){
		win();
		break;
	}
}


