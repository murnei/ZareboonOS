#!/bin/bash
set -e

ASM="../asm"
SRC="../src"
INC="../inc"

FS_LBA=64
KERNEL_LBA=3

CFLAGS=(
    -target i386-pc-none-elf
    -march=i386
    -ffreestanding
    -fno-pie
    -fno-stack-protector
    -nostdlib
    -nostdinc
    -O2
    "-I$INC"
)

if command -v qemu-system-x86_64 >/dev/null 2>&1; then
    QEMU=qemu-system-x86_64
elif command -v qemu-system-i386 >/dev/null 2>&1; then
    QEMU=qemu-system-i386
else
    echo "ERROR: QEMU не найден (pkg install qemu-system-x86-64-headless)"
    exit 1
fi

mkdir -p source
rm -f *.bin *.o os_image.bin qemu_debug.log
rm -f source/*

echo "[1/7] Building bootloader..."
nasm "$ASM/bootloader.asm" -f bin -o source/bootloader.bin

echo "[2/7] Building entry..."
nasm "$ASM/entry.asm" -f elf32 -o source/entry.o

OBJS=("source/entry.o")

echo "[3/7] Compiling C sources..."
for src in "$SRC"/*.c; do
    obj="source/$(basename "${src%.c}.o")"
    clang "${CFLAGS[@]}" -c "$src" -o "$obj"
    OBJS+=("$obj")
done

echo "[4/7] Linking kernel..."
ld -m elf_i386 -T linker.ld --oformat binary "${OBJS[@]}" -o source/kernel.bin

KERNEL_SIZE=$(stat -c%s source/kernel.bin)
KERNEL_SECTORS=$(((KERNEL_SIZE + 511) / 512))
KERNEL_MAX=$((FS_LBA - KERNEL_LBA))

echo "kernel.bin size = $KERNEL_SIZE bytes"
echo "loading $KERNEL_SECTORS sectors (max $KERNEL_MAX)"

if [ "$KERNEL_SECTORS" -gt "$KERNEL_MAX" ]; then
    echo "ERROR: kernel overlaps FAT area"
    exit 1
fi

echo "[5/7] Building stage2..."
nasm "$ASM/kernel.asm" \
    -f bin \
    -D "SECTOR_COUNT=$KERNEL_SECTORS" \
    -D "KERNEL_LBA=$KERNEL_LBA" \
    -o source/stage2.bin

STAGE2_SIZE=$(stat -c%s source/stage2.bin)
STAGE2_MAX=$(((KERNEL_LBA - 1) * 512))
echo "stage2.bin size = $STAGE2_SIZE bytes (max $STAGE2_MAX)"

if [ "$STAGE2_SIZE" -gt "$STAGE2_MAX" ]; then
    echo "ERROR: stage2 overlaps kernel"
    exit 1
fi

echo "[6/7] Creating OS image..."
if [ ! -f fat_area.img ]; then
    echo "ERROR: fat_area.img not found! Run ./disk.sh first."
    exit 1
fi

FAT_SECTORS=$((($(stat -c%s fat_area.img) + 511) / 512))
IMAGE_SECTORS=$((FS_LBA + FAT_SECTORS))
echo "image size = $IMAGE_SECTORS sectors"

dd if=/dev/zero of=os_image.bin bs=512 count=$IMAGE_SECTORS status=none
dd if=source/bootloader.bin of=os_image.bin bs=512 count=1 conv=notrunc status=none
dd if=source/stage2.bin of=os_image.bin bs=512 seek=1 conv=notrunc status=none
dd if=source/kernel.bin of=os_image.bin bs=512 seek=$KERNEL_LBA conv=notrunc status=none
dd if=fat_area.img of=os_image.bin bs=512 seek=$FS_LBA conv=notrunc status=none

sync_fat_back() {
    if [ -f os_image.bin ]; then
        dd if=os_image.bin of=fat_area.img bs=512 skip=$FS_LBA count=$FAT_SECTORS conv=notrunc status=none
        echo "FAT area saved back to fat_area.img"
    fi
}
trap sync_fat_back EXIT

echo "[7/7] Starting QEMU (VNC на localhost:5900)..."
"$QEMU" \
    -drive file=os_image.bin,format=raw,if=none,id=disk \
    -device ide-hd,drive=disk,bootindex=0 \
    -display none \
    -vnc 127.0.0.1:0 \
    -monitor stdio \
    -d int,cpu_reset \
    -D qemu_debug.log \
    -no-reboot \
    -no-shutdown
