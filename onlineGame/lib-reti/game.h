#ifndef GAME_INCLUSA
#define GAME_INCLUSA
#include <time.h>
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

// definizione degli opcodes
#define UPDATE_OBJECT 0x01
#define UNLOCK 0x02
#define TAKE 0x03
#define USE


// per il momento assumiamo che se l'oggetto e' bloccato
// viene stampata la stessa frase per tutti:
// "oggetto bloccato..."

struct oggetto
{
	char nome[64];
	char descrizione[256];
	char descrizioneBloccato[256];
	char enigma[256];
	char risposta[64];
	char status;
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
// variabile globale per la stanza

struct player
{
	int sd;
	char username[50];
	struct oggetto * inventario[INVENTARIO];
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
	int token;
};


void init();
void stampaRoom();
void stampaOggetto(struct oggetto *);
void stampaLocation(struct location *);
struct oggetto * findOggetto(char *);
struct location * findLocation(char *);
struct ricetta * findRicetta(struct oggetto*,struct oggetto *);
void look(char *);
void sblocca(struct oggetto*);
void ottieni(struct oggetto*);
void take(char *);
void rimuoviInventario(struct oggetto *);
size_t aggiungiInventario(struct oggetto *);
void use(char *,char*);
void objs();
void game();

#endif
