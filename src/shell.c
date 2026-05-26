#include "stdio.h"
#include "system.h"
#include "disk.h"
#include "programs.h"
#include "keyboard.h"

void format_fat_name(const char* input, char* output);

char history[3][512];
int history_count = 0;
int history_index = -1;
char saved_input[512];

int load_file(char* filename, uint8_t* target_address) {
    char fat_name[11];
    format_fat_name(filename, fat_name);

    uint8_t sector[512];
    read_sector(19, sector);

    for (int i = 0; i < 16; i++) {
        int off = i * 32;
        char match = 1;
        for (int k = 0; k < 11; k++) {
            if (sector[off + k] != fat_name[k]) {
                match = 0;
                break;
            }
        }

        if (match) {
            uint16_t cluster = sector[off + 26] | (sector[off + 27] << 8);
            uint32_t file_lba = 33 + (cluster - 2);
            read_sector(file_lba, target_address);
            return 1;
        }
    }
    return 0;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void format_fat_name(const char* input, char* output) {
    for (int i = 0; i < 11; i++) output[i] = ' ';
    int i = 0, j = 0;
    while (input[i] != '.' && input[i] != '\0' && j < 8) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        output[j++] = c;
        i++;
    }
    if (input[i] == '.') {
        i++;
        j = 8;
        while (input[i] != '\0' && j < 11) {
            char c = input[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            output[j++] = c;
            i++;
        }
    }
}

void cmd_help() {
    print("\nAvailable: help, clear, echo, time, ls, cat, touch, run, edit, asm, reboot");
}

void cmd_clear() {
    clear();
}

void cmd_echo(char* arg) {
    if (arg && *arg != '\0') {
        print("\n");
        print(arg);
    } else {
        print("\nError: echo <text>");
    }
}

void cmd_time() {
    unsigned char h_bcd = get_rtc_register(0x04);
    unsigned char m_bcd = get_rtc_register(0x02);
    unsigned char s_bcd = get_rtc_register(0x00);
    int hours = ((h_bcd / 16) * 10) + (h_bcd % 16);
    int mins  = ((m_bcd / 16) * 10) + (m_bcd % 16);
    int secs  = ((s_bcd / 16) * 10) + (s_bcd % 16);
    print("\nTime: ");
    print_int(hours); print(":");
    print_int(mins); print(":");
    print_int(secs);
}

void cmd_ls() {
    uint8_t sector[512];
    read_sector(19, sector);
    print("\n");
    for (int entry = 0; entry < 16; entry++) {
        int offset = entry * 32;
        if (sector[offset] == 0x00) break;
        if (sector[offset] == 0xE5) continue;
        if (sector[offset + 11] == 0x0F) continue;

        for (int k = 0; k < 11; k++) {
            if (sector[offset + k] != ' ') {
                char s[2] = {sector[offset + k], '\0'};
                print(s);
            }
            if (k == 7 && sector[offset + 8] != ' ') print(".");
        }
        uint16_t cluster = sector[offset + 26] | (sector[offset + 27] << 8);
        print(" | CL:"); print_int(cluster); print("\n");
    }
}

void cmd_cat(char* arg) {
    if (arg && *arg != '\0') {
        char fat_name[11];
        format_fat_name(arg, fat_name);
        uint8_t sector[512];
        read_sector(19, sector);
        int found_offset = -1;
        for (int entry = 0; entry < 16; entry++) {
            int off = entry * 32;
            char match = 1;
            for (int k = 0; k < 11; k++) if (sector[off + k] != fat_name[k]) match = 0;
            if (match) { found_offset = off; break; }
        }
        if (found_offset != -1) {
            uint16_t cluster = sector[found_offset + 26] | (sector[found_offset + 27] << 8);
            uint32_t file_lba = 33 + (cluster - 2);
            read_sector(file_lba, sector);
            print("\n");
            for (int k = 0; k < 512; k++) {
                if (sector[k] >= 32 && sector[k] <= 126) {
                    char s[2] = {sector[k], '\0'}; print(s);
                } else if (sector[k] == '\n') print("\n");
            }
        } else {
            print("\nNot found.");
        }
    } else {
        print("\nUsage: cat <filename>");
    }
}

void cmd_touch(char* arg) {
    if (arg && *arg != '\0') {
        char fat_name[11];
        format_fat_name(arg, fat_name);
        uint8_t sector[512];
        read_sector(19, sector);
        int free_off = -1;
        uint16_t max_cluster = 2;

        for (int e = 0; e < 16; e++) {
            int off = e * 32;
            if (sector[off] != 0x00 && sector[off] != 0xE5) {
                uint16_t c = sector[off + 26] | (sector[off + 27] << 8);
                if (c > max_cluster) max_cluster = c;
            }
            if ((sector[off] == 0x00 || sector[off] == 0xE5) && free_off == -1) {
                free_off = off;
            }
        }

        if (free_off != -1) {
            for (int k = 0; k < 11; k++) sector[free_off + k] = fat_name[k];
            sector[free_off + 11] = 0x20;
            uint16_t cl = max_cluster + 1;
            sector[free_off + 26] = cl & 0xFF; sector[free_off + 27] = cl >> 8;
            write_sector(19, sector);
            print("\nCreated.");
        } else {
            print("\nError: Root directory is full.");
        }
    } else {
        print("\nUsage: touch <filename>");
    }
}

void cmd_run(char* arg) {
    if (arg && *arg != '\0') {
        __asm__ volatile("sti");
        run_program(arg);
    } else {
        print("\nUsage: run <filename.zrn>");
    }
}

void cmd_edit(char* arg) {
    if (!arg || *arg == '\0') {
        print("\nUsage: edit <filename>");
        return;
    }

    char fat_name[11];
    format_fat_name(arg, fat_name);

    uint8_t root[512];
    read_sector(19, root);

    int off = -1;
    for (int e = 0; e < 16; e++) {
        char m = 1;
        for (int k = 0; k < 11; k++) if (root[e * 32 + k] != fat_name[k]) m = 0;
        if (m) { off = e * 32; break; }
    }

    if (off == -1) {
        cmd_touch(arg);
        read_sector(19, root);
        for (int e = 0; e < 16; e++) {
            char m = 1;
            for (int k = 0; k < 11; k++) if (root[e * 32 + k] != fat_name[k]) m = 0;
            if (m) { off = e * 32; break; }
        }
    }

    if (off != -1) {
        uint16_t cluster = root[off + 26] | (root[off + 27] << 8);
        uint32_t lba = 33 + (cluster - 2);
        text_editor(arg, lba, root, off);
    }
}

void cmd_asm(char* arg) {
    if (arg && *arg != '\0') {
        uint8_t source_buffer[512];
        for(int k = 0; k < 512; k++) source_buffer[k] = 0;

        print("\n[Compiler] Reading source file...");
        if (load_file(arg, source_buffer) == 0) {
            print("\nError: Source file not found.");
        } else {
            char out_name[16];
            int len = 0;
            while (arg[len] != '\0' && arg[len] != '.' && len < 8) {
                out_name[len] = arg[len];
                len++;
            }
            out_name[len++] = '.';
            out_name[len++] = 'z';
            out_name[len++] = 'r';
            out_name[len++] = 'n';
            out_name[len] = '\0';

            uint8_t root[512];
            read_sector(19, root);
            char out_fat[11];
            format_fat_name(out_name, out_fat);

            int found_off = -1;
            int free_off = -1;
            uint16_t max_cluster = 2;

            for (int e = 0; e < 16; e++) {
                int off = e * 32;
                if (root[off] != 0x00 && root[off] != 0xE5) {
                    uint16_t c = root[off + 26] | (root[off + 27] << 8);
                    if (c > max_cluster) max_cluster = c;

                    char m = 1;
                    for (int k = 0; k < 11; k++) {
                        if (root[off + k] != out_fat[k]) m = 0;
                    }
                    if (m) found_off = off;
                }
                if ((root[off] == 0x00 || root[off] == 0xE5) && free_off == -1) {
                    free_off = off;
                }
            }

            uint32_t target_lba = 33;
            if (found_off != -1) {
                uint16_t cluster = root[found_off + 26] | (root[found_off + 27] << 8);
                target_lba = 33 + (cluster - 2);
            } else if (free_off != -1) {
                uint16_t cl = max_cluster + 1;
                for (int k = 0; k < 11; k++) root[free_off + k] = out_fat[k];
                root[free_off + 11] = 0x20;
                root[free_off + 26] = cl & 0xFF; root[free_off + 27] = cl >> 8;
                write_sector(19, root);
                target_lba = 33 + (cl - 2);
            } else {
                print("\nError: Root directory is full.");
                return;
            }

            print("\n[Compiler] Compiling to ");
            print(out_name);
            print("...");

            extern void compile_script(const char* source_text, char* output_filename, uint32_t target_lba);
            compile_script((const char*)source_buffer, out_name, target_lba);

            print("\n[Compiler] Success! Type 'run ");
            print(out_name);
            print("' to execute.");
        }
    } else {
        print("\nUsage: asm <filename.txt>");
    }
}

void command_manager(char* cmd) {
    int i = 0;
    while (cmd[i] != ' ' && cmd[i] != '\0') {
        i++;
    }

    char* arg = 0;
    if (cmd[i] == ' ') {
        cmd[i] = '\0';
        arg = &cmd[i + 1];
    }

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "echo") == 0) {
        cmd_echo(arg);
    } else if (strcmp(cmd, "time") == 0) {
        cmd_time();
    } else if (strcmp(cmd, "ls") == 0) {
        cmd_ls();
    } else if (strcmp(cmd, "cat") == 0) {
        cmd_cat(arg);
    } else if (strcmp(cmd, "touch") == 0) {
        cmd_touch(arg);
    } else if (strcmp(cmd, "run") == 0) {
        cmd_run(arg);
    } else if (strcmp(cmd, "edit") == 0) {
        cmd_edit(arg);
    } else if (strcmp(cmd, "asm") == 0) {
        cmd_asm(arg);
    } else if (strcmp(cmd, "reboot") == 0) {
        reboot();
    } else if (cmd[0] != '\0') {
        print("\nUnknown command.");
    }
}

void read_line(char* buffer, int max_len) {
    int index = 0;
    history_index = -1;

    while (1) {
        char c = get_char();

        if (c == '\n') {
            buffer[index] = '\0';
            print("\n");
            if (index > 0) {
                for (int i = 2; i > 0; i--) {
                    for (int j = 0; j < 512; j++) history[i][j] = history[i - 1][j];
                }
                for (int j = 0; j <= index; j++) history[0][j] = buffer[j];
                if (history_count < 3) history_count++;
            }
            return;
        }
        else if (c == '\b') {
            if (index > 0) {
                index--;
                if (cursor_x == 0 && cursor_y > 0) {
                    cursor_x = 79;
                    cursor_y--;
                } else if (cursor_x > 0) {
                    cursor_x--;
                }
                put_char(cursor_x, cursor_y, ' ', 0x0F);
                update_cursor(cursor_x, cursor_y);
            }
        }
        else if (c == 1 || c == 2) {
            if (c == 1 && history_count > 0 && history_index < history_count - 1) {
                if (history_index == -1) {
                    buffer[index] = '\0';
                    for (int j = 0; j <= index; j++) saved_input[j] = buffer[j];
                }
                history_index++;
            }
            else if (c == 2 && history_index >= 0) {
                history_index--;
            }
            else {
                continue;
            }

            while (index > 0) {
                index--;
                if (cursor_x == 0 && cursor_y > 0) { cursor_x = 79; cursor_y--; }
                else if (cursor_x > 0) cursor_x--;
                put_char(cursor_x, cursor_y, ' ', 0x0F);
            }

            char* source = (history_index == -1) ? saved_input : history[history_index];
            int i = 0;
            while (source[i] != '\0' && index < max_len - 1) {
                buffer[index++] = source[i];
                char str[2] = {source[i], '\0'};
                print(str);
                i++;
            }
            update_cursor(cursor_x, cursor_y);
        }
        else if (index < max_len - 1 && c >= 32 && c <= 126) {
            buffer[index++] = c;
            char str[2] = {c, '\0'};
            print(str);
        }
    }
}

void shell_main() {
    char cmd_buf[512];
    clear();
    draw_os_logo();
    while (1) {
        print("\n> ");
        read_line(cmd_buf, 512);
        command_manager(cmd_buf);
    }
}
