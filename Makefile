CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD -Isrc/OsmFileParser/src
LD=ccache g++ 
LDFLAGS=-Lsrc/OsmFileParser/lib
LD_LIBS=-pthread -lz -lprotobuf-lite -losmpbf -llibosmfileparser
CPP_FILES := $(wildcard src/*.cpp) 
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 

bin/osmpbf2apidb : $(OBJ_FILES) src/OsmFileParser/lib/libosmfileparser.a
	$(LD) $(LDFLAGS) -o $@ $^ $(LD_LIBS)

obj/%.o : src/%.cpp
	$(ASTYLE) $(ASTYLE_FLAGS) $< $(<:.cpp=.hpp)
	$(CC) $(CPPFLAGS) -c $< -o $@


src/OsmFileParser/lib/libosmfileparser.a :
	cd src/OsmFileParser; make

clean :
	rm -f obj/*.o bin/* src/OsmFileParser/lib/*

-include $(OBJFILES:.o=.d)
