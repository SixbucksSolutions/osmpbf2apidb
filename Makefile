CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD
LD=ccache g++ 
LDFLAGS=
LD_LIBS=-pthread -lz -lprotobuf-lite -losmpbf 
CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 

bin/osmpbf2apidb : $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(LD_LIBS)

obj/%.o : src/%.cpp 
	$(ASTYLE) $(ASTYLE_FLAGS) $< $(<:.cpp=.hpp)
	$(CC) $(CPPFLAGS) -c $< -o $@ 

clean :
	rm -f obj/* bin/*

-include $(OBJFILES:.o=.d)
