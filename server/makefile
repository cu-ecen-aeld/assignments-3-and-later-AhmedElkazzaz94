# Define variables
CC := gcc
CFLAGS := -Wall -Werror
TARGET := aesdsocket
SRC := aesdsocket.c
OBJ := $(SRC:.c=.o)

# Cross-compilation support
ifeq ($(CROSS_COMPILE),)
    CROSS_COMPILE := 
endif

CC := $(CROSS_COMPILE)gcc

# Default target to build the aesdsocket application
all: $(TARGET)

# Rule to build the aesdsocket application
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove the aesdsocket application and .o files
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: all clean
