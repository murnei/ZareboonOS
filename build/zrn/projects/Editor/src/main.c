#include "zrn.h"
#include "zrn_syscall.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "gfx.h"
#include "system.h"
#include "args.h"
#include "buffer.h"
#include "view.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define NAME_CAPACITY 13
#define TAB_WIDTH 4

static buffer_t buffer;

static uint8_t key_read(void) {
    char ch;
    do {
        ch = ((sys_getch_t)SYSCALL_TABLE[SYS_GETCH])();
    } while (ch == 0);
    return (uint8_t)ch;
}

static bool is_printable(uint8_t ch) {
    return ch >= 32 && ch <= 126;
}

static void read_name(char* name) {
    uint32_t length = 0;
    name[0] = '\0';

    for (;;) {
        view_prompt("File name: ", name);
        uint8_t ch = key_read();

        if (ch == '\n') {
            if (length > 0) return;
        } else if (ch == '\b') {
            if (length > 0) name[--length] = '\0';
        } else if (ch > 32 && ch <= 126 && length + 1 < NAME_CAPACITY) {
            name[length++] = (char)ch;
            name[length] = '\0';
        }
    }
}

static void copy_name(char* name, const char* source) {
    uint32_t length = 0;

    while (source[length] && length + 1 < NAME_CAPACITY) {
        name[length] = source[length];
        length++;
    }
    name[length] = '\0';
}

static bool load(const char* name) {
    int32_t count = read_file((const uint8_t*)name, buffer.data, BUFFER_CAPACITY - 1);

    if (count < 0) {
        buffer_clear(&buffer);
        return false;
    }

    uint32_t kept = 0;
    for (uint32_t i = 0; i < BUFFER_CAPACITY - 1 && buffer.data[i]; i++) {
        uint8_t ch = buffer.data[i];
        if (ch == '\n' || is_printable(ch)) buffer.data[kept++] = ch;
    }

    buffer.length = kept;
    return true;
}

static void save(const char* name, bool exists) {
    if (!exists) create_file((const uint8_t*)name);
    write_file((const uint8_t*)name, buffer.data, buffer.length);
}

static void insert_tab(void) {
    for (uint32_t i = 0; i < TAB_WIDTH; i++) buffer_push(&buffer, ' ');
}

static bool edit(const char* name, bool exists) {
    const char* status = exists ? "opened" : "new file";

    for (;;) {
        view_editor(&buffer, name, status);

        uint8_t ch = key_read();
        status = "";

        if (ch == '\n') {
            if (buffer_line_is(&buffer, ":wq")) {
                buffer_drop_line(&buffer);
                save(name, exists);
                return true;
            }
            if (buffer_line_is(&buffer, ":q")) return false;
            if (!buffer_push(&buffer, '\n')) status = "buffer full";
        } else if (ch == '\b') {
            buffer_pop(&buffer);
        } else if (ch == '\t') {
            insert_tab();
        } else if (is_printable(ch)) {
            if (!buffer_push(&buffer, ch)) status = "buffer full";
        }
    }
}

ZRN_ENTRY
void main(void) {
    char name[NAME_CAPACITY];

    if (!gfx_open(SCREEN_WIDTH, SCREEN_HEIGHT)) return;

    if (args_count() > 0) copy_name(name, args_at(0));
    else read_name(name);

    buffer_clear(&buffer);

    bool exists = load(name);
    bool saved = edit(name, exists);

    gfx_close();

    print(saved ? "Saved %s" : "Closed %s", name);
}
