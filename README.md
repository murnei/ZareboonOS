# ZareboonOS

**ZareboonOS** is a hobby x86 operating system written from scratch in **C and Assembly**.

The project started as an experiment in low-level programming and operating system development and has grown into a standalone system with its own filesystem support, executable format, system calls, and userspace environment.

## Current state

ZareboonOS currently supports:

* x86 architecture
* **1024×768** graphics mode
* **FAT32** filesystem
* Reading files from disk
* External `.zrn` programs and scripts
* A custom syscall-based system API
* Execution of third-party `.zrn` programs
* A custom runtime environment
* Kernel and userspace components
* C and Assembly code throughout the system

The system is designed to run independently rather than as an application on top of another operating system.

## `.zrn` programs

ZareboonOS has its own program format and scripting environment.

`.zrn` programs can be stored on the FAT32 filesystem and launched directly by the operating system.

Programs interact with the system through the ZareboonOS syscall API instead of directly depending on kernel internals.

This allows external programs to use functionality provided by the operating system while remaining separate from the kernel itself.

## Architecture

The project is primarily written in:

* **C** — kernel, filesystem, system APIs and system logic
* **Assembly** — boot code, low-level CPU operations and entry points

The system is built from the ground up without using an existing operating-system kernel or framework.

## Filesystem

ZareboonOS uses **FAT32** as its filesystem.

The kernel contains its own FAT32 implementation for accessing files stored on disk. This is used by the system to load files and execute `.zrn` programs.

## Building

The project is currently built using an x86 cross-compilation toolchain and custom build scripts.

Build instructions are provided in the repository.

## Status

ZareboonOS is an experimental hobby operating system.

The project is primarily intended for:

* learning operating system development;
* experimenting with low-level programming;
* exploring x86 architecture;
* developing custom system interfaces and runtimes.

It is not intended to replace a general-purpose operating system.

## License

ZareboonOS is licensed under the **GNU General Public License v3.0**.
