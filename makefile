CC = gcc
CC_FLAGS = -Wall -Wextra -Wno-alloc-size -I.
DEV_FLAGS = -fPIC -shared

DEBUG ?= true
ifeq ($(DEBUG), false)
	CC_FLAGS += -O3
else
	CC_FLAGS += -g -O0
endif

VM_DIR = vm
VM_BIN = build/svm
VM_OBJ_DIR = build/obj/vm

ASM_DIR = assembler
ASM_BIN = build/sasm
ASM_OBJ_DIR = build/obj/asm

COMMON_DIR = common
COMMON_BIN = build/common.a
COMMON_OBJ_DIR = build/obj/common

DEV_DIR = dev
DEV_BIN_DIR = build

EXAPMLES_DIR = examples
EXAPMLES_BIN_DIR = build/examples

all: assembler vm dev examples

$(shell mkdir -p build build/obj $(ASM_OBJ_DIR) $(VM_OBJ_DIR) $(COMMON_OBJ_DIR) $(DEV_BIN_DIR) \
	$(EXAPMLES_BIN_DIR))

HEADERS=

# -------------------------------------------------------------------------------------------------
# COMMON

HEADERS += $(wildcard $(COMMON_DIR)/*.h)
COMMON_OBJS = $(patsubst $(COMMON_DIR)/%.c, $(COMMON_OBJ_DIR)/%.o, $(wildcard $(COMMON_DIR)/*.c))

$(COMMON_OBJ_DIR)/%.o: $(COMMON_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

$(COMMON_BIN): $(COMMON_OBJS)
	$(AR) rcs $@ $^

# -------------------------------------------------------------------------------------------------
# SASM

HEADERS += $(wildcard $(ASM_DIR)/*.h)
ASM_OBJS = $(patsubst $(ASM_DIR)/%.c, $(ASM_OBJ_DIR)/%.o, $(wildcard $(ASM_DIR)/*.c))

$(ASM_OBJ_DIR)/%.o: $(ASM_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

$(ASM_BIN): $(ASM_OBJS) $(COMMON_BIN)
	$(CC) $(CC_FLAGS) $^ -o $@

.PHONY: assembler
assembler: $(ASM_BIN)

# -------------------------------------------------------------------------------------------------
# SVM

HEADERS += $(wildcard $(VM_DIR)/*.h)
VM_OBJS = $(patsubst $(VM_DIR)/%.c, $(VM_OBJ_DIR)/%.o, $(wildcard $(VM_DIR)/*.c))

$(VM_OBJ_DIR)/%.o: $(VM_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

$(VM_BIN): $(VM_OBJS) $(COMMON_BIN)
	$(CC) $(CC_FLAGS) $^ -o $@

.PHONY: vm
vm: $(VM_BIN)

# -------------------------------------------------------------------------------------------------
# DEVICES

DEVS = $(patsubst $(DEV_DIR)/%.c, $(DEV_BIN_DIR)/%.so, $(wildcard $(DEV_DIR)/*.c))

$(DEV_BIN_DIR)/console.so: $(DEV_DIR)/console.c $(HEADERS)
	$(CC) $(DEV_FLAGS) $(CC_FLAGS) $< -o $@

.PHONY: dev
dev: $(DEVS)

# -------------------------------------------------------------------------------------------------
# EXAMPLES

EXAMPLES = $(patsubst $(EXAPMLES_DIR)/%.asm, $(EXAPMLES_BIN_DIR)/%, $(wildcard $(EXAPMLES_DIR)/*.asm))

$(EXAPMLES_BIN_DIR)/%: $(EXAPMLES_DIR)/%.asm $(ASM_BIN)
	$(ASM_BIN) $< -o $@

.PHONY: examples
examples: $(EXAMPLES)

# -------------------------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf build

.PHONY: clean_dump
clean_dump:
	rm -rf *.dump
