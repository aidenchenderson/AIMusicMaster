# compiler
CXX = g++

# directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# compiler flags & linker flags
CXXFLAGS = -std=c++23 -Wall -I$(INCLUDE_DIR)
# link portaudio, real time, alsa, and pthreads
LDFLAGS = -lportaudio -lrt -lasound -pthread

# list of all source files and their corresponding object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# TODO: need to figure out a definitive name for the program
TARGET = $(BIN_DIR)/prog_name

all: $(TARGET)

# link the object files
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# compile the source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
# utility
.PHONY: clean rebuild run run-no-warn
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

rebuild: clean all

run: $(TARGET)
	$(TARGET)

run-no-warn: $(TARGET)
	$(TARGET) 2>/dev/null