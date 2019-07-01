CC=gcc
CFLAGS=-Wall
PTHREAD=-lpthread

all: main.o buffer.o
	$(CC) $(CFLAGS) -o run main.o buffer.o $(PTHREAD)

main.o: main.c buffer.h
	$(CC) $(CFLAGS) -c -o main.o main.c $(PTHREAD)

buffer.o: buffer.c buffer.h
	$(CC) $(CFLAGS) -c -o buffer.o buffer.c $(PTHREAD)

clean:
	rm *.o
	rm run
