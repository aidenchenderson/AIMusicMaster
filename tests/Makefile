# ------------------------------ #
# build specifications for tests #
# ------------------------------ #

CXX = g++

# search paths
BACKEND_DIR = ../app/backend
BACKEND_SRC_DIR = $(BACKEND_DIR)/src
EXTERNAL_LIB_DIR = $(BACKEND_DIR)/lib

# include paths
INCLUDE_PATHS = -I$(BACKEND_DIR)/include -I$(EXTERNAL_LIB_DIR) -I/usr/include/catch2

# output directories
BUILD_DIR = build

# compiler and linker flags
CXXFLAGS = -std=c++23 -Wall -Wextra $(INCLUDE_PATHS) 
LDFLAGS = -lpthread -lasound -ldl -lm

# backend source files
BACKEND_SRCS = $(wildcard $(BACKEND_SRC_DIR)/*.cpp)

# test source files
TEST_SRCS = $(wildcard *.cpp)
TEST_BINS = $(patsubst %.cpp, $(BUILD_DIR)/%, $(TEST_SRCS))

# --------------------- #
# build rules for tests #
# --------------------- #

all: $(BUILD_DIR) $(TEST_BINS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# linking the test binaries
$(BUILD_DIR)/%: %.cpp $(BACKEND_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# -------------- #
# test utilities #
# -------------- #

.PHONY: all clean test

test: all
	@for bin in $(TEST_BINS); do \
		echo "Running $$bin..."; \
		./$$bin; \
	done

clean:
	rm -rf $(BUILD_DIR)