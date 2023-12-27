#include "game.h"
#include <string.h>
#include <stdio.h>

#define OGGETTI 5
#define LOCAZIONI 5

// funzione di prova che inizializza gli oggetti
void init()
{
	memset(oggetti,0,sizeof(oggetto) * MAX_OGGETTI);
	memset(locazioni,0,sizeof(location) *  MAX_LOCAZIONI);
	// inizializzazione oggetti
	for (int i = 0; i < OGGETTI; i++){
		strcpy(oggetti[i].nome,"oggetto");
		strcpy(oggetti[i].descrizione,"descrizione oggetto");
		strcpy(oggetti[i].descrizioneBloccato,"questo oggetto e' ancora bloccato");
		strcpy(oggetti[i].enigma,"enigma oggetto");
		strcpy(oggetti[i].risposta,"risposta");
		oggetti[i].bloccato = 1;
	}

	// inizializzazione location
	for (int i = 0; i < LOCAZIONI; i++){
		strcpy(locazioni[i].nome,"nome location");
		strcpy(locazioni[i].descrizione,"dentro questa stanza si trovano gli oggetti ");
		for (int j = 0; j <  OGGETTI; j++){
			strcat(locazioni[i].descrizione," %s")
			locazioni[i].oggetti[j] = (oggetti + j);
		}
	}

	// inizializzazione della stanza
	strcpy(stanza.nome,"cucina");
	strcpy(stanza.descrizione,"in questa cucina ci sono le location");
	for (int i = 0; i < LOCAZIONI; i++){
		strcat(stanza.descrizione, " %s,");
		stanza.locazioni[i] = locazioni + i;
	}
}

void stampaRoom()
{
	printf(stanza.descrizione,locazioni[0].nome,locazioni[1].nome,locazioni[2].nome,locazioni[3].nome,locazioni[4].nome);
}

void stampaOggetto(struct oggetto * o)
{
	// devo controllare se l'oggetto e' bloccato
	if (o->bloccato){
		printf("%s\n",o->descrizioneBloccato);
	} else {
		printf("%s\n",o->descrizione);
	}
}

void stampaLocation(struct location * l)
{
	printf(l->descrizione,l->nome,l->oggetti[0].nome,l->oggetti[1].nome,l->oggetti[2].nome,l->oggetti[3].nome,l->oggetti[4].nome);
}

// funzione che restituisce un puntatore all'oggetto dato il nome
struct oggetto * findOggetto(char * c)
{
	for (int i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(oggetti[i].nome,c))
			return oggetti[i];
	}
	return NULL;
}

struct ricetta * findRicetta(struct oggetto * o1, struct oggetto * o2)
{
	for (int i = 0; i < MAX_RICETTE; i++){
		// nelle ricette non conta l'ordine degli oggetti usati
		if (	(o1 == ricette[i].oggetto1 && o2 == ricette[i].oggetto2) ||
			(o1 == ricette[i].oggetto2 && o2 == ricette[i].oggetto1)
				)
			return ricette[i];
	}
	return NULL;
}

void look(char * c)
{
	// discriminare output
	char * cursore = c;
	while (*cursore != ' ' && *cursore != '\0')
		cursore++;
	if (*cursore == '\0'){
		// mi trovo alla fine della stringa e quindi devo stampare la room
		stampaRoom();
		return;
	}
	cursore++;

	for (int i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(cursore,oggetti[i].nome)){
			stampaOggetto(oggetti[i]);
			return;
		}
	}

	for (int i = 0; i < MAX_LOCAZIONI; i++){
		if (!strcmp(cursore,locazioni[i].nome)){
			stampaLocation(locazioni[i]);
			return;
		}
	}

	// argomento sbagliato
	printf("bad argument");
}

void sblocca(struct oggetto * o)
{
	char buffer[50];
	printf("%s\n",o->enigma);
	while(1){
		fgets(stdin,63,buffer);
		if (!strcmp(buffer,o->risposta)){ // devo sbloccare l'oggetto
			printf("\r\033[KCorretto!\n");
			o->bloccato = 0;
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
		inventario[i] = o;
	}
	printf("Inventario pieno!\n");
}

// funzione per ottenere l'oggetto
void take(char * c)
{
	char *  cursore = c;
	char buffer[64];
	while (*cursore != ' ' && *cursore != '\0')
		cursore++;

	if (*cursore == '\0')
	{
		printf("look - bad argument\n");
		return;
	}
	cursore++;

	for (int i = 0; i < MAX_OGGETTI; i++){
		if (!strcmp(cursore,oggetti[i].nome)){
			// devo controllare se l'oggetto e' bloccato oppure no
			struct oggetto * o = oggetti[i];
			if (o->bloccato){
				sblocca(o);
			} else { // oggetto sbloccato
				ottieni(o);
			}
			return;
		}
	}
	printf("look - non-existing object\n");
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
void use(char * c)
{
	char * use,obj1,obj2;
	char * buffer;
	struct oggetto * o1, o2;
	struct ricetta * r;
	// faccio una copia del buffer
	strcpy(buffer,c);
	use = strtok(buffer," ");
	obj1 = strtok(NULL," ");
	obj2 = strtok(NULL," ");
	if (!obj1)
	{
		printf("use - need at least 1 arugment\n");
	}
	// ottengo i puntatori agli oggetti
	o1 = findOggetto(obj1);
	if (obj2)
		o2 = findOggetto(obj2);

	// cerco la ricetta
	r = (o1,o2);
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
			r->dest->bloccato = 0;
			printf("Hai sbloccato %s\n",r->dest->nome);
			break;
	}
}

// stampa tutti gli oggetti nell'inventario
void objs()
{
	for (int i = 0; i < INVENTARIO; i++){
		printf("%d: ", i + 1);
		if (inventario[i])
			printf("%s\n",inventario[i].nome);
		else
			printf("vuoto\n");
	}
}

// funzione che si occupa di far runnare il gioco
void game()
{
	char buffer[128];
	char command[32];
	char arg1[32];
	char arg2[32];
	while(1){
		printf("\r\033[K> ");
		fgets(stdin,127,buffer);
		command = strtok(buffer," ");
		arg1 = strtok(NULL," ");
		arg2 = strtok(NULL," ");
		switch (command):
			case (NULL):
				break;
			case ("end"):
				return 0;
			case ("objs"):
				objs();
				break;
			case ("use"):
				use(buffer);
			case ("take"):
				take(buffer);
			case ("look"):
				look(buffer);
			default:
				break;
	}
}





