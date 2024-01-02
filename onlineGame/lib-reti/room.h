#ifndef ROOM_INCLUSA
#define ROOM_INCLUSA
#include "../lib-reti.h"
struct des_room
{ 
	struct des_room * next;	// puntatore per realizzare lista di rooms
	natl id;		// id della room	
	int pf[2];		// pipe per comunicazioni padre->figlio
	int fp[2];		// pipe per comunicazioni figlio->padre
	natl numPlayers;
	int players[MAX_PLAYERS];
	struct user * users[MAX_PLAYERS];
	natb status;
};

struct des_room * checkRoom(natl);
struct des_room * createRoom(natl);
size_t closeRoom(natl);
size_t joinRoom(int,struct user*,struct des_room *);
int backRoom(struct des_room *);
struct des_room * getRoom(natl);
size_t activeRooms(char *buffer);
size_t sendPlayer(struct des_room *, int,char*,char padre);
int receivePlayer(struct des_room *,char*, char padre);
size_t startRoom(struct des_room *);
#ifdef SERVER
void avviaRoom(int sd, int max_sd, fd_set * master, struct des_room * stanza);
void tornaIndietro(int sd, fd_set * master, struct des_room * stanza);
#endif
#endif
