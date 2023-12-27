#include <time.h>
#define MAX_LOCAZIONI 5
#define MAX_OGGETTI 5
#define MAX_RICETTE 10

#define INVENTARIO 5


#define GIVE 0x01
#define UNLOCK 0x02



// per il momento assumiamo che se l'oggetto e' bloccato
// viene stampata la stessa frase per tutti:
// "oggetto bloccato..."

struct oggetto
{
	char nome[64];
	char descrizione[128];
	char descrizioneBloccato[128];
	char enigma[128];
	char risposta[64];
	char bloccato;
};


struct location
{
	char nome[64];
	char descrizione[128];
	struct oggetto * oggetti[MAX_OGGETTI];
};


struct room
{
	char nome[64];
	char descrizione[128];
	struct location * locazioni[MAX_LOCAZIONI];
} stanza;
// variabile globale per la stanza

struct player
{
	struct oggetto * inventario[INVENTARIO];
} giocatore;

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
};



// strutture che contengono gli oggetti (globale)
struct oggetto oggetti[MAX_OGGETTI];

// strutture che contengono le locazioni (globale)
struct location locazioni[MAX_LOCAZIONI];

// struttura che contiene le ricette (globale)
struct ricetta ricette[MAX_RICETTE];


