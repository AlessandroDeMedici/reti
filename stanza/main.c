#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "game.h"

int main(){
	char buffer[120];
	stampaFrame();
	makeTitle();
	//inizializzazione del file (prima esecuzione)
	//initFile();

	// carico la stanza numero
	caricaRoom(&gioco.stanza,4);

	
	initGame();

	while(1){
		printf("\033[A\r\033[KOutput: ");
		fgets(buffer,30,stdin);
	}

	return 0;
}
