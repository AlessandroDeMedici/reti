#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "lib-reti/lib-reti.h"



int main () 
{
	int ret, sd;
	struct sockaddr_in server_addr; // per il server
	char username[50];
	char password[50];
	natb opcode = 0;
	char buffer[1024];
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
	
	//invio che voglio far parte della room 1
	opcode = ROOM_LIST;
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (!ret) ;
	receiveString(sd,buffer);
	printf("LISTA DELLE ROOMS:\n%s",buffer);

	// il devo attendere il ritorno di room list

	scanf("%d",&ret);

	close(sd);
	return 0;
}
