#include "lib-reti.h"
#include <errno.h>

int main(int argn, char * argv[])
{
	int len;
	char buffer[128];
	char username[50];
	int i;
	int new_sd;
	natb opcode = 0; // tipo definito in lib-reti/costanti.h

	// pipe per lo scambio di dati con il processo padre
	int pf[2];
	int fp[2];
	
	// numero di giocatori attivi
	natl players = 0;
	natl id = 0;

	// strutture per gli fd_set
	fd_set master;
	fd_set read_fds;
	int max_fd;
	int ret;

	// inizializzazione unico scenario disponibile
	init();

	// acquisisco i file descriptors delle pipe e id della room
	// pf (padre -> figlio)
	// fp (figlio -> padre)
	pf[0] = atoi(argv[1]);
	pf[1] = atoi(argv[2]);
	fp[0] = atoi(argv[3]);
	fp[1] = atoi(argv[4]);
	id = atoi(argv[5]);

	//inizializzo gli fd_set
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(pf[0],&master);
	max_fd = pf[0];

	while(1){
		read_fds = master;
		
		// faccio la select
		ret = select(max_fd + 1, &read_fds,NULL,NULL,NULL);
		if (ret == -1){
			sprintf(buffer,"(%d) errore durante la select",id);
			perror(buffer);
		}

		// se pronto, do la precedenza a pf[0]
		if (FD_ISSET(pf[0],&read_fds)){
			// se questo fd e' pronto allora la home ci sta invando un player
			// (siamo interessati solo a fd del player e username)
			ret = read(pf[0],&new_sd,sizeof(new_sd));	// sd del player
			ret = read(pf[0],&len,sizeof(len));		// lunghezza username del player
			ret = read(pf[0],username,len);			// username player
			
			// una volta ottenuto il player lo aggiungiamo ai giocatori
			// della partita
			addPlayer(username,new_sd); // funzione definita in lib-reti/game.c
			
			// teniamo il conto dei player
			players++;

			// log
			printf("(%d) %s è entrato a far parte della room, adesso ci sono %d players\n",id,username,players);
			
			if (new_sd > max_fd)
				max_fd = new_sd;
			
			// quando tutti i client si sono connessi li sblocco ed avvio il game
			if (players == MAX_PLAYERS){
				sbloccaPlayers(); // funzione definita in lib-reti/game.c
				startTime(); // funzione definita in lib-reti/game.c
			}

			FD_SET(new_sd,&master);
		}

		// una volta servito pf[0], passo a servire gli altri fd
		for (i = 0; i < max_fd + 1; i++){
			// se non e' pronto o e' pf[0] allora ignoral
			if (!FD_ISSET(i,&read_fds) || i == pf[0])
				continue;
			
			// ricevo l'opcode
			ret = recv(i,&opcode,sizeof(opcode),0);
			

			// se il client ha chiuso la connessione, per fare in modo
			// di notificarlo alla home faccio come se avessi ricevuto
			// un comando di QUIT_ROOM
			if (!ret)
				opcode = QUIT_ROOM;


			// switch sui possibili comandi
			switch(opcode){
				// comando di QUIT_ROOM
				case (QUIT_ROOM):
					// rilascio gli oggetti posseduti dal client
					quitRoom(i); // funzione definita in lib-reti/room.c
					
					// rispedisco il fd all home
					write(fp[1],&i,sizeof(i));

					// decremento il numero di players
					players--;

					//log
					printf("(%d) il socket %d è uscito dalla room, adesso ci sono %d players\n",id,i,players);
					FD_CLR(i,&master);

					if (!players){
						// se non ci sono piu players chiudo la room
						printf("(%d) chiusura della room...\n",id);
						exit(0);
					}
					break;
				default:
				// comandi inerenti al gioco
					game(id,i,opcode); // funzione definita in lib-reti/game.c
					break;
			}
		}
	}
	return 0;
}
