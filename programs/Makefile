
LIB := ../build/lib/libuser.a
SRC := $(shell find . -name '*.c')
BIN := $(patsubst ./%.c,../build/bin/%,$(SRC))

.PHONY: all

$(shell mkdir -p ../build/bin)

all: $(BIN)

$(LIB):
	cd ../lib && make

../build/bin/%: ./%.c $(LIB)
	gcc -m32 -masm=intel -ffreestanding -fno-pie -no-pie -fno-pic -fno-stack-protector -fno-builtin -nostdlib -I ../include -o $@ $^
