#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib-reti/lib-reti.h"
#include <fcntl.h>

int main () 
{	
	char username[] = "admin%d";
	char temp[50];

	int ret, max_sd, sd;
	char buffer[1024];
	struct sockaddr_in my_addr, client_addr;
	socklen_t len;
	fd_set master, read_master;

	// creazione socket
	sd = socket(AF_INET, SOCK_STREAM, 0);
	// inizializzazione indirizzo
	memset(&my_addr, 0, sizeof(my_addr)); // Pulizia
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(1212);
	inet_pton(AF_INET, "1227.0.0.1", &my_addr.sin_addr);
	// allaccio del socket
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if (!ret) ;
	// iniziamo ad attendere connessioni
	ret = listen(sd, 10);
	if (!ret) ;
	len = sizeof(client_addr);

	// inizializzo fd set
	FD_ZERO(&read_master);
	FD_ZERO(&master);
	FD_SET(sd,&master);
	max_sd = sd;
	
	// nuovo utente di prova
	for (int i = 0; i < 20; i++){
		sprintf(temp,username,i);
		nuovoUtente(temp,temp);
	}

	while(1)
	{
		read_master = master;
		ret = select(max_sd + 1,&read_master,NULL,NULL,NULL);
		printf("select ha ritornato: %d\n",ret);
		for (int i = 0; i < max_sd + 1; i++){
			if (!FD_ISSET(i,&read_master))
				continue;
			if (i == sd){
				// si accetta una nuova connessione
				char username[50];
				char password[50];
				int new_sd = accept(sd,(struct sockaddr *)&client_addr,&len);
				struct user * utente;
				struct des_connection * connessione = nuovaConnessione(new_sd);
				utente = serverLogin(new_sd,username,password);
				if (utente){
					// login o registrazione completati correttamente
					loginConnessione(connessione,utente);
					FD_SET(new_sd,&master);
					if (new_sd > max_sd){
						max_sd = new_sd;
					}
				} else {
					// login o registrazione non completati correttamente
					chiudiConnessione(new_sd);
					close(new_sd);
				}
			} else {
				// un file descriptor e' pronto
				natb opcode = 0;
				int ret = 0;
				// se questo file descriptor e' associato ad una pipe allora una room
				// ci sta inviando un messaggio
				// altrimenti e' un socket
				struct des_room * stanza = getRoom(i);
				if (!stanza){
					natl room_id = 0;
					struct des_connection * conn = 0;
					// questo e' un socket che ci sta inviando un messaggio
					ret = recv(i,&opcode,sizeof(opcode),0);
					if (!ret){ 
						// il socket ha chiuso la comunicazione
						FD_CLR(i,&master);
						close(i);
						chiudiConnessione(i);
					}
					// switch sui vari comandi
					switch(opcode){
						case(START_ROOM):	
							// ottengo il descrittore di connessione
							conn = getConnessione(i);

							// ricevo il room_id
							ret = recv(i,&room_id,sizeof(room_id),0);
							room_id = ntohl(room_id);
							printf("%d vuole entrare nella room %d\n",i,room_id);
							// entro nella room, se questa non esiste prima la creo
							stanza = checkRoom(room_id);
							if (!stanza){
								stanza = createRoom(room_id);
								printf("La room non esisteva, l'ho creata\n");
								// se e' stata creata bisogna inserire fd della pipe in master
								FD_SET(stanza->fp[0],&master);
								if (stanza->fp[0] > max_sd)
									max_sd = stanza->fp[0];
							} else {
								printf("La room esisteva\n");
							}
							ret = joinRoom(i,conn->utente,stanza);
							if (!ret){
								playingConnessione(i);
								printf("%d e' entrato nella room %d\n",i,room_id);
								FD_CLR(i,&master);	
							} else { // la room era piena
								printf("%d non e' entrato nella room %d perche era piena\n",i,room_id);
							}
							break;
						case(ROOM_LIST):
							if (!activeRooms(buffer))
								sendString(i,buffer);
							else
								sendString(i,"Nessuna stanza attiva\n");
							break;
						default:
							break;
					}
				} else {
					// questo e' un fd relativo ad una pipe che ci sta inviando un socket indietro
					int ricevuti = 0, sd = 0;
					natb opcode;
					stanza->status = QUITTING;
					while(ricevuti < sizeof(sd)){
						ret = read(i,&sd + ricevuti,sizeof(sd) - ricevuti);
						ricevuti += ret;
					}
					printf("%d sta tornando alla home\n",sd);
					printf("players prima: %d\n",stanza->numPlayers);
					stanza->numPlayers--;
					printf("players dopo: %d\n",stanza->numPlayers);
					// se non ci sono piu giocatori nella stanza uccidi il processo
					// e rimuovi il file descriptor
					if (!stanza->numPlayers){
						// elimina la room
						if(closeRoom(stanza->id))
							printf("room non chiusa correttamente\n");
						printf("la room è stata chiusa\n");
						FD_CLR(stanza->fp[0],&master);
					}
					// reinserisci sd in master
					homeConnessione(sd);
					FD_SET(sd,&master);
				}	
			}
		}
	}
	return 0;
}


