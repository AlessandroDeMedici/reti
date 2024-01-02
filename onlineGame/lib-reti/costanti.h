#ifndef COSTANTI_INCLUSA
#define COSTANTI_INCLUSA
#include "netinet/in.h"
// costanti
#define MAX_PLAYERS 2		// numero massimo di giocatori per room
#define MAX_TENTATIVI 3		// numero massimo di tentativi per fare login
#define MINUTES 10		// numero di minuti entro i quali finire il gioco


// status del giocatore
#define NOT_LOGGED 0x00		// satus del giocatore che non ha fatto login (gestito dal main)
#define LOGGED 0x01		// status del giocatore che ha fatto login (gestito dal main)
#define PLAYING 0x02		// status del giocatore che sta giocando (gestito dalla room)	

// status della room
#define CREATED 0x00		// status della room creata, i giocatori stanno joinando
#define STARTED 0x01		// status della room in cui i giocatori stanno giocando e non e' piu possibile entrare
#define QUITTING 0x01		// status della room terminata, i giocatori stanno uscendo

// opcodes (messaggi) - home
#define LOGIN 0x01		// messaggio di login
#define REGISTER 0x00		// messaggio di register
#define LOGOUT 0x02		// messaggio di logout
#define START_ROOM 0xFF		// messaggio di start room
#define ROOM_LIST 0xBF		// messaggio di richiesta della lista delle rooms
#define OK 0xFA			// messaggio di ok
#define NOK 0xAF		// messaggio di non ok

// opcodes (messaggi) - room
#define QUIT_ROOM 0x08		// messaggio di uscita dalla room
#define UPDATE_OBJECT 0x01	// messaggio di update dello stato di un oggetto
#define UNLOCK 0x02		// messaggio di sblocco di un oggetto
#define TAKE 0x03		// messaggio di take di un oggetto
#define USE 0x04		// messaggio di use di uno o due oggetti
#define UPDATE_TOKEN 0x05	// messaggio di update del numero dei token
#define INC_TOKEN 0x06		// messaggio di incremento del numero di token
#define GET_TIME 0x07		// messaggio per ottenere lo start time del gioco
#define INVIA_MESSAGGIO 0x0A	// messaggio per invaire il messaggio
#define RICEVI_MESSAGGIO 0x0B	// messaggio per ricevere un messaggio

typedef uint8_t natb;
typedef uint16_t natw;
typedef uint32_t natl;
typedef uint64_t natq;


#endif




