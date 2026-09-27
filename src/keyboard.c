#include "keyboard.h"
#include "stdint.h"
#include "stdbool.h"
#include "io.h"

#define KEYBOARD_BUFFER_SIZE 256

static char key_buffer[KEYBOARD_BUFFER_SIZE];
static uint16_t buf_head = 0;
static uint16_t buf_tail = 0;

static bool shift_pressed = false;
static bool caps_lock = false;

static const char ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9',  '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
    't',  'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm',  ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,
};

static const char ascii_shift[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(',  ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',
    'T',  'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"',  '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M',  '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,
};

#define SC_LEFT_SHIFT   0x2A
#define SC_RIGHT_SHIFT  0x36
#define SC_CAPS_LOCK    0x3A
#define SC_RELEASE_MASK 0x80

static void buffer_push(char c) {
    uint16_t next = (buf_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != buf_tail) {
        key_buffer[buf_head] = c;
        buf_head = next;
    }
}

char scancode_to_ascii(uint8_t scancode) {
    bool use_shift = shift_pressed;
    char base = ascii[scancode];
    bool is_letter = (base >= 'a' && base <= 'z');

    if (is_letter && caps_lock) {
        use_shift = !use_shift;
    }

    return use_shift ? ascii_shift[scancode] : ascii[scancode];
}

void keyboard_handler_c() {
    uint8_t scancode = inb(0x60);

    bool released = (scancode & SC_RELEASE_MASK) != 0;
    uint8_t code = scancode & ~SC_RELEASE_MASK;

    switch (code) {
        case SC_LEFT_SHIFT:
        case SC_RIGHT_SHIFT:
            shift_pressed = !released;
            goto eoi;

        case SC_CAPS_LOCK:
            if (!released) {
                caps_lock = !caps_lock;
            }
            goto eoi;

        default:
            break;
    }

    if (!released && code < 128) {
        char c = scancode_to_ascii(code);
        if (c != 0) {
            buffer_push(c);
        }
    }

eoi:
    outb(0x20, 0x20);
}

bool keyboard_has_input() {
    return buf_head != buf_tail;
}

char keyboard_read_char() {
    if (buf_head == buf_tail) {
        return 0;
    }

    char c = key_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}
