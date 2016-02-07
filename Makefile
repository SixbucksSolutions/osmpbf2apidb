CXX = ccache g++
CXXFLAGS = -Wall -Wextra -std=c++11 -Wpedantic -O2 -Ilib -Ilib/OsmFileParser/src

SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.cpp)
HPPS := $(wildcard $(SRCDIR)/*.hpp)
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=.o)))
BINDIR := bin

LD=g++
LD_LIBS=-lboost_system -lboost_filesystem -pthread -lz -lprotobuf-lite -losmpbf

OSMFILEPARSER_LIB := lib/OsmFileParser/lib/libosmfileparser.a

ASTYLE := astyle
ASTYLE_FLAGS := --options=astyle.cfg

DEPDIR := dep
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all :
	cd lib/OsmFileParser ; make
	make astyle
	make $(BINDIR)/osmpbf2apidb

$(BINDIR)/osmpbf2apidb : $(OBJS) $(OSMFILEPARSER_LIB) | $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^ $(LD_LIBS)
	
astyle : $(SRCS) $(HPPS)
	$(ASTYLE) $(ASTYLE_FLAGS) $(SRCS) $(HPPS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(DEPDIR)/%.d | $(OBJDIR) $(DEPDIR)
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

$(DEPDIR) :
	mkdir -p $(DEPDIR)

$(BINDIR) :
	mkdir -p $(BINDIR)

clean :
	cd lib/OsmFileParser; make clean
	rm -f $(OBJS) $(BINDIR)/osmpbf2apidb

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(notdir $(basename $(SRCS))))
