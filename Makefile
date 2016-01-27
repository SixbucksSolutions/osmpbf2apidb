CC=g++
CPPFLAGS=-W -Wall -Wextra
LD=g++

bin/osmpbf2pgsql : obj/osmpbf2pgsql.o obj/PbfReader.o
	$(LD) obj/osmpbf2pgsql.o obj/PbfReader.o -o bin/osmpbf2pgsql
obj/osmpbf2pgsql.o : src/osmpbf2pgsql.hpp src/osmpbf2pgsql.cpp
	$(CC) $(CPPFLAGS) -c src/osmpbf2pgsql.cpp -o obj/osmpbf2pgsql.o
obj/PbfReader.o : src/PbfReader.hpp src/PbfReader.cpp
	$(CC) $(CPPFLASG) -c src/PbfReader.cpp -o obj/PbfReader.o
