#Compiler to use
CXX = g++

# Flags for g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g

#Output file
OUTPUT = emu

#Code sources
SRCDIR = src
CODE_SOURCES = $(wildcard $(SRCDIR)/*.cpp)

#object files
OBJDIR = obj
OBJECTS = $(CODE_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(OUTPUT)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJECTS) $(OUTPUT)