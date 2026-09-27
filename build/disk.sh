#!/bin/bash
set -e

DISK_NAME="fat_area.img"
CHOICE=
SRC="zrn"
INC="../inc"
CFLAGS="-target i386-pc-none-elf -march=i386 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc -O2 -I$INC"


while true; do
    clear
    echo "=============================="
    echo "    ####   ###  ###  #   #    "
    echo "    #   #   #   #    # #      "
    echo "    #   #   #   ###  # #      "
    echo "    #   #   #     #  #  #     "
    echo "    ####   ###  ###  #   #    "
    echo "=============================="

    echo "0 - exit"
    echo "1 - create/recreate disk in fat32"
    echo "2 - copy file on disk"
    echo "3 - compile and copy on disk"
    echo "4 - delete files from disk"

    read -p "your choice: " CHOICE

    case $CHOICE in
        0)
            clear
            break
            ;;
        1)
            rm $DISK_NAME

            dd if=/dev/zero of="$DISK_NAME" bs=4096 count=25600
            mkfs.vfat -F 32 -n "DUD_DATA" "$DISK_NAME"
            ;;
        2)
            read -p "enter file name: " CHOICE
		    CLEAN_NAME=$(basename "$CHOICE")

			mcopy -o -D o -i "$DISK_NAME" "$CHOICE" "::$CLEAN_NAME"
			;;
        3)
            read -p "enter file name without extension: " CHOICE
            clang $CFLAGS -c "$SRC/$CHOICE.c" -o "$SRC/$CHOICE.zrn"
            
            ld -m elf_i386 -T zrn/zrn.ld --oformat binary "$SRC/$CHOICE.zrn" -o $SRC/$CHOICE

            mcopy -o -D o -i "$DISK_NAME" "$SRC/$CHOICE" "::$CHOICE.zrn"
            ;;
        4)
            mdir -i "$DISK_NAME" ::*
            echo ""
            read -p "enter file name: " CHOICE
            mdel -i "$DISK_NAME" "::$CHOICE"
            ;;
        *)
            echo "Usage: no choice found"
            sleep 1
            ;;
      esac

      clear
done
