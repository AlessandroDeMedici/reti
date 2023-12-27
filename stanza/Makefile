CC = -gcc
FLAGS = -Wall -pthread

all: main.o game.o
	$(CC) -o main main.o game.h game.o $(FLAGS)
main.o:
	$(CC) -c main.c $(FLAGS)
game.o:
	$(CC) -c game.c $(FLAGS)

