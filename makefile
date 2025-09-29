CC := gcc
CFLAGS = -g -Wall -Wextra -std=gnu99 -Iinclude -I. -MMD -MP -DHAVE_STDINT_H=1 -Wno-unused-parameter -Wno-unused-function -Wno-format-truncation
LFLAGS := -lcurl

# Find all C files in source/ AND lib/jansson/
# This ensures all Jansson .c files are compiled alongside your app files.
SRC := $(shell find source -name '*.c') $(shell find lib/jansson -name '*.c')

OBJ := $(patsubst source/%.c,build/%.o,$(filter source/%.c,$(SRC)))
# Also add object files for lib/jansson files
OBJ += $(patsubst lib/jansson/%.c,build/jansson-%.o,$(filter lib/jansson/%.c,$(SRC)))

BUILD_DIR := build
DEP := $(OBJ:.o=.d)
BIN := build/app

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Rule for building your application's object files (from source/ main.c, etc.)
build/%.o: source/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for building Jansson's object files (from lib/jansson/*.c)
# We use a unique pattern (jansson-%.o) to avoid naming conflicts if your source files share names.
build/jansson-%.o: lib/jansson/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Original lines:
# build/%.o: source/%.c
# 	@mkdir -p $(@D)
# 	$(CC) $(CFLAGS) -c $< -o $@


run: $(BIN)
	./$(BIN)

clean:
	$(RM) -rf $(BUILD_DIR)

# Include dependency files if they exist
-include $(DEP)
