CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD
CPP_FILES := $(wildcard src/*.cpp) 
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 
AR := /usr/bin/ar
AR_FLAGS := -cvq

lib/libosmfileparser.a : $(OBJ_FILES)
	$(AR) $(AR_FLAGS) -o $@ $^ 

obj/%.o : src/%.cpp
	$(ASTYLE) $(ASTYLE_FLAGS) $< $(<:.cpp=.hpp)
	$(CC) $(CPPFLAGS) -c $< -o $@

clean :
	rm -f obj/*.o lib/*

-include $(OBJFILES:.o=.d)
