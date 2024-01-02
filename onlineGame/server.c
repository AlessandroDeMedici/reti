#include "lib-reti.h"
#include <pthread.h>

int chiusura = 0;
int started = 0;

void * server(void * arg)
{
	int port;
	char username[] = "admin%d";
	char temp[50];
	int i;

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
	memset(&my_addr, 0, sizeof(my_addr)); // Pulizia
	my_addr.sin_family = AF_INET ;
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
		sprintf(temp,username,i);
		nuovoUtente(temp,temp);
	}

	while(1)
	{
		// controllo sulla chiusura del server
		if (chiusura){
			if (nessunaConnessione() && nessunaRoom()){
				started = 0;
				close(sd);
				pthread_exit(0);
			}
		}
		read_master = master;
		ret = select(max_sd + 1,&read_master,NULL,NULL,NULL);
		for (i = 0; i < max_sd + 1; i++){
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
				int ret = 0;
				// se questo file descriptor e' associato ad una pipe allora una room
				// ci sta inviando un messaggio
				// altrimenti e' un socket
				struct des_room * stanza = getRoom(i);
				if (!stanza){
					natb opcode = 0;
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
							avviaRoom(i, max_sd,&master,stanza);
							break;
						case(ROOM_LIST):
							roomList(i);
							break;
						default:
							break;
					}
				} else {
					// riportare i socket
					tornaIndietro(i,&master,stanza);
				}	
			}
		}
	}
	pthread_exit(NULL);
}

int main () 
{
	char buffer[128];
	char *command;
	char *arg;
	int port;
	int * temp;
	int i;
	pthread_t thread;
	// stampo il menu
	printMenu();
	while (1){
		chiusura = 0;
		fgets(buffer,128,stdin);
		// correggo l'input (ho bisogno della marca di fine stringa)
		for (i = 0; i < 128; i++){
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		}

		command = strtok(buffer," ");
		arg = strtok(NULL," ");

		if (!command){
			continue;
		}

		if (!strcmp(command,"start")){
			if (started)
			{
				printf("start - server already started...\n");
				continue;
			}
			if (!arg)
			{
				printf("start - missing argument...\n");
				continue;
			}
			// avvio il thread
			sscanf(arg,"%d",&port);	
			temp = (int *)malloc(sizeof(int));
			*temp = port;
			printf("Server in fase di avvio...\n");
			started = 1;
			pthread_create(&thread,NULL,server,(void *)temp);
		} else if (!strcmp(command,"stop")){
			if (!started){
				printf("Server non ancora avviato...\n");
				continue;
			}
			chiusura = 1;
			printf("Attendo la chiusura di tutte le connessioni...\n");
			pthread_join(thread,NULL);
			printf("Server chiuso correttamente!\n");
			started = 0;
		} else if (!strcmp(command,"help")){
			printMenu();
		} else if (!strcmp(command,"exit")){
			if (started){
				printf("impossibile uscire, server in esecuzione...\n");
			} else
				break;
		}
	}
}


