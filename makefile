CC=gcc
CFLAGS=-I. -pthread


factorial : utils.o factorial.o
	$(CC) utils.o factorial.o -o factorial -pthread $(CFLAGS)
client : utils.o client.o
	$(CC) -o client utils.o client.o -pthread $(CFLAGS)
factorial.o : factorial.c
	$(CC) -c factorial.c -o factorial.o -pthread $(CFLAGS)
client.o : client.c
	$(CC) -o client.o -c client.c -pthread $(CFLAGS)
utils.o : utils.c utils.h
	$(CC) -o utils.o -c utils.c -pthread $(CFLAGS)