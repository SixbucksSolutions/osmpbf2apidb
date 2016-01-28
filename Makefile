CC=g++
CPPFLAGS=-W -Wall -Wextra -O -std=c++11
LD=g++

bin/osmpbf2pgsql : obj/osmpbf2pgsql.o obj/PbfReader.o obj/fileformat.o obj/osmformat.o
	$(LD) $(LDFLAGS) obj/osmpbf2pgsql.o obj/PbfReader.o obj/fileformat.o obj/osmformat.o \
		-pthread -lz -lprotobuf-lite -o bin/osmpbf2pgsql
obj/osmpbf2pgsql.o : src/osmpbf2pgsql.hpp src/osmpbf2pgsql.cpp
	$(CC) $(CPPFLAGS) -c src/osmpbf2pgsql.cpp -o obj/osmpbf2pgsql.o
obj/PbfReader.o : src/PbfReader.hpp src/PbfReader.cpp
	$(CC) $(CPPFLAGS) -c src/PbfReader.cpp -o obj/PbfReader.o
obj/fileformat.o : src/fileformat.pb.h src/fileformat.pb.cc
	$(CC) $(CPPFLAGS) -c src/fileformat.pb.cc -o obj/fileformat.o
obj/osmformat.o : src/osmformat.pb.h src/osmformat.pb.cc
	$(CC) $(CPPFLAGS) -c src/osmformat.pb.cc -o obj/osmformat.o
clean :
	rm -f obj/* bin/*
