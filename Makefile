CC=g++
CPPFLAGS=-W -Wall -Wextra -O -std=c++11
LD=g++

bin/osmpbf2pgsql : obj/osmpbf2pgsql.o obj/PbfReader.o 
	$(LD) $(LDFLAGS) obj/osmpbf2pgsql.o obj/PbfReader.o \
		-pthread -lz -lprotobuf-lite -losmpbf -o bin/osmpbf2pgsql
obj/osmpbf2pgsql.o : src/osmpbf2pgsql.hpp src/osmpbf2pgsql.cpp
	$(CC) $(CPPFLAGS) -c src/osmpbf2pgsql.cpp -o obj/osmpbf2pgsql.o
obj/PbfReader.o : src/PbfReader.hpp src/PbfReader.cpp
	$(CC) $(CPPFLAGS) -c src/PbfReader.cpp -o obj/PbfReader.o
clean :
	rm -f obj/* bin/*
