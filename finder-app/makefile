# Define variables
CC := gcc
CFLAGS := -Wall -Werror
TARGET := writer
SRC := writer.c
OBJ := $(SRC:.c=.o)

# Cross-compilation support
ifeq ($(CROSS_COMPILE),)
    CROSS_COMPILE := 
endif

CC := $(CROSS_COMPILE)gcc

# Default target to build the writer application
all: $(TARGET)

# Rule to build the writer application
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove the writer application and .o files
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: all clean