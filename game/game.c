#include "game.h"
#include <string.h>
#include <stdio.h>

#define OGGETTI 5
#define LOCAZIONI 5

// funzione di prova che inizializza gli oggetti
// da modificare

void init()
{
	memset(oggetti,0,sizeof(struct oggetto) * MAX_OGGETTI);
	memset(locazioni,0,sizeof(struct location) *  MAX_LOCAZIONI);
	// inizializzazione oggetti
	for (int i = 0; i < OGGETTI; i++){
		strcpy(oggetti[i].nome,"oggetto");
		strcpy(oggetti[i].descrizione,"descrizione oggetto");
		strcpy(oggetti[i].descrizioneBloccato,"questo oggetto e' ancora bloccato");
		strcpy(oggetti[i].enigma,"enigma oggetto");
		strcpy(oggetti[i].risposta,"risposta");
		oggetti[i].status = BLOCCATO;
	}

	// inizializzazione location
	for (int i = 0; i < LOCAZIONI; i++){
		strcpy(locazioni[i].nome,"cucina");
		strcpy(locazioni[i].descrizione,"dentro questa location (%s) si trovano gli oggetti");
		for (int j = 0; j <  OGGETTI; j++){
			strcat(locazioni[i].descrizione," %s");
			locazioni[i].oggetti[j] = (oggetti + j);
		}
	}

	// inizializzazione della stanza
	strcpy(stanza.nome,"casa");
	strcpy(stanza.descrizione,"in questa room (%s) ci sono le location");
	for (int i = 0; i < LOCAZIONI; i++){
		strcat(stanza.descrizione, " %s");
		stanza.locazioni[i] = locazioni + i;
	}
}

void stampaRoom()
{
	printf(stanza.descrizione,stanza.nome,locazioni[0].nome,locazioni[1].nome,locazioni[2].nome,locazioni[3].nome,locazioni[4].nome);
	printf("\n");
}

void stampaOggetto(struct oggetto * o)
{
	// devo controllare se l'oggetto e' bloccato
	if (o->status == BLOCCATO){
		printf("%s\n",o->descrizioneBloccato);
	} else {
		printf("%s\n",o->descrizione);
	}
}

void stampaLocation(struct location * l)
{
	printf(l->descrizione,l->nome,l->oggetti[0]->nome,l->oggetti[1]->nome,l->oggetti[2]->nome,l->oggetti[3]->nome,l->oggetti[4]->nome);
	printf("\n");
}

// funzione che restituisce un puntatore all'oggetto dato il nome
struct oggetto * findOggetto(char * c)
{
	if (!c)
		return NULL;
	for (int i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(oggetti[i].nome,c))
			return &oggetti[i];
	}
	return NULL;
}

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

void look(char * c)
{
	// se non ci sono argomenti specificati devo stampare
	// la descrizione della room
	if (!c){
		stampaRoom();
		return;
	}

	// find oggetto e stampa oggetto
	struct oggetto * o = findOggetto(c);
	if (o){
		stampaOggetto(o);
		return;
	}

	// find location e stampa location
	struct location * l = findLocation(c);
	if (l){
		stampaLocation(l);
		return;
	}

	// argomento sbagliato
	printf("look - bad argument\n");
}

void sblocca(struct oggetto * o)
{
	char buffer[50];
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
			return;
		} else if (!strcmp(buffer,"exit")){ // il player non vuole piu indovinare per ora
			return;
		} else { // errore (aggiungere tentativi--)
			printf("\r\033[KSbagliato\n");
		}
	}
}

// funzione per inserire l'oggetto all'interno dell'inventario
void ottieni(struct oggetto * o)
{
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i])
			continue;
		giocatore.inventario[i] = o;
		o->status = TAKEN;
		return;
	}
	printf("Inventario pieno!\n");
}

// funzione per ottenere l'oggetto
void take(char * c)
{
	if (!c)
	{
		printf("look - bad argument\n");
		return;
	}
	
	// provo ad ottenere l'oggetto
	struct oggetto * o = findOggetto(c);
	if (!o){ // oggetto non trovato
		printf("look - non-existing object\n");
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
void rimuoviInventario(struct oggetto * o)
{
	for (int i = 0; i < INVENTARIO; i++){
		if (giocatore.inventario[i] == o)
			giocatore.inventario[i] = NULL;
	}
}

// funzione di utilita per aggiungere un oggetto all'inventario
size_t aggiungiInventario(struct oggetto * o)
{
	for (int i = 0; i < INVENTARIO; i++){
		if (!giocatore.inventario[i]){
			giocatore.inventario[i] = 0;
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
	o1 = findOggetto(obj1);
	if (obj2)
		o2 = findOggetto(obj2);
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
			printf("Hai ottenuto %s\n",r->dest->nome);
			break;
		case (UNLOCK):
			r->dest->status = FREE;
			printf("Hai sbloccato %s\n",r->dest->nome);
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

// funzione che si occupa di far runnare il gioco
void game()
{
	char buffer[128];
	char * command;
	char * arg1;
	char * arg2;
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
			continue;
	}
}





