CC = g++
CFLAGS = -Wall -std=c++11

all: servidor cliente

servidor: server.o
	$(CC) $(CFLAGS) -o servidor server.o -lpthread

cliente: client.o
	$(CC) $(CFLAGS) -o cliente client.o

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

clean:
	rm -f *.o servidor cliente
