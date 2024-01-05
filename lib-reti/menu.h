#ifndef MENU_INCLUSA
#define MENU_INCLUSA
#include "../lib-reti.h"
// metodi comuni
void roomList(int sd);
void printMenu();
void printHome();
void printHelp();
#ifdef SERVER
#else
// metodi del client
void printMenu();
size_t avviaRoom(int sd,char * arg1);

#endif
#endif
