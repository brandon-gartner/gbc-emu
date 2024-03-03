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
	rm -f $(OBJECTS) $(OUTPUT) tests/joypad_test tests/mbc_test tests/timing_test tests/frame_sync_test tests/cgb_init_test tests/cgb_banking_test tests/cgb_palette_test tests/cgb_render_test tests/cgb_speed_test tests/cgb_dma_test tests/cgb_integration_test

test: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -Isrc tests/joypad_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/joypad_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/mbc_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/mbc_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/timing_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/timing_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/frame_sync_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/frame_sync_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_init_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_init_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_banking_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_banking_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_palette_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_palette_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_render_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_render_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_speed_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_speed_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_dma_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_dma_test $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -Isrc tests/cgb_integration_test.cpp $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) -o tests/cgb_integration_test $(LDFLAGS)
	./tests/joypad_test
	./tests/mbc_test
	./tests/timing_test
	./tests/frame_sync_test
	./tests/cgb_init_test
	./tests/cgb_banking_test
	./tests/cgb_palette_test
	./tests/cgb_render_test
	./tests/cgb_speed_test
	./tests/cgb_dma_test
	./tests/cgb_integration_test

.PHONY: all clean test