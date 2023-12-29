CC = gcc
CFLAGS = -Wall

# Lista di tutti i file .c nella cartella lib-reti
LIB_SRCS = $(wildcard lib-reti/*.c)

# Lista di tutti i file oggetto generati dalla compilazione dei file nella cartella lib-reti
LIB_OBJS = $(patsubst %.c, %.o, $(LIB_SRCS))

all: server client room client1

server: server.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o server server.o $(LIB_OBJS)

client: client.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o client client.o $(LIB_OBJS)

client1: client.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o client1 client1.c $(LIB_OBJS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

# Regola generica per compilare i file oggetto dalla cartella lib-reti
lib-reti/%.o: lib-reti/%.c
	$(CC) $(CFLAGS) -c $< -o $@

room:
	$(CC) $(CFLAGS) -o room roomProc.c

clean:
	rm -f server client room *.o lib-reti/*.o

