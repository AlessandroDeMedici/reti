#include "menu.h"

#ifdef SERVER
// descrizione:
// funzione usata dal server per rispondere alla richiesta di roomList
// argomenti:
// sd -> descrittore del socket
void roomList(int sd)
{
	char buffer[512];
	if (!activeRooms(buffer))
		sendString(sd,buffer);
	else
		sendString(sd,"Nessuna stanza attiva\n");
	printf("(Main) (%d) ha fatto roomList()\n",sd);
}

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
// descrizione:
// funzione che stampa il menu mostrato in fase di login o di registrazione
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
	int ret = send(sd,&opcode,sizeof(opcode),0);
	if (ret <= 0)
		// errore nell'invio dell'opcode
		return;
	receiveString(sd,buffer);
	printf("LISTA DELLE ROOMS:\n%s",buffer);
}

// descrizione:
// funzione usata dal client per inviare una richiesta di avvia room
// argomenti:
// sd -> descrittore del socket
// arg1 ->  stringa che contiene il room_id
size_t avviaRoom(int sd, char * arg1)
{
	natb opcode = START_ROOM;
	natl room;
	// acquisisco il numero di room
	sscanf(arg1,"%d",&room);

	// invio il messaggio di START_ROOM
	send(sd,&opcode,sizeof(opcode),0);

	// invio il numero di room
	room = htonl(room);
	send(sd,&room,sizeof(room),0);

	// attendo il messaggio di OK
	printf("In attesa che gli altri giocatori entrino nella room %d...\n",htonl(room));
	recv(sd,&opcode,sizeof(opcode),0);
	if (opcode == OK){
		printf("Benvenuto nella room!\n");
		return 0;
	} else
		return 1;
}
#endif

// descrizione:
// funzione che stampa il menu in home page
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
// funzione che stampa il menu del gioco con tutti i comandi
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



