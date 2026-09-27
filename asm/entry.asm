[bits 32]

section .text.entry

global _start

extern kernel_main
extern graphic_init
extern isr_handler_c
extern keyboard_handler_c
extern timer_handler_c

global timer_isr_stub
global isr_stub_table
global keyboard_isr_stub

_start:
   push ebx

   call graphic_init

   add esp, 4

   call kernel_main

loop:
   hlt
   jmp loop

%assign i 0
%rep 32

isr_stub_%+i:
    %if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21
    %else
        push 0
    %endif
    push i
    jmp isr_common_stub

%assign i i+1

%endrep

isr_stub_table:

%assign i 0
%rep 32
    dd isr_stub_%+i
%assign i i+1
%endrep

isr_common_stub:
    pushad

    call isr_handler_c

    popad

    add esp, 8

    iretd

keyboard_isr_stub:
    pushad
    call keyboard_handler_c
    popad
    iretd

timer_isr_stub:
    pushad
    call timer_handler_c
    popad
    iretd
