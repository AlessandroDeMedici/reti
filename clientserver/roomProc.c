#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "./lib-reti/lib-reti.h"


// il processo figlio eseguira questo codice
// ricevera in ingresso una pipe
// gestira la comunicazione con il processo server tramite una pipe
// piccolo riassunto:
// 	1. gestira un singolo game con una sua scena (inizialmente sempre la stessa, successivamente amplieremo)
// 	2. ricevera connessioni dal main e gestira i loro socket
// 	3. rispedira i player al main
//
//
//
// per il momento implementiamo un'applicazione che riceve socket dal main, attende che siano 4 i player
// attende 5 secondi e poi li rispedisce tutti al main
//
// LA PIPE VIENE USATA SOLO PER SCAMBIARE LA STESSA STRUTTURA CHE SONO I PLAYER
int main(int argn, char * argv[])
{
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

	// controllo sugli input DA AGGIUNGERE

	// acquisisco i file descriptors delle pipe
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
		ret = select(max_fd + 1, &read_fds,NULL,NULL,NULL);
		printf("(%d): descrittori pronti: %d\n",id,ret);
		if (ret == -1)
			printf("errno: %d\n",errno);
		for (int i = 0; i < max_fd + 1; i++){
			if (!FD_ISSET(i,&read_fds))
				continue;
			if (i == pf[0]){
				// e' arrivato un nuovo player
				int new_sd, ricevuti = 0;
				while(ricevuti < sizeof(new_sd)){
					ret = read(i,&new_sd,sizeof(new_sd));		// ricevuto
					ricevuti += ret;
				}
				players++;
				printf("(%d): %d è entrato a far parte della room, adesso ci sono %d players\n",id,new_sd,players);
				if (new_sd > max_fd)
					max_fd = new_sd;
				FD_SET(new_sd,&master);
			} else {
				// devo servire la richiesta di un player
				natb opcode = 0;
				int inviati = 0;
				int ricevuti = 0;
				ret = 0;
				while (ricevuti < sizeof(opcode)){
					ret = recv(i,&opcode + ricevuti,sizeof(opcode) - ricevuti,0);
					ricevuti += ret;
				}
				// se il client ha chiuso la connessione questo viene
				// notificato al main process ugualmente
				if (!ret)
					opcode = QUIT_ROOM;
				// switch sui possibili comandi
				switch(opcode){
					case (QUIT_ROOM):
						// vieni rispedito al main server process
						while(inviati < sizeof(i)){
							ret = write(fp[1],&i,sizeof(i));		// inviato
							inviati += ret;
						}
						players--;
						printf("(%d): %d è uscito dalla room, adesso ci sono %d players\n",id,i,players);
						FD_CLR(i,&master);
					
					
						// se non ci sono piu players
						if (!players){
							printf("(%d): chiusura della room...\n",id);
							exit(0);
						}
						break;
					default:
						printf("(%d): %d mi ha inviato %d\n",id,i,opcode);
						break;
				}
			}
		}
	}
	return 0;
}
