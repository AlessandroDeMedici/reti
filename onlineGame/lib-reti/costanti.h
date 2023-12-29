#ifndef COSTANTI_INCLUSA
#define COSTANTI_INCLUSA

#include <netinet/in.h>

// costanti
#define MAX_PLAYERS 3		// numero massimo di giocatori per room
#define MAX_TENTATIVI 3		// numero massimo di tentativi per fare login

// status del giocatore
#define NOT_LOGGED 0x00		// satus del giocatore che non ha fatto login (gestito dal main)
#define LOGGED 0x01		// status del giocatore che ha fatto login (gestito dal main)
#define PLAYING 0x02		// status del giocatore che sta giocando (gestito dalla room)	

// status della room
#define CREATED 0x00		// status della room creata, i giocatori stanno joinando
#define STARTED 0x01		// status della room in cui tutti i giocatori sono entrati ed il game sta iniziando
#define QUITTING 0x01		// status della room terminata, i giocatori stanno uscendo

// opcodes (messaggi)
#define LOGIN 0x01		// messaggio di login
#define REGISTER 0x00		// messaggio di register
#define LOGOUT 0x02		// messaggio di logout
#define OK 0xFA			// messaggio di ok
#define NOK 0xAF		// messaggio di non ok
// opcodes (messaggi) - room
#define START_ROOM 0xFF		// messaggio di start room
#define QUIT_ROOM 0x7F		// messaggio di uscita dalla room
#define ROOM_LIST 0xBF		// messaggio di richiesta della lista delle rooms

// opcodes (messaggi) - escape room
#define USE 0xBD
#define LOOK 0xBE
#define TAKE 0xBF

typedef uint8_t natb;
typedef uint16_t natw;
typedef uint32_t natl;
typedef uint64_t natq;


#endif




