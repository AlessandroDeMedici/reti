#ifndef LOGIN_INCLUSA
#define LOGIN_INCLUSA
#include "../lib-reti.h"

size_t makeStringaLogin(char *, char *, char *);
void demakeStringaLogin(char *, char *, char *);
size_t inviaLogin(int,char*,char*);
size_t riceviLogin(int, char *, char *);
size_t loginUtente(int, char *, char *);
struct user * loginServer(int, char *, char *);
size_t registerUtente(int,char *,char *);
struct user * registerServer(int,char*,char*);
size_t userLogin(int,char *,char *);
struct user * serverLogin(int,char*,char*);

#endif
