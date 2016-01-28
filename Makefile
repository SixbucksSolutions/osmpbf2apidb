CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD
LD=ccache g++
LD_LIBS=-pthread -lz -lprotobuf-lite -losmpbf -o bin/osmpbf2pgsql
CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

bin/osmpbf2pgsql : $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(LD_LIBS)

obj/%.o : src/%.cpp 
	$(CC) $(CPPFLAGS) -c -o $@ $<

clean :
	rm -f obj/* bin/*

-include $(OBJFILES:.o=.d)
