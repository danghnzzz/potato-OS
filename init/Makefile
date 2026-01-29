LIB := ../build/lib/libuser.a
SRC := ./init.c
OBJ := ../build/init/init.o
LINKED_OBJ := ../build/init/init.r.o
RELOC_OBJ := ../build/init/init.user.o

.PHONY: all

$(shell mkdir -p ../build/init)

all: $(RELOC_OBJ)

$(OBJ): $(SRC)
	gcc -m32 -masm=intel -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -nostdlib -I ../include -c -o $@ $<

$(LIB):
	cd ../lib && make

$(LINKED_OBJ): $(OBJ) $(LIB)
	ld -m elf_i386 -nostdlib -r -o $@ $^

$(RELOC_OBJ): $(LINKED_OBJ)
	objcopy \
		--rename-section .text=.user_text,alloc,load,readonly,code,contents \
		--rename-section .rodata=.user_rodata,alloc,load,readonly,data,contents \
		--rename-section .data=.user_data,alloc,load,data,contents \
		--rename-section .bss=.user_bss,alloc,load,data \
		$< $@
