#include "game.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MINUTES 3
#define ROWS 10
#define COLUMNS 50

void * aggiornaTimer()
{
	time_t current_time;
	while (1)
	{
		time(&current_time);
		int remaining = MINUTES * 60 - difftime(current_time,gioco.start_time);
		printf("\033[s\033[%dA\r\033[%dC\033[1;31mTEMPO RIMANENTE: %02d:%02d\033[00m\033[u",ROWS-1,COLUMNS/2 - 11,remaining/60,remaining%60);
		fflush(stdout);
		if (!remaining)
			break;
		sleep(1);
	}
	pthread_exit(NULL);
	return 0;
}

void salvaRoom(struct room *r)
{
	FILE *file;
	// Apri il file in modalità append binaria
	file = fopen("map", "ab");
	if (file == NULL) {
        	perror("Errore nell'apertura del file");
        	exit(EXIT_FAILURE);
	}
	
	// Scrivi la struttura a pezzi di 100 byte
	fwrite(r, sizeof(struct room), 1, file);
	
	// Chiudi il file
	fclose(file);
}

size_t caricaRoom(struct room *r, natl id)
{
	FILE *file;
	size_t ret;
	
	// Apri il file in modalità lettura binaria
	file = fopen("map", "rb");

	if (file == NULL) {
        	perror("Errore nell'apertura del file");
        	exit(EXIT_FAILURE);
	}
	
	while(1){
		if (!fread(r, sizeof(struct room), 1, file))
		{	// sono arrivato alla fine del file
			memset(r,0,sizeof(struct room));
			fclose(file);
			return 1;
		}
		if (r->id == id)
			break;
	}
	// Chiudi il file
	fclose(file);
	return 0;
}

void initFile()
{
	char nome[] = "nome";
	char descrizione[] = "descrizione";
	char enigma[] = "enigma";
	char risposta[] = "risposta";
	char roomdesc[] = "stanza: %d, oggetto1: %s, oggetto2: %s, oggetto3: %s";
	natl id = 0;
	struct room stanza;

	for (int j = 0; j < 5; j++){	
		for (int i = 0; i < NUM_OGGETTI; i++){
			sprintf(stanza.oggetti[i].nome,"%s",nome);
			sprintf(stanza.oggetti[i].descrizione,"%s",descrizione);
			sprintf(stanza.oggetti[i].enigma,"%s",enigma);
			sprintf(stanza.oggetti[i].risposta,"%s",risposta);
		}

		sprintf(stanza.descrizione,"%s",roomdesc);
		stanza.id = id++;
		salvaRoom(&stanza);
	}
}

void initGame()
{
	pthread_t tid;
	time(&gioco.start_time);
	pthread_create(&tid,NULL,aggiornaTimer,NULL);
};

void stampaFrame()
{

	// top
	for (int i = 0; i < COLUMNS; i++){
		printf("*");
	}
	printf("\n");
	
	//intermedio
	for (int j = 0; j < ROWS - 2; j++){
		printf("*");
		for (int i = 0; i < COLUMNS - 2; i++)
			printf(" ");
		printf("*\n");
	}

	// bottom
	for (int i = 0; i < COLUMNS; i++){
		printf("*");
	}
	printf("\n");

	printf("\n");
}

natb elabString(char * buffer)
{
	char look[] = "look";
	char start[] = "start";
	char use[] = "use";
	char objs[] = "objs";
	char take[] = "take";
	char end[] = "end";

	if (!strncmp(buffer,start,strlen(start))){
		// e' stato riconosciuto il comando di start
	} else if (!strncmp(buffer,look,strlen(look))){
		// e' stato riconosciuto il comando di look
	} else if (!strncmp(buffer,take,strlen(take))){
		// riconosciuto comando di take
	} else if (!strncmp(buffer,use,strlen(use))){
		// riconosciuto comando di use
	} else if (!strncmp(buffer,objs,strlen(objs))){
		// riconosciuto comando di objs
	} else if (!strncmp(buffer,end,strlen(end))){
		// riconosciuto comando di end
	} else {
		
	}

}

void makeTitle()
{
	printf("\033[s\033[%dA\r\033[%dC\033[1;31mBENVENUTO ALLA ESCAPE ROOM\033[00m\n",ROWS-1,COLUMNS/2 - 13,remaining/60,remaining%60);
	printf("\r\033[COPZIONI DISPONIBILI:\n");
	printf("\r\033[Clogin\n");
	printf("\r\033[Csingup\n");
	printf("\033[u");
}


