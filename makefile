#Compiler to use
CXX = g++

# Flags for g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

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
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(OUTPUT) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJECTS) $(OUTPUT) tests/joypad_test tests/mbc_test tests/timing_test

test: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -Isrc tests/joypad_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/joypad_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/mbc_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/mbc_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/timing_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/timing_test $(LDFLAGS)
	./tests/joypad_test
	./tests/mbc_test
	./tests/timing_test

.PHONY: all clean test