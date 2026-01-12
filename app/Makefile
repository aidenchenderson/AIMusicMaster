# -------------------- #
# build specifications #
# -------------------- #

CXX = g++

# search paths
BACKEND_DIR = backend
FRONTEND_DIR = frontend
EXTERNAL_LIB_DIR = $(BACKEND_DIR)/lib
INCLUDE_PATHS = -I. -I$(BACKEND_DIR)/include -I$(FRONTEND_DIR)/include -I$(EXTERNAL_LIB_DIR)

# output directories
BUILD_DIR = build
BIN_DIR = bin

# compiler flags
CXXFLAGS = -std=c++23 -Wall $(INCLUDE_PATHS)
# linker flags
LDFLAGS = -pthread -lfftw3 -ldl -lm

# source files
SRCS = main.cpp $(wildcard $(BACKEND_DIR)/src/*.cpp) $(wildcard $(FRONTEND_DIR)/src/*.cpp)

# object file mapping
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

TARGET = $(BIN_DIR)/guitar_genre_guru

# ----------- #
# build rules #
# ----------- #

all: $(TARGET)

# linking the final binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# compiling c++ files
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --------- #
# utilities #
# --------- #

.PHONY: clean rebuild run
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)