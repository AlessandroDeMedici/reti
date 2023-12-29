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
	int ret, sd, inviati = 0;
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
	

	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&server_addr, 0, sizeof(server_addr)); // Pulizia
	server_addr.sin_family = AF_INET ;
	server_addr.sin_port = htons(1212);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);


	ret = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret) ;

	userLogin(sd,username,password);
	
	scanf("%d",&ret);
	
	//invio che voglio far parte della room 1
	ret = 0;
	opcode = START_ROOM;
	while(!ret){
		ret = send(sd,&opcode,sizeof(opcode),0);
	}
	
	// invio il numero della room
	inviati = 0;
	room_id = htonl(room_id);
	while (inviati < sizeof(room_id)){
		ret = send(sd,&room_id + inviati,sizeof(room_id) - inviati,0);
		inviati += ret;
	}
	
	scanf("%d",&ret);
	
	// invio il messaggio di ok
	ret = 0;
	opcode = OK;
	while(!ret){
		ret = send(sd,&opcode,sizeof(opcode),0);
	}

	scanf("%d",&ret);

	// voglio uscire dalla room
	ret = 0;
	opcode = QUIT_ROOM;
	ret = send(sd,&opcode,sizeof(opcode),0);
	while(!ret){
		ret = send(sd,&opcode,sizeof(opcode),0);
	}

	scanf("%d",&ret);

	close(sd);
	exit(0);
	return 0;
}
