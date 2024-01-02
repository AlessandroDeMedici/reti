#include "lib-reti.h"

int main (int argn, char * argv[]) 
{
	int port;
	int ret, main;
	struct sockaddr_in server_addr; // per il server
	char username[64];
	char password[64];
	
	// controllo sugli input
	if (argn < 2)
	{
		printf("client - missing port\n");
		return 1;
	}

	// inizializzazione della porta
	port = atoi(argv[1]);
	if (port < 0)
	{
		printf("client - bad port\n");
		return 1;
	}
		
	// inizializzazione gioco
	init();

	/* Creazione socket */
	main = socket(AF_INET, SOCK_STREAM, 0);
	
	initsd(main);

	/* Creazione indirizzo del server */
	memset(&server_addr, 0, sizeof(server_addr)); // Pulizia
	server_addr.sin_family = AF_INET ;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);


	ret = connect(main, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret) ;

	// funzione per fare il login
	if (userLogin(main,username,password)){
		close(main);
		exit(0);
	}
	
	// una volta fatto il login si entra nel main menu
	printHome();

	while (1){
		char buffer[256];
		char * command;
		char * arg1;
		
		printf("> ");
		fgets(buffer,255,stdin);
		for (int i = 0; i < 256; i++)
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		
		command = strtok(buffer," ");
		arg1 = strtok(NULL," ");

		if (!command){
			printf("\r\033[K");
			continue;
		}
		else if (!strcmp(command,"start")){ // avvia la room
			if (!arg1){
				printf("start - missing room id\n");
				continue;
			}
			if (!avviaRoom(main,arg1))
				game();
			else {
				printf("Room piena o gia' avviata\n");
				continue;
			}
		} else if (strstr(command,"list")){
			roomList(main);
		} else if (strstr(command,"end")) // chiude la connessione con il server
			break;
		else
			printf("\033[A\r\033[K");
	}
	
	close(main);
	exit(0);
	return 0;
}
