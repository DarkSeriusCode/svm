CC = gcc
CC_FLAGS = -Wall -Wextra -I.

RELEASE ?= false
ifeq ($(RELEASE), false)
	CC_FLAGS += -g -O0
else
	CC_FLAGS += -O3
endif

SASM_DIR = assembler
COMMON_DIR = common

all: assembler

$(shell mkdir -p build build/obj build/obj/sasm build/obj/common)

HEADERS=

# -------------------------------------------------------------------------------------------------

HEADERS += $(wildcard $(COMMON_DIR)/*.h)
COMMON_OBJS = $(patsubst $(COMMON_DIR)/%.c, build/obj/common/%.o, $(wildcard $(COMMON_DIR)/*.c))

build/obj/common/%.o: $(COMMON_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

build/common.a: $(COMMON_OBJS)
	$(AR) rcs $@ $^

# -------------------------------------------------------------------------------------------------

HEADERS += $(wildcard $(SASM_DIR)/*.h)
SASM_OBJS = $(patsubst $(SASM_DIR)/%.c, build/obj/sasm/%.o, $(wildcard $(SASM_DIR)/*.c))

build/obj/sasm/%.o: $(SASM_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

build/sasm: $(SASM_OBJS) build/common.a
	$(CC) $(CC_FLAGS) $^ -o $@

.PHONY: assembler
assembler: build/sasm

# -------------------------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf build
