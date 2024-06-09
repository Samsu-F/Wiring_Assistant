# Compiler
CC = gcc

# Compiler flags
CFLAGS = -O3 -Werror -W -Wall -Wextra -Wpedantic -Wshadow -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wswitch-default -Wcast-align=strict -Wbad-function-cast -Wstrict-overflow=4 -Winline -Wundef -Wnested-externs -Wunreachable-code -Wlogical-op -Wfloat-equal -Wredundant-decls -Wold-style-definition -Wwrite-strings -Wformat=2 -Wno-error=unused-parameter -Wno-error=inline -Wno-error=unreachable-code -fanalyzer

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
	@echo Linking $(TARGET)
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile C source files into object files
build/%.o: %.c $(HDRS)
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(shell mkdir -p build)

# Clean up the build files
clean:
	rm -rf build

# Phony targets
.PHONY: all clean
