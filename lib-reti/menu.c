#include "menu.h"

#ifdef SERVER
// codice del server

// descrizione:
// funzione usata dal server per rispondere alla richiesta di roomList
// argomenti:
// sd -> descrittore del socket
void roomList(int sd)
{
	char buffer[512];

	if (!activeRooms(buffer)) // funzione definita in lib-reti/room.c
		sendString(sd,buffer);
	else
		// se non sono attive stanze invio una stringa.
		// un'ottimizzazione e' inviare una dimensione nulla della stringa
		// e il client riconosce che non sono presenti stanze.
		sendString(sd,"Nessuna stanza attiva\n");
	printf("(Main) (%d) ha fatto roomList()\n",sd);
}

// descrizione:
// funzione usata dal server per stampare il menu principale
void printMenu()
{
	printf(ANSI_GREEN "***************** SERVER ESCAPE ROOM ****************\n" ANSI_RESET);
	printf("Comandi:\nstart [port]\t-> avvia il server sulla porta [port]\n");
	printf("stop\t\t-> termina il server\n");
	printf("help\t\t-> stampa questo menu con i comandi\n");
	printf("exit\t\t-> esci\n");
	printf(ANSI_GREEN "*****************************************************\n" ANSI_RESET);
}

#else
// codice del client

// descrizione:
// funzione usata dal client per stampare il menu in fase di registrazione o accesso
void printMenu()
{
	printf(ANSI_GREEN "******************** ESCAPE ROOM ********************\n" ANSI_RESET);
	printf("Benvenuto sul server\n");
	printf("login:\t\tfai login\nregister:\tregistati\n");
	printf(ANSI_GREEN "*****************************************************\n" ANSI_RESET);
	printf("> ");
}

// descrizione:
// funzione usata dal client per richiedere le stanze attive
// argomenti:
// sd -> descrittore del socket
void roomList(int sd)
{
	natb opcode = ROOM_LIST;
	char buffer[256];
	
	// invio opcode
	int ret = send(sd,&opcode,sizeof(opcode),0);
	if (ret == -1){
		// errore nell'invio dell'opcode
		perror("roomList - errore in fase di send");
		return;
	}

	// ricevo la stringa
	receiveString(sd,buffer);
	printf("LISTA DELLE ROOMS:\n%s",buffer);
}

// descrizione:
// funzione usata dal client per inviare una richiesta di avvia room
// argomenti:
// sd -> descrittore del socket
// arg1 ->  stringa che contiene il room_id
// ritorno:
// ritorna 0 in caso di successo, 1 altrimenti
size_t avviaRoom(int sd, char * arg1)
{
	natb opcode = START_ROOM;
	natl room;
	int ret;

	// acquisisco il numero di room
	sscanf(arg1,"%d",&room);

	// invio il messaggio di START_ROOM
	ret = send(sd,&opcode,sizeof(opcode),0);
	if (ret == -1){
		perror("avviaRoom - errore in fase di send");
		return 1;
	}

	// invio il numero di room
	room = htonl(room);
	ret = send(sd,&room,sizeof(room),0);
	if (ret == -1){
		perror("avviaRoom - errore in fase di send");
		return 1;
	}


	// attendo il messaggio di OK
	printf("(room %d) Waiting for players...\n",htonl(room));
	ret = recv(sd,&opcode,sizeof(opcode),0);
	if (!ret){
		perror("avviaRoom - errore in fase di recv");
		return 1;
	}
	if (opcode == OK){
		printf("Benvenuto nella room!\n");
		return 0;
	} else
		return 1;
}
#endif

// descrizione:
// funzione che stampa il main menu della home
void printHome()
{
	printf(ANSI_GREEN "******************** ESCAPE ROOM ********************\n" ANSI_RESET);
	printf("Benvenuto nel main menu:\n");
	printf("start [room]:\tavvia la room con ID [room]\n");
	printf("list:\t\tcontrolla tutte le rooma attive\n");
	printf("end:\t\tesci e chiudi la connessione\n");
	printf(ANSI_GREEN "*****************************************************\n" ANSI_RESET);
}

// descrizione:
// funzione che stampa il menu di gioco
void printHelp()
{
	printf(ANSI_GREEN "******************* Buona partita ******************\n" ANSI_RESET);
	printf("Comandi:\nlook [oggetto|location]\t\tstampa la descrizione di un oggetto o location\n");
	printf("take [oggetto]\t\t\tsblocca o ottieni un oggetto\n");
	printf("use [oggetto1] [oggetto2]\tusa un oggetto\n");
	printf("time\t\t\t\tottieni il tempo rimanente\n");
	printf("tell [messaggio]\t\tinvia il [messaggio] a tutti i player\n");
	printf("help\t\t\t\tstampa nuovamente questo menu con i comandi\n");
	printf("exit\t\t\t\tesci dall'enigma o dalla partita\n");
	printf(ANSI_GREEN "****************************************************\n" ANSI_RESET);
}



