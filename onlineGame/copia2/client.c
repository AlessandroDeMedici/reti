#include "lib-reti.h"

int main (int argn, char * argv[]) 
{
	int port;
	int ret, main_socket;
	struct sockaddr_in server_addr; 
	char username[64];
	char password[64];
	
	// controllo sugli input
	if (argn < 2){
		printf("client - missing port\n");
		return 1;
	}

	// inizializzazione della porta
	port = atoi(argv[1]);
	if (port < 0){
		printf("client - bad port\n");
		return 1;
	}
		

	// inizializzazione del socket
	main_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	
	// inizializzazione dell'unico scenario disponibile
	init(); // definita in lib-reti/game.c
	
	// inizializzazione delle strutture per il game
	initsd(main_socket); // definita in lib-reti/game.c

	// inizializzazione indirizzo del server
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	// connessione con il server
	ret = connect(main_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret){ // non sono riuscito a connettermi
		perror("server unreachable");
		return 1;
	}

	// fase di login o di register
	if (userLogin(main_socket,username,password)){ // funzione definita in lib-reti/login.c
		close(main_socket);
		return 1;
	}
	
	// una volta terminata con successo la fase di accesso o registazione
	// si entra nel menu principale
	
	// stampo il menu principale
	printHome();	// definita in lib-reti/menu.c

	while (1){

		char buffer[256];
		char * command;
		char * arg1;
		int i;

		// stampo il prompt
		printf("> ");

		// ottengo una riga da input
		fgets(buffer,255,stdin);
		for (i = 0; i < 256; i++)
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		
		// divido la stringa ottenuta in:
		// comando <arg1> (opzionale)
		command = strtok(buffer," ");
		arg1 = strtok(NULL," ");

		if (!command){
			// l'utente ha soltanto premuto invio
			continue;
		}
		else if (!strcmp(command,"start")){
			// l'utente vuole avviare la room
			if (!arg1){
				// ma non ha inserito il room id
				printf("start - missing room id\n");
				continue;
			}
			// avvio la room
			if (!avviaRoom(main_socket,arg1)) // funzione definita in lib-reti/menu.c
				// room avviata con successo
				// rimango in game() finche' non torno al main menu
				game(); // funzione definita in lib-reti/game.c
			else {
				// room non avviata con successo
				printf("Room piena o gia' avviata\n");
				continue;
			}
		} else if (!strcmp(command,"list")){
			// l'utente vuole una lista delle room attualmente attive
			roomList(main_socket); // funzione definita in lib-reti/menu.c
		} else if (!strcmp(command,"end"))
			// l'utente vuole chiudere l'applicazione
			break;
		else
			// l'utente ha inserito un comando scorretto, cancello il commando
			printf("\033[A\r\033[K");
	}
	
	close(main_socket);
	exit(0);
}
