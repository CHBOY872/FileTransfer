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

createclient: 
	mkdir -p ../client
	make client
	mv client ../client
	chmod -x ../client/client

createserver:
	mkdir -p ../server
	make server
	mv server ../server
	chmod -x ../server/server

clear:
	rm *.o

runall:
	make createclient
	make createserver
	make clear

runclient:
	make createclient
	make clear

runserver:
	make createserver
	make clear
