# Makefile for simplescheduler and shell

CC = gcc
CFLAGS = -Wall -Wextra -g

# List the source files for simplescheduler and shell
SOURCES_SIMP = simplescheduler.c
SOURCES_SHELL = shell.c

# Define the names of the executables
TARGET_SIMP = simplescheduler
TARGET_SHELL = shell

# Default target to build both simplescheduler and shell
all: $(TARGET_SIMP) $(TARGET_SHELL)

# Build simplescheduler
$(TARGET_SIMP): $(SOURCES_SIMP)
	$(CC) $(CFLAGS) -o $@ $^

# Build shell
$(TARGET_SHELL): $(SOURCES_SHELL)
	$(CC) $(CFLAGS) -o $@ $^

# Clean the executables
clean:
	rm -f $(TARGET_SIMP) $(TARGET_SHELL)
