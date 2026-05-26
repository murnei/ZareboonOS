#include "stdio.h"
#include "keyboard.h"
#include "idt.h"
#include "system.h"
#include "shell.h"

__attribute__((section(".syscalls")))
void* syscall_table[] = {
    (void*)print,       // Индекс 0
    (void*)clear,       // Индекс 1
    (void*)get_char,    // Индекс 2
    (void*)put_char,
    (void*)sleep,
    (void*)print_int,
};

void syscall_dispatcher(int id, void* arg1) {
    switch (id) {
        case 0: { // print
            void (*func)(const char*) = syscall_table[0];
            func((const char*)arg1);
            break;
        }
        case 1: { // clear
            void (*func)() = syscall_table[1];
            func();
            break;
        }
        case 4: { // sleep (принимает число)
            void (*func)(int) = syscall_table[4];
            func((int)arg1); 
            break;
        }
        case 5: {
            void (*func)(int) = syscall_table[5];
	    func((int)arg1);
	    break;
        }
    }
}

void kernel_main() {
    idt_install();

    clear();
    draw_os_logo();

    shell_main();

    while(1) {
        __asm__ volatile("hlt");
    } 
}
