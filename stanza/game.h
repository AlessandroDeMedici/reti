#ifndef GAME_INCLUSA
#define GAME_INCLUSA
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>

#define NUM_OGGETTI 3

typedef uint8_t natb;
typedef uint16_t natw;
typedef uint32_t natl;
typedef uint64_t natq;

struct oggetto
{
	char nome[32];
	char descrizione[512];
	char enigma[512];
	char risposta[32];
};

struct room
{
	struct oggetto oggetti[NUM_OGGETTI];
	char descrizione[512];
	natl id;
};

struct game
{
	struct room stanza;
	time_t start_time;
	char timer[6];
} gioco;

void salvaRoom(struct room*);
size_t caricaRoom(struct room*, natl id);
void initFile();
void initGame();
void stampaFrame();
natb elabString(char *);
void makeTitle();

#endif
