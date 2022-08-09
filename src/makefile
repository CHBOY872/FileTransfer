CC=gcc
CFLAGS=-Wall -ansi

CPP=g++
CPPFLAGS=-Wall -ansi
SRCMODULES=FileHandler.cpp Server.cpp
OBJMODULED=$(SRCMODULES:.cpp=.o)

%.o: %.cpp %.hpp
	$(CPP) $(CPPFLAGS) -g $< -c -o $@

server: main.cpp $(SRCMODULES)
	$(CPP) $(CPPFLAGS) -g $^ -o $@

client: client.c
	$(CC) $(CFLAGS) -g $< -o $@

all:
	make server
	make client

runcall:
	mkdir -p ../server
	mkdir -p ../client
	make all
	mv server ../server
	mv client ../client
	chmod ugo+x ../server
	chmod ugo+x ../client
	echo "afjkdhfkjadhfjkadhfjahfkajdfhakdhfjadfhajkdfhadfkjhadfkjadhfkjadhfjahdfkjadfjafhajkdfakjdf" > ../server/main.txt
	rm *.o