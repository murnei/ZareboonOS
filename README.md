# ZareboonOS

ZareboonOS is a minimalist, bare-metal operating system written from scratch in C and Assembly. The entire project is developed, compiled, and maintained directly within the Termux environment on Android.

## Features
- **Termux Native:** Built completely on mobile using `clang` and `nasm` via Termux.
- **FAT12 File System:** Exclusively designed to work with and manage the FAT12 file system architecture.
- **Custom Shell:** Includes a lightweight `zsh` implementation supporting basic CLI variables, execution, and environment controls (`clear`, `sleep`).
- **Hardware Drivers:** Features custom implementation for IDT (Interrupt Descriptor Table), keyboard input, and disk I/O.

## Project Structure
- `src/` — Core C source files (kernel, shell, drivers).
- `include/` — Header files (`.h`).
- `asm/` — Low-level assembly bootloader and kernel entry points.
- `build/` — Linker scripts and automation tools.

## Requirements & Building
To compile and test the OS image, you need `clang`, `nasm`, `ld`, and `qemu-system-x86_64` installed in your Termux environment.

To build and run:
```bash
cd build
./build.sh
