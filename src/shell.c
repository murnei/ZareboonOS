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
#include "ata.h"
#include "display.h"
#include "args.h"

#define LINE_CAPACITY 256
#define PROMPT "\n>"
#define TEXT_COLOR rgb(255, 255, 255)
#define BACKGROUND_COLOR rgb(0, 0, 0)

typedef void (*command_fn_t)(const uint8_t* arg);

typedef struct {
    const char* name;
    command_fn_t run;
} command_t;

typedef struct {
    uint8_t command[LINE_CAPACITY];
    uint8_t arg[LINE_CAPACITY];
    uint8_t index;
    uint8_t command_length;
    bool in_command;
} line_t;

static bool console_suspended = false;

void console_suspend(void) {
    console_suspended = true;
}

void console_resume(void) {
    console_suspended = false;
    clear();
}

static void cmd_mkdir(const uint8_t* arg) {
    make_dir(arg);
}

static void cmd_rmdir(const uint8_t* arg) {
    remove_dir(arg);
}

static void cmd_cat(const uint8_t* arg) {
    fs_cat(arg);
}

static void cmd_help(const uint8_t* arg);

static void cmd_clear(const uint8_t* arg) {
    (void)arg;
    clear();
}

static void cmd_ls(const uint8_t* arg) {
    (void)arg;
    fat32_list_files();
}

static void cmd_echo(const uint8_t* arg) {
    print((const char*)arg);
}

static void cmd_run(const uint8_t* arg) {
    uint8_t name[LINE_CAPACITY];
    uint32_t index = 0;

    while (arg[index] && arg[index] != ' ' && index < LINE_CAPACITY - 1) {
        name[index] = arg[index];
        index++;
    }
    name[index] = '\0';

    if (index == 0) {
        print("Usage: run <program> [args]");
        return;
    }

    while (arg[index] == ' ') index++;

    args_set(arg + index);
    exec_program(name);
    args_set((const uint8_t*)"");
    display_close();
}

static void cmd_time(const uint8_t* arg) {
    (void)arg;
    print_format_time();
}

static void cmd_logo(const uint8_t* arg) {
    (void)arg;
    draw_os_logo();
}

static const command_t commands[] = {
    { "help", cmd_help },
    { "clear", cmd_clear },
    { "ls", cmd_ls },
    { "echo", cmd_echo },
    { "run", cmd_run },
    { "time", cmd_time },
    { "logo", cmd_logo },
    { "mkdir", cmd_mkdir },
    { "rmdir", cmd_rmdir },
    { "cat", cmd_cat }
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

static void cmd_help(const uint8_t* arg) {
    (void)arg;
    print("Commands: ");
    for (uint32_t i = 0; i < COMMAND_COUNT; i++) {
        print(commands[i].name);
        if (i + 1 < COMMAND_COUNT) print(", ");
    }
}

static void execute(const uint8_t* command, const uint8_t* arg) {
    for (uint32_t i = 0; i < COMMAND_COUNT; i++) {
        if (strcmp(command, (const uint8_t*)commands[i].name)) {
            commands[i].run(arg);
            return;
        }
    }
    print("Usage: No command found");
}

static void system_init(void) {
    init_idt();
    ata_init();
    fs_init();
    heap_init();
    syscalls_init();
}

static void line_reset(line_t* line) {
    line->index = 0;
    line->command_length = 0;
    line->in_command = true;
    line->command[0] = '\0';
    line->arg[0] = '\0';
}

static uint8_t* line_target(line_t* line) {
    return line->in_command ? line->command : line->arg;
}

static void erase_previous_cell(void) {
    move_cursor(-1, 0);
    draw_black_square(get_raw_cursor_x(), get_raw_cursor_y(), BACKGROUND_COLOR);
    update_cursor();
}

static void line_submit(line_t* line) {
    line_target(line)[line->index] = '\0';

    bool empty = line->in_command ? (line->index == 0) : (line->command_length == 0);

    new_line();

    if (!empty) execute(line->command, line->arg);

    line_reset(line);
    print(PROMPT);
}

static void line_backspace(line_t* line) {
    if (line->index > 0) {
        line->index--;
        line_target(line)[line->index] = '\0';
        erase_previous_cell();
        return;
    }

    if (!line->in_command) {
        line->in_command = true;
        line->index = line->command_length;
        line->command[line->index] = '\0';
        line->arg[0] = '\0';
        erase_previous_cell();
    }
}

static void line_space(line_t* line) {
    line->command[line->index] = '\0';
    line->command_length = line->index;
    move_cursor(1, 0);
    update_cursor();
    line->in_command = false;
    line->index = 0;
    line->arg[0] = '\0';
}

static void line_tab(void) {
    move_cursor(4, 0);
    update_cursor();
}

static void line_append(line_t* line, uint8_t ch) {
    if (line->index >= LINE_CAPACITY - 1) return;

    uint8_t* target = line_target(line);
    target[line->index] = ch;
    target[line->index + 1] = '\0';
    line->index++;

    print_char(ch, TEXT_COLOR);
    update_cursor();
}

static void handle_key(line_t* line, uint8_t ch) {
    if (ch == '\n') {
        line_submit(line);
    } else if (ch == '\b') {
        line_backspace(line);
    } else if (ch == ' ' && line->in_command) {
        line_space(line);
    } else if (ch == '\t') {
        line_tab();
    } else {
        line_append(line, ch);
    }
}

void shell_main(void) {
    system_init();
    draw_os_logo();

    line_t line;
    line_reset(&line);

    print(PROMPT);

    while (1) {
        if (!console_suspended && keyboard_has_input()) {
            handle_key(&line, keyboard_read_char());
        }
        __asm__ volatile("hlt");
    }
}
