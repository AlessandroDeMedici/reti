#include "utility.h"
#include <string.h>
#include <arpa/inet.h>
#include "costanti.h"
#include <stdio.h>

size_t sendString(int sd, char * string)
{ 
	int ret, sent = 0, len = strlen(string) + 1;
	natl length = htonl(len);
	printf("la stringa e' lunga: %d\n",len);
	ret = send(sd,&length,sizeof(natl),0);
	ret = send(sd,string + sent,len - sent,0);
	if (!ret) ;
	return 0;
}

size_t receiveString(int sd, char * string)
{ 
	int len, ret, received = 0;
	natl length;
	ret = recv(sd,&length,sizeof(natl),0);
	len = ntohl(length);
	ret = 0;
	ret = recv(sd,string + received,len - received,0);
	if (!ret) ;
	return 0;
}

void stampaAnimata(char * buffer)
{
	for (int i = 0; buffer[i] != '\0'; i++){
		printf("%c",buffer[i]);
		fflush(stdout);
		usleep(10000);
	}
}

