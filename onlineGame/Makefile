CC = gcc
CFLAGS = -Wall

# Lista di tutti i file .c nella cartella lib-reti
LIB_SRCS = $(wildcard lib-reti/*.c)

# Lista di tutti i file oggetto generati dalla compilazione dei file nella cartella lib-reti
LIB_OBJS = $(patsubst lib-reti/%.c, lib-reti/client_%.o, $(LIB_SRCS)) $(patsubst lib-reti/%.c, lib-reti/server_%.o, $(LIB_SRCS))

all: server client room

server: server.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o server server.o $(patsubst lib-reti/%.c, lib-reti/server_%.o, $(LIB_SRCS)) -DSERVER -pthread

client: client.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o client client.o $(patsubst lib-reti/%.c, lib-reti/client_%.o, $(LIB_SRCS))

room: room.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o room room.o $(patsubst lib-reti/%.c, lib-reti/server_%.o, $(LIB_SRCS)) -DSERVER

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -DSERVER -pthread

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

room.o:
	$(CC) $(CFLAGS) -c roomProc.c -o room.o -DSERVER

# Regola generica per compilare i file oggetto dalla cartella lib-reti
lib-reti/client_%.o: lib-reti/%.c
	$(CC) $(CFLAGS) -c $< -o $@

lib-reti/server_%.o: lib-reti/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -DSERVER

clean:
	rm -f server client room *.o lib-reti/*.o

