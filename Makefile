# Config
LLVM_VER?=9

## Source Files
C_SRC := $(wildcard *.c)
C_IR  := $(patsubst %.c, %.ll, ${C_SRC})
C_ASM := $(patsubst %.ll, %.s, ${C_IR})

## Compiler Binaries
CLANG := $(shell which clang-${LLVM_VER})
CLANG_FORMT := $(shell which clang-format-${LLVM_VER})
LLC := $(shell which llc-${LLVM_VER})

## Targets

# All, Default Target
.PHONY: all
all:
	@echo ---- Formatting Source Code ----
	${MAKE} -C . format
	@echo ---- Compiling Binary ----
	${MAKE} -C . super4

# Output Binary
super4: ${C_ASM}
	${CLANG} -I/usr/include -lftdi -o ${@} ${^}

# Format C
.PHONY: format
format: ${C_SRC}
	clang-format-${LLVM_VER} -i ${^}

# IR Output target
.PHONY: ir
ir: ${C_IR}

%.ll: %.c
	${CLANG} -S -emit-llvm -o ${@} ${<}

# Assembly Output Target
.PHONY: asm
asm: ${C_ASM}

%.s: %.ll
	${LLC} -o ${@} ${<}

# Clean
.PHONY: clean
clean:
	git clean -xfd
