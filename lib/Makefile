SRC := $(shell find . -name '*.c')
OBJ := $(patsubst ./%.c,../build/lib/%.o,$(SRC))

.PHONY: all

$(shell mkdir -p ../build/lib)

all: ../build/lib.o

../build/lib/%.o: ./%.c
	gcc -m32 -masm=intel -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -c -I ../include -o $@ $<

../build/lib.o: $(OBJ)
	ld -m elf_i386 -r -o ../build/lib.o $(OBJ)
