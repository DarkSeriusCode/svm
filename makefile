CC = ccache clang
CC_FLAGS = -Wall -Wextra

DEBUG ?= true
ifeq ($(DEBUG), true)
	CC_FLAGS += -g -O0
endif

SASM_DIR = assembler

all:
	@echo "Targets: assembler"

$(shell mkdir -p build build/obj)

# -------------------------------------------------------------------------------------------------

SASM_HDRS = $(wildcard $(SASM_DIR)/*.h)
SASM_OBJS = $(patsubst $(SASM_DIR)/%.c, build/obj/sasm/%.o, $(wildcard $(SASM_DIR)/*.c))

build/obj/sasm/%.o: $(SASM_DIR)/%.c $(SASM_HDRS)
	$(CC) -c $< $(CC_FLAGS) -o $@

build/sasm: $(SASM_OBJS)
	$(CC) $(CC_FLAGS) $^ -o $@

.PHONY: assembler
assembler: $(shell mkdir -p build/obj/sasm) build/sasm

# -------------------------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf build
