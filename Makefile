# Makefile for DSC Encoder
CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -I. -Isrc
CXXFLAGS = -Wall -O2 -I. -Isrc
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
C_SOURCES = $(SRC_DIR)/cmd_parse.c \
            $(SRC_DIR)/dpx.c \
            $(SRC_DIR)/dsc_utils.c \
            $(SRC_DIR)/encoder_main.c \
            $(SRC_DIR)/fifo.c \
            $(SRC_DIR)/logging.c \
            $(SRC_DIR)/multiplex.c \
            $(SRC_DIR)/psnr.c \
            $(SRC_DIR)/utl.c

# C++ sources (including C files with C++ dependencies)
CPP_SOURCES = $(SRC_DIR)/dsc_codec.c \
              $(SRC_DIR)/rgb2ycocg.c

# Object files
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
CPP_C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(CPP_SOURCES)))
CPP_CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(CPP_SOURCES)))
OBJECTS = $(C_OBJECTS) $(CPP_C_OBJECTS) $(CPP_CPP_OBJECTS)

# Target executable
TARGET = $(BUILD_DIR)/dsc_encoder

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile .c files that need C++ compilation (from CPP_SOURCES)
$(BUILD_DIR)/dsc_codec.o: $(SRC_DIR)/dsc_codec.c
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/rgb2ycocg.o: $(SRC_DIR)/rgb2ycocg.c
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild
