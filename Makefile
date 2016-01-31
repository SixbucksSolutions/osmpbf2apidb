CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD -Ilib
LD=g++
LD_LIBS=-pthread -lz -lprotobuf-lite -losmpbf 
CPP_FILES := $(wildcard src/*.cpp) 
HPP_FILES := $(wildcard src/*.hpp)
SOURCE_FILES := $(HPP_FILES) $(CPP_FILES)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
OSMFILEPARSER_LIB := lib/OsmFileParser/lib/libosmfileparser.a
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 
RM=rm
RM_FLAGS=-f

all : 
	cd lib/OsmFileParser; make
	make bin/osmpbf2apidb

bin/osmpbf2apidb : $(OBJ_FILES) 
	$(ASTYLE) $(ASTYLE_FLAGS) $(SOURCE_FILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(OSMFILEPARSER_LIB) $(LD_LIBS)

obj/%.o : src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@ 

clean : 
	cd lib/OsmFileParser; make clean
	$(RM) $(RM_FLAGS) $(OBJ_FILES) bin/osmpbf2apidb

-include $(OBJFILES:.o=.d)
