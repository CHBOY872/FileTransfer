CC = gcc
CFLAGSD = -Wall -g -ansi
CFLAGSR = -Wall -s -ansi

CXX = g++
CXXFLAGSD = -Wall -g -ansi
CXXFLAGSR = -Wall -s -ansi
SOURCEMODULES = FileTransferhandler.cpp Server.cpp
OBJECTMODULES = $(SOURCEMODULES:.cpp=.o)


%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGSD) ./$< -c

server: main.cpp $(OBJECTMODULES)
	$(CXX) $(CXXFLAGSR) ./$^ -o $@

client: client.c
	$(CC) $(CFLAGSR) ./$^ -o $@

create: 
	mkdir -p ../server
	mkdir -p ../client
	make server
	make client
	mv client ../client
	mv server ../server
	chmod -x ../client/client
	chmod -x ../server/server

clear:
	rm *.o

run:
	make create
	make clear