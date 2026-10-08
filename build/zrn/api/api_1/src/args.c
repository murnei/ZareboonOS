#include "stdint.h"
#include "stdbool.h"
#include "zrn_syscall.h"
#include "args.h"

#define ARGS_MAX_LENGTH 256
#define ARGS_MAX_COUNT 16

static char line[ARGS_MAX_LENGTH];
static char storage[ARGS_MAX_LENGTH];
static const char* tokens[ARGS_MAX_COUNT];
static uint32_t count = 0;
static bool parsed = false;

static void parse(void) {
    if (parsed) return;
    parsed = true;

    const uint8_t* raw = ((sys_args_t)SYSCALL_TABLE[SYS_ARGS])();
    uint32_t length = 0;

    if (raw) {
        while (raw[length] && length < ARGS_MAX_LENGTH - 1) {
            line[length] = (char)raw[length];
            storage[length] = (char)raw[length];
            length++;
        }
    }
    line[length] = '\0';
    storage[length] = '\0';

    uint32_t index = 0;
    while (storage[index]) {
        while (storage[index] == ' ') storage[index++] = '\0';
        if (!storage[index]) break;

        if (count < ARGS_MAX_COUNT) tokens[count++] = &storage[index];
        while (storage[index] && storage[index] != ' ') index++;
    }
}

uint32_t args_count(void) {
    parse();
    return count;
}

const char* args_at(uint32_t index) {
    parse();
    return index < count ? tokens[index] : 0;
}

const char* args_line(void) {
    parse();
    return line;
}
