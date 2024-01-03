#include "lib-reti.h"
#include <pthread.h>

int chiusura = 0;
int started = 0;

void * server(void * arg)
{
	int port;
	int i;
	char username[50];
	char password[50];
	struct user * utente;
	struct des_connection * connessione;
	struct des_room * stanza;
	natb opcode = 0;	// tipo definito in lib-reti/costanti.h

	// variabili di appoggio per creare degli utenti di prova
	// (admin0,admin0),(admin1,admin1),...,(admin20,admin20)
	char username_prova[] = "admin%d";
	char temp[50];
	
	int ret, max_sd, sd;
	struct sockaddr_in my_addr, client_addr;
	socklen_t len;
	fd_set master, read_master;

	// passaggio della porta
	port = *(int *)arg;
	free(arg);
	
	// creazione socket
	sd = socket(AF_INET, SOCK_STREAM, 0);
	
	// inizializzazione indirizzo
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);
	
	// allaccio del socket
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if (ret == -1){
		started = 0;
		perror("Error binding...");
		pthread_exit(NULL);
	}
	
	// inizio ad ascoltare connessioni
	ret = listen(sd, 10);
	if (ret == -1){
		started = 0;
		perror("Error listen...");
		pthread_exit(NULL);
	}
	len = sizeof(client_addr);

	// server avviato correttamente
	printf("(Main) server avviato correttamente!\n");

	// inizializzo fd set
	FD_ZERO(&read_master);
	FD_ZERO(&master);
	FD_SET(sd,&master);
	max_sd = sd;
	
	// utenti di prova
	for (i = 0; i < 20; i++){
		sprintf(temp,username_prova,i);
		nuovoUtente(temp,temp);
	}

	while(1)
	{
		// prima di attendere le prossime richieste
		// controllo che il server non sia stato richiesto stop dall'utente
		if (chiusura){
			// puo terminare solo in caso di nessuna connessione e di nessuna room
			if (nessunaConnessione() && nessunaRoom()){
				started = 0;
				close(sd);
				pthread_exit(0);
			}
		}
		// copia fd_set
		read_master = master;
		// select bloccante (si blocca finche non arriva almeno una richiesta)
		ret = select(max_sd + 1,&read_master,NULL,NULL,NULL);
		if (ret <= 0){
			// si e' verificato un errore
			perror("(Main) errore nella select");
		}

		// ciclo su tutti i socket descriptor da 0 a max_sd
		for (i = 0; i < max_sd + 1; i++){
			// non e' un socket pronto
			if (!FD_ISSET(i,&read_master))
				continue;
			// e' arrivata una richiesta dal listening socket (nuova connessione)
			if (i == sd){
				// si accetta una nuova connessione
				int new_sd = accept(sd,(struct sockaddr *)&client_addr,&len);
				
				// si inizializzano la nuova connessione e l'utente
				// e si procede con la fase di login o di registrazione
				connessione = nuovaConnessione(new_sd); // definita in lib-reti/connessione.c
				utente = serverLogin(new_sd,username,password); // definita in lib-reti/login.c

				if (utente){
					// login o registrazione completati correttamente
					loginConnessione(connessione,utente); // definita in lib-reti/connessione.c
					FD_SET(new_sd,&master);
					if (new_sd > max_sd){
						max_sd = new_sd;
					}
				} else {
					// login o registrazione non completati correttamente
					chiudiConnessione(new_sd); // definita in lib-reti/connessione.c
					close(new_sd);
				}
			} else {
				// un file descriptor e' pronto
				// questo file descriptor puo essere associato ad una pipe (e quindi una room)
				// oppure ad un utente nella home
				int ret = 0;
				stanza = getRoom(i); // definita in lib-reti/room.c
				if (!stanza){
					// questo file descriptor e' associato ad un socket (un client nella home)
					ret = recv(i,&opcode,sizeof(opcode),0);
					if (!ret){
						// il socket ha chiuso la comunicazione
						FD_CLR(i,&master);
						close(i);
						chiudiConnessione(i); // definita in lib-reti/connessione.c
						continue;
					}
					// switch sui vari comandi
					switch(opcode){
						case(START_ROOM):
							// l'utente vuole creare o entrare in una room
							avviaRoom(i, max_sd,&master); 
							// funzione definita in lib-reti/menu.c
							break;
						case(ROOM_LIST):
							// l'utente vuole una lista delle room attualmente attive
							roomList(i); // funzione definita in lib-reti/menu.c
							break;
						default:
							break;
					}
				} else {
					// questo file descriptor e' associato ad una pipe (e dunque ad una room)
					tornaIndietro(i,&master,stanza); // definita in lib-reti/room.c
				}	
			}
		}
	}
	pthread_exit(NULL);
}

int main (int argn, char * argv[]) 
{
	char buffer[128];
	char *command;
	char *arg;
	int port;
	int * temp;
	int i;
	int first = 1;
	pthread_t thread;

	// stampo il menu
	printMenu();
	
	while (1){
		chiusura = 0;
		// prima esecuzione:
		// e' richiesto di poter avviare il server come ./server <port>
		if (first && argn > 1){
			// prima esecuzione
			sprintf(buffer,"start %d",atoi(argv[1]));
			first = 0;
		} else {
			// esecuzioni successive alla prima
			fgets(buffer,128,stdin);
			// correggo l'input (ho bisogno della marca di fine stringa)
			for (i = 0; i < 128; i++){
				if (buffer[i] == '\n'){
					buffer[i] = '\0';
					break;
				}
			}
		}

		// divido la stringa in comando e argomento divisi da spazio
		command = strtok(buffer," ");
		arg = strtok(NULL,"");

		if (!command){
			// l'utente ha soltanto premuto invio e non ci sono comandi
			continue;
		}

		if (!strcmp(command,"start")){
			// l'utente vuole avviare il server
			if (started){
				// ma e' gia avviato
				printf("start - server already started...\n");
				continue;
			}
			if (!arg){
				// ma non ha fornito la porta
				printf("start - missing argument...\n");
				continue;
			}
			// posso avviare correttamente il server (in un thread separato)
			// in questo modo posso continuare ad eseguire questo menu
			sscanf(arg,"%d",&port);
			temp = (int *)malloc(sizeof(int));
			*temp = port;
			printf("Server in fase di avvio...\n");
			started = 1;
			pthread_create(&thread,NULL,server,(void *)temp);
		} 
		else if (!strcmp(command,"stop")){
			// l'utente vuole terminare il server
			if (!started){
				// ma il server non e' avviato
				printf("Server non ancora avviato...\n");
				continue;
			}
			// posso chiudere correttamente il server
			chiusura = 1;
			printf("Attendo la chiusura di tutte le connessioni...\n");
			// attendo che il thread sia terminato
			pthread_join(thread,NULL);
			printf("Server chiuso correttamente!\n");
			started = 0;
		} 
		else if (!strcmp(command,"help")){
			// l'utente vuole vedere nuovamente il menu
			printMenu();
		} 
		else if (!strcmp(command,"exit")){
			// l'utente vuole uscire
			if (started){
				// ma c'e' un server in esecuzione
				printf("impossibile uscire, server in esecuzione...\n");
			} else
				break;
		}
	}
}


