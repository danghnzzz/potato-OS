OBJ := \
	./build/boot.o \
	./build/kernel.o \
	./build/init/init.user.o
TARGET := potatoOS
IMG_SIZE_MiB := 64

.PHONY: all build_iso build_img run_img clean programs

$(shell mkdir -p ./build)
$(shell mkdir -p ./mnt)

all: build_iso build_img

./build/boot.o:
	nasm -f elf32 -o ./build/boot.o ./boot/boot.asm

./build/kernel.o:
	cd ./kernel && make

./build/init/init.user.o:
	cd ./init && make

./build/$(TARGET).elf: $(OBJ)
	ld -m elf_i386 -T ./linker.ld -o ./build/$(TARGET).elf $(OBJ)

programs:
	cd ./programs && make

build_iso: ./build/$(TARGET).elf
	mkdir -p ./build/$(TARGET)
	mkdir -p ./build/$(TARGET)/boot
	cp -r ./grub ./build/$(TARGET)/boot
	cp ./build/$(TARGET).elf ./build/$(TARGET)/boot/$(TARGET).elf
	grub-mkrescue -o $(TARGET).iso ./build/$(TARGET)

build_img: ./build/$(TARGET).elf programs
	dd if=/dev/zero of=$(TARGET).img bs=1M count=$(IMG_SIZE_MiB)
	parted -s $(TARGET).img \
		mklabel msdos \
    	mkpart primary 1MiB 100% \
    	set 1 boot on
	LOOP=$$(sudo losetup --find --partscan --show $(TARGET).img); \
		sudo mkfs.minix -1 -n 30 "$${LOOP}p1"; \
		sudo mount -t minix "$${LOOP}p1" ./mnt/; \
		sudo mkdir -p ./mnt/boot/grub; \
		sudo cp ./build/$(TARGET).elf ./mnt/boot/$(TARGET).elf; \
		sudo cp ./grub/grub.cfg ./mnt/boot/grub/grub.cfg; \
		sudo mkdir -p ./mnt/bin; \
		sudo cp -r ./build/bin/. ./mnt/bin/; \
		sudo grub-install --target=i386-pc --boot-directory=./mnt/boot --modules="part_msdos minix" --no-floppy "$$LOOP"; \
		sync; \
		sudo umount ./mnt/; \
		sudo losetup -d "$$LOOP"

run_iso: $(TARGET).iso
	qemu-system-i386 -m 64M -boot d -cdrom $(TARGET).iso

run_img: $(TARGET).iso
	qemu-system-i386 -m 64M -boot c -drive file=$(TARGET).iso,format=raw,if=ide,index=0,media=disk

clean:
	rm -rf ./build
	rm -rf ./mnt
	rm -f $(TARGET).iso 
	rm -f $(TARGET).img
