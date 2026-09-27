#!/bin/bash
set -e

ASM="../asm"
SRC="../src"
INC="../inc"
CFLAGS="-target i386-pc-none-elf -march=i386 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc -O2 -I$INC"

mkdir -p source
rm -f *.bin *.o os_image.bin qemu_debug.log
rm -f source/*

nasm "$ASM/bootloader.asm" -f bin -o source/bootloader.bin
nasm "$ASM/entry.asm" -f elf32 -o source/entry.o

OBJS=("source/entry.o")
for src in "$SRC"/*.c; do
    obj="source/$(basename "${src%.c}.o")"
    clang $CFLAGS -c "$src" -o "$obj"
    OBJS+=("$obj")
done

ld -m elf_i386 -T linker.ld --oformat binary "${OBJS[@]}" -o source/kernel.bin

KERNEL_SIZE=$(stat -c%s source/kernel.bin)
KERNEL_SECTORS=$(( (KERNEL_SIZE + 511) / 512 ))
KERNEL_SECTORS=$(( KERNEL_SECTORS + 10 ))

echo "kernel.bin size = $KERNEL_SIZE bytes, loading $KERNEL_SECTORS sectors"

if [ "$KERNEL_SECTORS" -gt 120 ]; then
    echo "ERROR: kernel too large for single-segment real-mode loader ($KERNEL_SECTORS sectors > 120)"
    exit 1
fi

nasm "$ASM/kernel.asm" -f bin -D SECTOR_COUNT=$KERNEL_SECTORS -o source/stage2.bin

dd if=/dev/zero of=os_image.bin bs=512 count=2880
dd if=source/bootloader.bin of=os_image.bin bs=512 count=1 conv=notrunc
dd if=source/stage2.bin of=os_image.bin bs=512 seek=1 count=1 conv=notrunc
dd if=source/kernel.bin of=os_image.bin bs=512 seek=2 conv=notrunc

if [ ! -f fat_area.img ]; then
    dd if=/dev/zero of=fat_area.img bs=512 count=2846
    mkfs.vfat -F 32 -n "DUD_DATA" fat_area.img
fi

dd if=fat_area.img of=os_image.bin bs=512 seek=34 conv=notrunc

qemu-system-x86_64 \
  -drive file=os_image.bin,format=raw,if=floppy \
  -boot a \
  -monitor stdio \
  -d int,cpu_reset -D qemu_debug.log \
  -no-reboot -no-shutdown
