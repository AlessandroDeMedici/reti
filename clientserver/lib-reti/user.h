// user.h
#ifndef USER_INCLUSA
#define USER_INCLUSA

#include <arpa/inet.h>

struct user
{
	char username[50];
	char password[50];
	struct user * next;
};

// lista di tutti gli utenti
struct user * nuovoUtente(char * username, char * password);
struct user * controllaUtente(char * username, char * password);


#endif
