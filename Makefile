CC=ccache g++
CPPFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -g -Ilib
LD=g++
LD_LIBS=-pthread -lz -lprotobuf-lite -losmpbf 
CPP_FILES := $(wildcard src/*.cpp) 
HPP_FILES := $(wildcard src/*.hpp)
SOURCE_FILES := $(HPP_FILES) $(CPP_FILES)
OBJ_DIR := obj
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
BIN_DIR := bin
OSMFILEPARSER_LIB := lib/OsmFileParser/lib/libosmfileparser.a
ASTYLE=astyle
ASTYLE_FLAGS=--options=astyle.cfg 
RM=rm
RM_FLAGS=-f

DEPDIR := dep
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.cpp = $(CC) $(DEPFLAGS) $(CPPFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all : 
	cd lib/OsmFileParser; make
	make astyle
	make $(BIN_DIR)/osmpbf2apidb

$(BIN_DIR)/osmpbf2apidb : $(OBJ_FILES) $(OSMFILEPARSER_LIB) | $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $^ $(OSMFILEPARSER_LIB) $(LD_LIBS)

$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)

astyle : $(SOURCE_FILES)
	$(ASTYLE) $(ASTYLE_FLAGS) $(SOURCE_FILES)  

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(DEPDIR)/%.d | $(OBJ_DIR)
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)

$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)

$(BIN_DIR) :
	mkdir -p $(BIN_DIR)


clean : 
	cd lib/OsmFileParser; make clean
	$(RM) $(RM_FLAGS) $(OBJ_FILES) $(BIN_DIR)/osmpbf2apidb

# What this magic is doing
#
#   Starting point: 
#			src/foo.cpp 	src/bar.cpp
#
#	Desired ending point:
#			dep/foo.d		dep/bar.d
#
# 	$(CPP_FILES:.cpp=.d)  : takes list of all .cpp files,
#			changes each ".cpp" extension to ".d"
#
#			Before : "src/foo.cpp src/bar.cpp"
#			After  : "src/foo.d   src/bar.d"
#			
#   $(notdir ... )   : Drop "src/" directory info before filename
#
#			Before : "src/foo.d   src/bar.d"
#			After  : "foo.d       bar.d"
#
# 	$(addprefix ...) : prepends "dep/" to filename, 
#
#			Before : "foo.d		  bar.d"
#			After  : "dep/foo.d   dep/bar.d"
# 
-include $(addprefix $(DEPDIR)/,$(notdir $(CPP_FILES:.cpp=.d)))
