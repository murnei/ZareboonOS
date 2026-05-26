#include "stdio.h"
#include "keyboard.h"
#include "idt.h"
#include "disk.h"
#include "shell.h"

extern void* syscall_table[];

void run_program(char* filename) {
    uint8_t* load_address = (uint8_t*)0x20000;

    if (load_file(filename, load_address) == 0) {
        print("\nError: Could not load file.");
        return;
    }

    if (load_address[0] == 0x7F && load_address[1] == 'Z' &&
        load_address[2] == 'R' && load_address[3] == 'N') {

        print("\nStarting Zareboon Executable...");

        void* entry_point = (void*)0x20004;

        __asm__ volatile (
            "mov %0, %%ebx\n\t"
            "call *%1\n\t"
            :
            : "r"(syscall_table), "r"(entry_point)
            : "ebx", "memory"
        );

    } else {
        print("\nError: Not a valid .zrn executable (Invalid Signature).");
    }
}

void text_editor(const char* filename, uint32_t lba, uint8_t* root_buffer, int root_offset) {
    uint8_t buffer[512];
    for (int i = 0; i < 512; i++) buffer[i] = 0;

    read_sector(lba, buffer);

    int len = 0;
    while (len < 512 && buffer[len] != '\0') len++;
    int pos = len;

    while (1) {
        clear();
        print("--- Zareboon Editor --- File: ");
        print(filename);
        print(" --- [ESC: Save & Exit]\n\n");

        int cx = cursor_x;
        int cy = cursor_y;

        for (int i = 0; i < len; i++) {
            if (i == pos) {
                cx = cursor_x;
                cy = cursor_y;
            }
            char s[2] = {buffer[i], '\0'};
            print(s);
        }
        if (pos == len) {
            cx = cursor_x;
            cy = cursor_y;
        }

        cursor_x = cx;
        cursor_y = cy;
        update_cursor(cursor_x, cursor_y);

        char c = get_char();

        if (c == 27) {
            break;
        } 
        else if (c == 3) {
            if (pos > 0) pos--;
        } 
        else if (c == 4) {
            if (pos < len) pos++;
        } 
        else if (c == 1) {
            int col = 0;
            int curr = pos;
            while (curr > 0 && buffer[curr - 1] != '\n') {
                curr--;
                col++;
            }
            if (curr > 0) {
                curr--;
                int prev_start = curr;
                while (prev_start > 0 && buffer[prev_start - 1] != '\n') {
                    prev_start--;
                }
                pos = prev_start;
                while (pos < curr && col > 0) {
                    pos++;
                    col--;
                }
            }
        } 
        else if (c == 2) {
            int col = 0;
            int curr = pos;
            while (curr > 0 && buffer[curr - 1] != '\n') {
                curr--;
                col++;
            }
            int next_line = pos;
            while (next_line < len && buffer[next_line] != '\n') {
                next_line++;
            }
            if (next_line < len) {
                next_line++;
                pos = next_line;
                while (pos < len && buffer[pos] != '\n' && col > 0) {
                    pos++;
                    col--;
                }
            }
        } 
        else if (c == '\b') {
            if (pos > 0) {
                for (int i = pos - 1; i < len; i++) buffer[i] = buffer[i + 1];
                pos--;
                len--;
                buffer[len] = '\0';
            }
        } 
        else if (len < 511 && ((c >= 32 && c <= 126) || c == '\n')) {
            for (int i = len; i > pos; i--) buffer[i] = buffer[i - 1];
            buffer[pos] = c;
            pos++;
            len++;
            buffer[len] = '\0';
        }
    }

    write_sector(lba, buffer);
    root_buffer[root_offset + 28] = 0x00;
    root_buffer[root_offset + 29] = 0x02;
    write_sector(19, root_buffer);
    clear();
    print("\n[System] File saved successfully.\n");
}
