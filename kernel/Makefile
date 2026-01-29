SRC := $(shell find . -name '*.c')
OBJ := $(patsubst ./%.c,../build/kernel/%.o,$(SRC))

.PHONY: all

$(shell mkdir -p ../build/kernel)

all: ../build/kernel.o

../build/kernel/%.o: ./%.c
	gcc -m32 -masm=intel -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -nostdlib -I ../include -c -o $@ $<

../build/kernel.o: $(OBJ)
	ld -m elf_i386 -nostdlib -r -o ../build/kernel.o $(OBJ)
