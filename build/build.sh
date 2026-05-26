#!/usr/bin/env bash
set -e

SRC="../src"
INC="../include"
ASM="../asm"

rm -f *.o *.bin

CFLAGS="-target i386-pc-none-elf -march=i386 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc -I$INC"

nasm -f bin $ASM/bootloader.asm -o boot.bin
nasm -f bin $ASM/kernel.asm -o kernel.bin
nasm -f elf32 $ASM/kernel_entry.asm -o entry.o

clang $CFLAGS -c $SRC/keyboard.c -o keyboard.o
clang $CFLAGS -c $SRC/kernel.c -o kernel.o
clang $CFLAGS -c $SRC/stdio.c -o stdio.o
clang $CFLAGS -c $SRC/idt.c -o idt.o
clang $CFLAGS -c $SRC/system.c -o system.o
clang $CFLAGS -c $SRC/disk.c -o disk.o
clang $CFLAGS -c $SRC/shell.c -o shell.o
clang $CFLAGS -c $SRC/programs.c -o programs.o
clang $CFLAGS -c $SRC/compiler.c -o compiler.o

ld -m elf_i386 -T linker.ld --image-base=0 --oformat binary \
   entry.o kernel.o keyboard.o stdio.o idt.o system.o disk.o shell.o programs.o compiler.o \
   -o kernel_c.bin

dd if=/dev/zero of=os_image.bin bs=512 count=2880
dd if=boot.bin of=os_image.bin bs=512 count=1 conv=notrunc
dd if=kernel.bin of=os_image.bin bs=512 seek=1 conv=notrunc
dd if=kernel_c.bin of=os_image.bin bs=512 seek=3 conv=notrunc

echo "Build successful! Launching..."
qemu-system-x86_64 \
    -drive file=os_image.bin,format=raw,if=floppy \
    -drive file=test-disk.img,format=raw,if=ide,bus=0,unit=0,media=disk \
    -boot a \
    -monitor stdio
