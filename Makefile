OBJ := \
	./build/boot.o \
	./build/kernel.o \
	./build/init/init.user.o
TARGET := potatoOS
IMG_SIZE := 64M

.PHONY: all build_iso build_img run_img clean

$(shell mkdir -p ./build)

all: build_iso build_img

./build/boot.o:
	nasm -f elf32 -o ./build/boot.o ./boot/boot.asm

./build/kernel.o:
	cd ./kernel && make

./build/init/init.user.o:
	cd ./init && make

./build/$(TARGET).elf: $(OBJ)
	ld -m elf_i386 -T ./linker.ld -o ./build/$(TARGET).elf $(OBJ)

build_iso: ./build/$(TARGET).elf
	mkdir -p ./build/$(TARGET)
	mkdir -p ./build/$(TARGET)/boot
	cp -r ./grub ./build/$(TARGET)/boot
	cp ./build/$(TARGET).elf ./build/$(TARGET)/boot/$(TARGET).elf
	grub-mkrescue -o $(TARGET).iso ./build/$(TARGET)

build_img: build_iso
	qemu-img create -f raw $(TARGET).img $(IMG_SIZE)
	dd if=$(TARGET).iso of=$(TARGET).img conv=notrunc

run_iso: build_iso
	qemu-system-i386 -m 64M -boot d -cdrom $(TARGET).iso

run_img: build_img
	qemu-system-i386 -m 64M -boot c -drive file=$(TARGET).img,format=raw,if=ide,index=0,media=disk

clean:
	rm -rf ./build
	rm -f $(TARGET).iso $(TARGET).img
