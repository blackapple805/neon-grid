# Makefile for neon-life (C Game of Life)
#
# Usage:
#   make            # build ./neon-life (optimized)
#   make debug      # build with sanitizers + debug symbols
#   make run        # build and run
#   make clean      # remove build artifacts
#
# The warning flags are intentionally strict â€” in C the compiler is your first
# and best bug-catcher, so we ask it to complain about everything.

CC      := cc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

# Build everything in build/ so the source tree stays clean.
SRC_DIR := src
OBJ_DIR := build
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET  := neon-life

.PHONY: all run debug clean

all: $(TARGET)

# Link step.
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Compile step: each .c -> .o. The | $(OBJ_DIR) is an order-only prereq so the
# directory exists before we write into it.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: all
	./$(TARGET)

# Debug build: AddressSanitizer + UndefinedBehaviorSanitizer catch the memory
# bugs C is famous for (leaks, out-of-bounds, use-after-free) at runtime.
debug: CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined
debug: LDFLAGS := -fsanitize=address,undefined
debug: clean all

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

