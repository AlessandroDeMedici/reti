#ifndef GAME_INCLUSA
#define GAME_INCLUSA
#include <time.h>
#include "../costanti.h"

#define MAX_LOCAZIONI 5
#define MAX_OGGETTI 10
#define MAX_RICETTE 10
#define MAX_TOKEN 1		// numero di token per vincere


#define INVENTARIO 5

#define FREE 0x00		// oggetto raccoglibile
#define BLOCCATO 0x01		// oggetto bloccato da enigma
#define TAKEN 0x02		// oggetto nell'inventario
#define HIDDEN 0x03		// oggetto nascosto


#define GIVE 0x01		// oggetto HIDDEN -> TAKEN
#define UNLOCK 0x02		// oggetto HIDDEN -> FREE
#define TOKEN 0x03		// oggetto TOKEN

#define STARTED 0x01
#define STOPPED 0x02


struct oggetto
{
	char nome[64];
	char descrizione[256];
	char descrizioneBloccato[256];
	char enigma[256];
	char risposta[64];
	natb status;
};

struct location
{
	char nome[64];
	char descrizione[256];
	struct oggetto * oggetti[MAX_OGGETTI];
};

struct room
{
	char nome[64];
	char descrizione[256];
	struct location * locazioni[MAX_LOCAZIONI];
};

struct player
{
	int sd;
	char username[50];
	struct oggetto * inventario[INVENTARIO];
	char p;
};

struct ricetta
{
	struct oggetto * oggetto1;	// primo oggetto usato
	struct oggetto * oggetto2;	// secondo oggetto usato
	struct oggetto * dest;		// oggetto destinatario
	char action;			// azione da compiere sull'oggetto destinatario
};

struct game
{
	char status;
	natb token;
	time_t start_time;
};


#ifdef SERVER
// metodi del server
void addPlayer(char * username, int sd);
void aggiornaOggetto(int sd);
void sbloccaOggetto(int sd);
void ottieniOggetto(int sd);
natb getPlayerId(int sd);
struct oggetto * findOggettoInventario(natl id,char * c);
void rimuoviInventario(natb id,struct oggetto * o);
size_t aggiungiInventario(natb id,struct oggetto * o);
void usaOggetto(int sd);
void objs(int sd);
void game(int sd, natb opcode);
void sbloccaPlayers();
void getToken();
void quitRoom();
void startTime();
void ottieniTempo();
#else
// metodi del client
void initsd(int sockd);
struct player giocatore;
void aggiornaOggetto(struct oggetto * o);
void sbloccaOggetto(struct oggetto * o);
void ottieniOggetto(struct oggetto * o);
struct oggetto * findOggettoInventario(char * c);
void look(char * c);
void take(char *c);
void sblocca(struct oggetto * o);
void ottieni(struct oggetto * o);
void rimuoviInventario(struct oggetto * o);
size_t aggiungiInventario(struct oggetto * o);
void usaOggetto(struct oggetto * o1, struct oggetto * o2);
void use(char * obj1, char * obj2);
void objs();
void startRoomID(char * room);
void game();
void getToken();
void token();
void quitRoom();
void ottieniTempo();
#endif

// metodi comuni
struct oggetto * findOggetto(char * c);
natb getObjectId(struct oggetto * o);
struct location * findLocation(char * c);
struct ricetta * findRicetta(struct oggetto * o1, struct oggetto * o2);
void win();
void stampaRoom();
void stampaLocation(struct location * l);
void stampaOggetto(struct oggetto * o);
void aggiungiRicetta(struct oggetto * o1, struct oggetto * o2, struct oggetto * dst, char action);
void init();
void token();
#endif
