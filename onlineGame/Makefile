CC = gcc
CFLAGS = -Wall

# Lista di tutti i file .c nella cartella lib-reti
LIB_SRCS = $(wildcard lib-reti/*.c)

# Lista di tutti i file oggetto generati dalla compilazione dei file nella cartella lib-reti
LIB_OBJS = $(patsubst %.c, %.o, $(LIB_SRCS))

all: server client room

server: server.o $(LIB_OBJS) game_server.o
	$(CC) $(CFLAGS) -o server server.o $(LIB_OBJS) -DSERVER lib-reti/game/game_server.o

client: client.o $(LIB_OBJS) game_client.o
	$(CC) $(CFLAGS) -o client client.o $(LIB_OBJS) lib-reti/game/game_client.o

room:	room.o $(LIB_OBJS) game_server.o
	$(CC) $(CFLAGS) -o room room.o $(LIB_OBJS) -DSERVER lib-reti/game/game_server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

game_server.o: 
	$(CC) $(CFLAGS) -c lib-reti/game/game.c -DSERVER -o lib-reti/game/game_server.o

game_client.o: 
	$(CC) $(CFLAGS) -c lib-reti/game/game.c -o lib-reti/game/game_client.o

room.o:
	$(CC) $(CFLAGS) -c roomProc.c -o room.o


# Regola generica per compilare i file oggetto dalla cartella lib-reti
lib-reti/%.o: lib-reti/%.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f server client room *.o lib-reti/*.o lib-reti/game/*.o
