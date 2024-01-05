// user.h
#ifndef USER_INCLUSA
#define USER_INCLUSA
#include "../lib-reti.h"

struct user
{
	char username[50];
	char password[50];
	struct user * next;
};

struct user * nuovoUtente(char*, char*);
struct user * controllaUtente(char*, char*);
struct user * controllaUsername(char *);

#endif
