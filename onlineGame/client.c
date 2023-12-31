#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "lib-reti/lib-reti.h"
#include <time.h>

int main () 
{
	int ret, main, inviati = 0;
	struct sockaddr_in server_addr; // per il server
	char username[50];
	char password[50];
	natb opcode = 0;
	natl room_id;
	srand(time(NULL));
	room_id = rand()%5;
	//room_id = 1024;
	memset(username,0,50);
	memset(password,0,50);
	
	// inizializzazione gioco
	init();

	/* Creazione socket */
	main = socket(AF_INET, SOCK_STREAM, 0);
	
	initsd(main);


	/* Creazione indirizzo del server */
	memset(&server_addr, 0, sizeof(server_addr)); // Pulizia
	server_addr.sin_family = AF_INET ;
	server_addr.sin_port = htons(1212);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);


	ret = connect(main, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret) ;

	// funzione per fare il login
	userLogin(main,username,password);
	
	// una volta fatto il login si entra nel main menu
	printHome();
	while (1){
		char buffer[256];
		char * command;
		char * arg1;
		fgets(buffer,255,stdin);
		for (int i = 0; i < 256; i++)
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		command = strtok(buffer," ");
		arg1 = strtok(NULL," ");
		if (!command)
			continue;
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
	}
	
	close(main);
	exit(0);
	
	return 0;
}
