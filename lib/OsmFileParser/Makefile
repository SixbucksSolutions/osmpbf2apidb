#CXX=ccache g++
#CXXFLAGS=-Wall -Wextra -std=c++11 -Wpedantic -O -Iinclude/
#HPP_FILES := $(wildcard include/*.hpp) $(wildcard src/*.hpp)
#CPP_FILES := $(wildcard src/*.cpp)
#SOURCE_FILES := $(HPP_FILES) $(CPP_FILES)
#OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
#ASTYLE=astyle
#ASTYLE_FLAGS=--options=astyle.cfg
#AR := ar
#AR_FLAGS := -cvq

CXX = ccache g++
CXXFLAGS = -Wall -Wextra -std=c++11 -Wpedantic -O -Iinclude/

SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.cpp)
LIBDIR := lib
LIB := $(LIBDIR)/libosmfileparser.a
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=.o)))

AR := ar
ARFLAGS := rcs

DEPDIR := dep
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

$(LIB) : $(OBJS)
	$(AR) $(ARFLAGS) -o $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(DEPDIR)/%.d
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)

clean :
	rm -f $(OBJS) lib/libosmfileparser.a

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(notdir $(basename $(SRCS))))
