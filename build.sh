#!/bin/bash
set -e

rm -rf build goobos.iso goobos.img
mkdir -p build/iso/boot/grub

CFLAGS="-m32 -O2 -ffreestanding -nostdlib -static -no-pie"


nasm -f elf32 src/asm/boot.s -o build/boot.o

python3 mkarchive.py src/rootfs build/archive.sfs
xxd -i build/archive.sfs > build/sfs.c

gcc $CFLAGS \
    -c build/sfs.c \
    -o build/sfs.o


for file in src/*.c; do
    name=$(basename "$file" .c)

    gcc $CFLAGS \
        -c "$file" \
        -o "build/$name.o"
done

ld -m elf_i386 \
    -T src/misc/linker.ld \
    -o build/kernel.bin \
    build/*.o

cp build/kernel.bin build/iso/boot/kernel.bin
cp src/misc/grub.cfg build/iso/boot/grub/grub.cfg

grub2-mkrescue \
    -o goobos.img \
    build/iso

qemu-system-i386 \
    -drive format=raw,file=goobos.img \
    -serial stdio \
    -audiodev driver=sdl,id=audio0 \
    -machine pcspk-audiodev=audio0 \
    -m 64M
