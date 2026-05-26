[bits 32]
section .text.entry
global _start
global idt_load

extern kernel_main
extern idtp

_start:
    call kernel_main
    jmp $

idt_load:
    lidt [idtp]
    ret

global irq1_handler
extern keyboard_handler_main

irq1_handler:
    pusha
    call keyboard_handler_main
    mov al, 0x20
    out 0x20, al
    popa
    iretd

global inw
global irq0_handler
extern timer_handler

irq0_handler:
    pusha
    call timer_handler
    mov al, 0x20
    out 0x20, al
    popa
    iretd

inw:
    mov dx, [esp + 4]
    xor ax, ax
    in ax, dx
    ret

global outw
outw:
    push ebp
    mov ebp, esp
    mov dx, [ebp + 8]
    mov ax, [ebp + 12]
    out dx, ax
    pop ebp
    ret

global syscall_handler_asm
extern syscall_dispatcher

syscall_handler_asm:
    pusha
    push ebx
    push eax
    call syscall_dispatcher
    add esp, 8
    popa
    iretd
