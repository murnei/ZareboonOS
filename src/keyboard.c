#include "keyboard.h"
#include "stdint.h"
#include "stdbool.h"
#include "io.h"

extern volatile bool app_running;
void app_abort(void);

#define KEYBOARD_BUFFER_SIZE 256
#define KEYBOARD_DATA_PORT   0x60
#define PIC_COMMAND_PORT     0x20
#define PIC_EOI              0x20

#define SC_EXTENDED_PREFIX   0xE0
#define SC_RELEASE_MASK      0x80

#define SC_LEFT_CTRL         0x1D
#define SC_LEFT_SHIFT        0x2A
#define SC_RIGHT_SHIFT       0x36
#define SC_CAPS_LOCK         0x3A
#define SC_C                 0x2E

#define SC_EXT_HOME          0x47
#define SC_EXT_UP            0x48
#define SC_EXT_LEFT          0x4B
#define SC_EXT_RIGHT         0x4D
#define SC_EXT_END           0x4F
#define SC_EXT_DOWN          0x50
#define SC_EXT_DELETE        0x53

static char key_buffer[KEYBOARD_BUFFER_SIZE];
static uint16_t buf_head = 0;
static uint16_t buf_tail = 0;

static bool shift_pressed = false;
static bool caps_lock = false;
static bool ctrl_pressed = false;
static bool extended = false;

static const char ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9',  '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
    't',  'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm',  ',', '.', '/', 0,   '*', 0,   ' ',
};

static const char ascii_shift[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(',  ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',
    'T',  'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"',  '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M',  '<', '>', '?', 0,   '*', 0,   ' ',
};

static void buffer_push(char c) {
    uint16_t next = (buf_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != buf_tail) {
        key_buffer[buf_head] = c;
        buf_head = next;
    }
}

static void send_eoi(void) {
    outb(PIC_COMMAND_PORT, PIC_EOI);
}

char scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) {
        return 0;
    }

    bool use_shift = shift_pressed;
    char base = ascii[scancode];
    bool is_letter = (base >= 'a' && base <= 'z');

    if (is_letter && caps_lock) {
        use_shift = !use_shift;
    }

    return use_shift ? ascii_shift[scancode] : ascii[scancode];
}

static uint8_t extended_to_key(uint8_t code) {
    switch (code) {
        case SC_EXT_UP:     return KEY_UP;
        case SC_EXT_DOWN:   return KEY_DOWN;
        case SC_EXT_LEFT:   return KEY_LEFT;
        case SC_EXT_RIGHT:  return KEY_RIGHT;
        case SC_EXT_HOME:   return KEY_HOME;
        case SC_EXT_END:    return KEY_END;
        case SC_EXT_DELETE: return KEY_DELETE;
        default:            return 0;
    }
}

static void handle_extended(uint8_t code, bool released) {
    if (code == SC_LEFT_CTRL) {
        ctrl_pressed = !released;
        return;
    }

    if (released) {
        return;
    }

    uint8_t key = extended_to_key(code);
    if (key != 0) {
        buffer_push((char)key);
    }
}

static bool handle_modifier(uint8_t code, bool released) {
    switch (code) {
        case SC_LEFT_CTRL:
            ctrl_pressed = !released;
            return true;

        case SC_LEFT_SHIFT:
        case SC_RIGHT_SHIFT:
            shift_pressed = !released;
            return true;

        case SC_CAPS_LOCK:
            if (!released) {
                caps_lock = !caps_lock;
            }
            return true;

        default:
            return false;
    }
}

static bool handle_character(uint8_t code) {
    if (ctrl_pressed && code == SC_C) {
        return app_running;
    }

    char c = scancode_to_ascii(code);
    if (c != 0) {
        buffer_push(c);
    }
    return false;
}

void keyboard_handler_c(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == SC_EXTENDED_PREFIX) {
        extended = true;
        send_eoi();
        return;
    }

    bool released = (scancode & SC_RELEASE_MASK) != 0;
    uint8_t code = scancode & ~SC_RELEASE_MASK;
    bool abort_app = false;

    if (extended) {
        extended = false;
        handle_extended(code, released);
    } else if (!handle_modifier(code, released) && !released) {
        abort_app = handle_character(code);
    }

    send_eoi();

    if (abort_app) {
        app_abort();
    }
}

bool keyboard_has_input(void) {
    return buf_head != buf_tail;
}

char keyboard_read_char(void) {
    if (buf_head == buf_tail) {
        return 0;
    }

    char c = key_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}
