CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -MMD -Iinclude/
HPP_FILES := $(wildcard include/*.hpp) $(wildcard src/*.hpp)
CPP_FILES := $(wildcard src/*.cpp) 
SOURCE_FILES := $(HPP_FILES) $(CPP_FILES)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 
AR := ar
AR_FLAGS := -cvq

lib/libosmfileparser.a : $(OBJ_FILES)
	make astyle
	$(AR) $(AR_FLAGS) -o $@ $(OBJ_FILES)

astyle : $(SOURCE_FILES)
	$(ASTYLE) $(ASTYLE_FLAGS) $^

obj/%.o : src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean :
	rm -f $(OBJ_FILES) lib/libosmfileparser.a

-include $(OBJFILES:.o=.d)
