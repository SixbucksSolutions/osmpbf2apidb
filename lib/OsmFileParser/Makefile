CXX = ccache g++
CXXFLAGS = -Wall -Wextra -std=c++11 -Wpedantic -O2 -Iinclude -Isrc

SRCDIR := src
INCLUDEDIR := include
SRCS := $(wildcard $(SRCDIR)/*.cpp)
HPPS := $(wildcard $(INCLUDEDIR)/*.hpp) $(wildcard $(SRCDIR)/*.hpp)
LIBDIR := lib
LIB := $(LIBDIR)/libosmfileparser.a
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=.o)))

AR := ar
ARFLAGS := rcs

ASTYLE := astyle
ASTYLE_FLAGS := --options=../../astyle.cfg

DEPDIR := dep
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all :
	make astyle
	make $(LIB)

astyle : $(SRCS) $(HPPS)
	$(ASTYLE) $(ASTYLE_FLAGS) $(SRCS) $(HPPS)

$(LIB) : $(OBJS) | $(LIBDIR)
	$(AR) $(ARFLAGS) -o $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(DEPDIR)/%.d | $(OBJDIR) $(DEPDIR)
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)


$(OBJDIR) :
	mkdir -p $(OBJDIR)

$(LIBDIR) :
	mkdir -p $(LIBDIR)

$(DEPDIR) :
	mkdir -p $(DEPDIR)

clean :
	rm -f $(OBJS) lib/libosmfileparser.a

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(notdir $(basename $(SRCS))))
