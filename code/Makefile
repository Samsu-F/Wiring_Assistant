# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror -O3

# Target executable
TARGET = build/wiring_assistant

# Source files
SRCS = a_star.c coordinate_struct.c graph.c pqueue.c wiring_assistant.c

# Object files
OBJS = $(addprefix build/,$(SRCS:.c=.o))

# Header files
HDRS = a_star.h coordinate_struct.h graph.h pqueue.h

# Default target
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile C source files into object files
build/%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(shell mkdir -p build)

# Clean up the build files
clean:
	rm -rf build

# Phony targets
.PHONY: all clean
