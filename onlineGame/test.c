#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define NUM_PROC 20

int main(){
	int pip[NUM_PROC][2];
	pid_t pid;
	char login[] = "1\n";
	char username[] = "admin%d\n";
	char message[50]; 
	for (int i = 0; i < NUM_PROC; i++){
		sleep(1);
		pipe(pip[i]);
		pid = fork();
		if (!pid){
			// processo figlio
			dup2(pip[i][0],STDIN_FILENO);
			execl("./client","./client",NULL);
			exit(1);
		}
	}
	for (int i = 0; i < NUM_PROC; i++){
		sprintf(message,username,i);
		//sleep(1);	// modalita login
		write(pip[i][1],login,strlen(login));
		//sleep(1);	// username
		write(pip[i][1],message,strlen(message));
		//sleep(1);	// password
		write(pip[i][1],message,strlen(message));
		//sleep(1);	// invio messaggio partecipare room
		write(pip[i][1],login,strlen(login));
	}
	for (int i = 0; i < NUM_PROC; i++){
		//sleep(1);	// invio messaggio di ok
		write(pip[i][1],login,strlen(login));
	}
	for (int i = 0; i < NUM_PROC; i++){
		//sleep(1);	// invio messaggio di quit
		write(pip[i][1],login,strlen(login));
	}
	for (int i = 0; i < NUM_PROC; i++){
		//sleep(1);	// chiudo i socket ed esco
		write(pip[i][1],login,strlen(login));
	}

	wait(NULL);
	exit(0);
}
