#include "stdio.h"
#include "disk.h"

int strcmp_len(const char* s1, const char* s2, int len) {
    for (int i = 0; i < len; i++) {
        if (s1[i] != s2[i]) return 0;
        if (s1[i] == '\0' || s2[i] == '\0') return s1[i] == s2[i];
    }
    return 1;
}

void compile_script(const char* source_text, char* output_filename, uint32_t target_lba) {
    uint8_t binary[512];
    for (int i = 0; i < 512; i++) binary[i] = 0;

    binary[0] = 0x7F;
    binary[1] = 'Z';
    binary[2] = 'R';
    binary[3] = 'N';

    int out_ptr = 4;
    int src_ptr = 0;

    while (source_text[src_ptr] != '\0' && out_ptr < 400) {
        while (source_text[src_ptr] == ' ' || source_text[src_ptr] == '\n' || source_text[src_ptr] == '\r') {
            src_ptr++;
        }
        if (source_text[src_ptr] == '\0') break;

        if (source_text[src_ptr] >= 'a' && source_text[src_ptr] <= 'z') {
            char var = source_text[src_ptr];
            int look = src_ptr + 1;
            while (source_text[look] == ' ') look++;
            if (source_text[look] == '=') {
                look++;
                while (source_text[look] == ' ') look++;
                if (source_text[look] >= '0' && source_text[look] <= '9') {
                    src_ptr = look;
                    int val = 0;
                    while (source_text[src_ptr] >= '0' && source_text[src_ptr] <= '9') {
                        val = val * 10 + (source_text[src_ptr] - '0');
                        src_ptr++;
                    }
                    uint32_t var_addr = 0x20000 + 400 + (var - 'a') * 4;
                    binary[out_ptr++] = 0xC7;
                    binary[out_ptr++] = 0x05;
                    binary[out_ptr++] = var_addr & 0xFF;
                    binary[out_ptr++] = (var_addr >> 8) & 0xFF;
                    binary[out_ptr++] = (var_addr >> 16) & 0xFF;
                    binary[out_ptr++] = (var_addr >> 24) & 0xFF;
                    binary[out_ptr++] = val & 0xFF;
                    binary[out_ptr++] = (val >> 8) & 0xFF;
                    binary[out_ptr++] = (val >> 16) & 0xFF;
                    binary[out_ptr++] = (val >> 24) & 0xFF;
                    continue;
                }
            }
        }

        if (strcmp_len(&source_text[src_ptr], "clear", 5)) {
            binary[out_ptr++] = 0xB8;
            binary[out_ptr++] = 0x01;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0xCD;
            binary[out_ptr++] = 0x80;
            src_ptr += 5;
        }
        else if (strcmp_len(&source_text[src_ptr], "print ", 6)) {
            src_ptr += 6;
            int check_ptr = src_ptr;
            while (source_text[check_ptr] == ' ') check_ptr++;
            if (source_text[check_ptr] >= 'a' && source_text[check_ptr] <= 'z') {
                int next_ptr = check_ptr + 1;
                while (source_text[next_ptr] == ' ') next_ptr++;
                if (source_text[next_ptr] == '\n' || source_text[next_ptr] == '\r' || source_text[next_ptr] == '\0') {
                    char var = source_text[check_ptr];
                    src_ptr = next_ptr;
                    uint32_t var_addr = 0x20000 + 400 + (var - 'a') * 4;
                    binary[out_ptr++] = 0x8B;
                    binary[out_ptr++] = 0x1D;
                    binary[out_ptr++] = var_addr & 0xFF;
                    binary[out_ptr++] = (var_addr >> 8) & 0xFF;
                    binary[out_ptr++] = (var_addr >> 16) & 0xFF;
                    binary[out_ptr++] = (var_addr >> 24) & 0xFF;
                    binary[out_ptr++] = 0xB8;
                    binary[out_ptr++] = 0x05;
                    binary[out_ptr++] = 0x00;
                    binary[out_ptr++] = 0x00;
                    binary[out_ptr++] = 0x00;
                    binary[out_ptr++] = 0xCD;
                    binary[out_ptr++] = 0x80;
                    continue;
                }
            }

            int str_start = src_ptr;
            while (source_text[src_ptr] != '\n' && source_text[src_ptr] != '\0' && source_text[src_ptr] != '\r') {
                src_ptr++;
            }
            int raw_len = src_ptr - str_start;
            int msg_address = 0x20000 + out_ptr + 17;
            binary[out_ptr++] = 0xBB;
            binary[out_ptr++] = (msg_address) & 0xFF;
            binary[out_ptr++] = (msg_address >> 8) & 0xFF;
            binary[out_ptr++] = (msg_address >> 16) & 0xFF;
            binary[out_ptr++] = (msg_address >> 24) & 0xFF;
            binary[out_ptr++] = 0xB8;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0xCD;
            binary[out_ptr++] = 0x80;
            binary[out_ptr++] = 0xE9;
            int jmp_pos = out_ptr;
            out_ptr += 4;
            int actual_str_len = 0;
            for (int i = 0; i < raw_len; i++) {
                if (source_text[str_start + i] == '\\' && source_text[str_start + i + 1] == 'n') {
                    binary[out_ptr++] = '\n';
                    i++;
                } else {
                    binary[out_ptr++] = source_text[str_start + i];
                }
                actual_str_len++;
            }
            binary[out_ptr++] = '\0';
            actual_str_len++;
            binary[jmp_pos] = actual_str_len & 0xFF;
            binary[jmp_pos + 1] = (actual_str_len >> 8) & 0xFF;
            binary[jmp_pos + 2] = (actual_str_len >> 16) & 0xFF;
            binary[jmp_pos + 3] = (actual_str_len >> 24) & 0xFF;
        }
        else if (strcmp_len(&source_text[src_ptr], "sleep ", 6)) {
            src_ptr += 6;
            int value = 0;
            while (source_text[src_ptr] >= '0' && source_text[src_ptr] <= '9') {
                value = value * 10 + (source_text[src_ptr] - '0');
                src_ptr++;
            }
            binary[out_ptr++] = 0xBB;
            binary[out_ptr++] = (value) & 0xFF;
            binary[out_ptr++] = (value >> 8) & 0xFF;
            binary[out_ptr++] = (value >> 16) & 0xFF;
            binary[out_ptr++] = (value >> 24) & 0xFF;
            binary[out_ptr++] = 0xB8;
            binary[out_ptr++] = 0x04;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0x00;
            binary[out_ptr++] = 0xCD;
            binary[out_ptr++] = 0x80;
        }
        else {
            while (source_text[src_ptr] != '\n' && source_text[src_ptr] != '\0') {
                src_ptr++;
            }
        }
    }

    binary[out_ptr++] = 0xC3;
    write_sector(target_lba, binary);
}
