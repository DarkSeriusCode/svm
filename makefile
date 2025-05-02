CC = gcc
CC_FLAGS = -Wall -Wextra -I.

DEBUG ?= true
ifeq ($(DEBUG), false)
	CC_FLAGS += -O3
else
	CC_FLAGS += -g -O0
endif

SVM_DIR = vm
SASM_DIR = assembler
COMMON_DIR = common

all: assembler vm

$(shell mkdir -p build build/obj build/obj/sasm build/obj/svm build/obj/common)

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

HEADERS += $(wildcard $(SVM_DIR)/*.h)
SVM_OBJS = $(patsubst $(SVM_DIR)/%.c, build/obj/svm/%.o, $(wildcard $(SVM_DIR)/*.c))

build/obj/svm/%.o: $(SVM_DIR)/%.c $(HEADERS)
	$(CC) -c $< $(CC_FLAGS) -o $@

build/svm: $(SVM_OBJS) build/common.a
	$(CC) $(CC_FLAGS) $^ -o $@

.PHONY: vm
vm: build/svm

# -------------------------------------------------------------------------------------------------


.PHONY: clean_dumps
clean_dumps:
	rm -rf *.dump

.PHONY: clean
clean: clean_dumps
	rm -rf build
