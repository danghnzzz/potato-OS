SRC := $(shell find . -name '*.c')
OBJ := $(patsubst ./%.c,../build/lib/%.o,$(SRC))
LIB := ../build/lib/libuser.a

.PHONY: all clean

$(shell mkdir -p ../build/lib)

all: $(LIB)

../build/lib/%.o: ./%.c
	gcc -m32 -masm=intel -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -c -I ../include -o $@ $<

$(LIB): $(OBJ)
	ar rcs $@ $(OBJ)
