OBJ := \
	./build/boot.o \
	./build/kernel.o
TARGET := potatoOS

.PHONY: all build_iso clean

$(shell mkdir -p ./build)

all: build_iso

./build/boot.o:
	nasm -f elf32 -o ./build/boot.o ./boot/boot.asm

./build/kernel.o:
	cd ./kernel && make

./build/$(TARGET).elf: $(OBJ)
	ld -m elf_i386 -T ./linker.ld -o ./build/$(TARGET).elf $(OBJ)

build_iso: ./build/$(TARGET).elf
	mkdir -p ./build/$(TARGET)
	mkdir -p ./build/$(TARGET)/boot
	cp -r ./grub ./build/$(TARGET)/boot
	cp ./build/$(TARGET).elf ./build/$(TARGET)/boot/$(TARGET).elf
	grub-mkrescue -o $(TARGET).iso ./build/$(TARGET)

clean:
	rm -rf ./build
