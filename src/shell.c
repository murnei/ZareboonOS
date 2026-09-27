#include "stdio.h"
#include "idt.h"
#include "keyboard.h"
#include "console.h"
#include "video.h"
#include "stdlib.h"
#include "cursor.h"
#include "string.h"
#include "stdbool.h"
#include "stdint.h"
#include "fs.h"
#include "loader.h"
#include "syscall.h"
#include "time.h"
#include "floppy.h"
#include "gui.h"

static void all_init(void) {
    init_idt();
    floppy_init();
    fs_init();
    heap_init();
    syscalls_init();
}

void format_time() {
    struct time_now now;
    get_time(&now);
    if (now.seconds < 10) {
        print("%d:%d:0%d", now.hours, now.minutes, now.seconds);
    } else {
        print("%d:%d:%d", now.hours, now.minutes, now.seconds);
    }
}

static void echo_shell(const char* arg) {
    print(arg);
}

void shell(const uint8_t* ch, const uint8_t* arg) {
    if (strcmp(ch, (const uint8_t*)"help")) {
        print("Commands: help, clear, ls, echo, run, time");
    } else if (strcmp(ch, (const uint8_t*)"clear")) {
        clear();
    } else if (strcmp(ch, (const uint8_t*)"run")) {
        exec_program(arg);
    } else if (strcmp(ch, (const uint8_t*)"ls")) {
        fat32_list_files();
    } else if (strcmp(ch, (const uint8_t*)"time")) { 
        format_time();
    } else if (strcmp(ch, (const uint8_t*)"echo")) {
        echo_shell((const char*)arg);
    } else if (strcmp(ch, (const uint8_t*)"sleep")) {
        uint32_t num = atoi(arg);
        sleep(num);
    } else {
        print("Usage: No command found");
    }
}

void shell_main() {
    all_init();

    draw_os_logo();

    uint8_t command[256];
    uint8_t arg[256];
    uint8_t index = 0;

    bool writing_command = true;

    print("\n>");

    while (1) {
        if (keyboard_has_input()) {
            uint8_t ch = keyboard_read_char();

            if (ch == '\n') {
                if (writing_command) {
                    command[index] = '\0';
                } else {
                    arg[index] = '\0';
                }

                new_line();
                shell(command, (const uint8_t*)arg);

                index = 0;
                writing_command = true;
                command[0] = '\0';
                arg[0] = '\0';
                print("\n>");
            } else if (ch == '\b') {
                if (index > 0) {
                    index--;
                    move_cursor(-1, 0);
                    print_char(' ', rgb(255, 255, 255));
                    move_cursor(-1, 0);
                }
            } else if (ch == ' ' && writing_command) {
                command[index] = '\0';
                print_char(ch, rgb(255, 255, 255));
                writing_command = false;
                index = 0;
            } else if (ch == '\t') {
                move_cursor(4, 0);
                print_char(ch, rgb(255, 255, 255));
            } else {
                if (index < 255) {
                    if (writing_command) {
                        command[index] = ch;
                    } else {
                        arg[index] = ch;
                    }

                    index++;
                    print_char(ch, rgb(255, 255, 255));
                }
            }
        }

        __asm__ volatile("hlt");
    }
}
