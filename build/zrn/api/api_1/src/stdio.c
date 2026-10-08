#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"
#include "gfx.h"
#include "term.h"
#include "cursor.h"

#define TEXT_COLOR gfx_rgb(255, 255, 255)

typedef struct {
    bool left;
    bool zero;
    int width;
} spec_t;

static void emit(char ch, uint32_t color) {
    if (gfx_is_open()) gfx_putc((uint8_t)ch, color);
    else term_putc(ch, color);
}

static void emit_pad(char ch, int count, uint32_t color) {
    while (count-- > 0) emit(ch, color);
}

static void emit_string(const char* text, const spec_t* spec, uint32_t color) {
    if (!text) text = "(null)";

    int length = 0;
    while (text[length]) length++;

    int pad = (spec->width > length) ? (spec->width - length) : 0;

    if (!spec->left) emit_pad(' ', pad, color);
    while (*text) emit(*text++, color);
    if (spec->left) emit_pad(' ', pad, color);
}

static void emit_char(char ch, const spec_t* spec, uint32_t color) {
    if (!spec->left) emit_pad(' ', spec->width - 1, color);
    emit(ch, color);
    if (spec->left) emit_pad(' ', spec->width - 1, color);
}

static void emit_number(uint32_t value, bool negative, uint32_t base, bool upper, const spec_t* spec, uint32_t color) {
    char digits_buffer[32];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int count = 0;

    if (value == 0) {
        digits_buffer[count++] = '0';
    } else {
        while (value > 0) {
            digits_buffer[count++] = digits[value % base];
            value /= base;
        }
    }

    int total = count + (negative ? 1 : 0);
    int pad = (spec->width > total) ? (spec->width - total) : 0;
    bool zero = spec->zero && !spec->left;

    if (!spec->left && !zero) emit_pad(' ', pad, color);
    if (negative) emit('-', color);
    if (zero) emit_pad('0', pad, color);

    while (count > 0) emit(digits_buffer[--count], color);

    if (spec->left) emit_pad(' ', pad, color);
}

static void emit_signed(int value, const spec_t* spec, uint32_t color) {
    bool negative = value < 0;
    uint32_t magnitude = negative ? (0u - (uint32_t)value) : (uint32_t)value;
    emit_number(magnitude, negative, 10, false, spec, color);
}

void print(const char* format, ...) {
    if (!format) return;

    va_list args;
    va_start(args, format);

    uint32_t color = TEXT_COLOR;

    for (const char* p = format; *p; p++) {
        if (*p != '%') {
            emit(*p, color);
            continue;
        }

        p++;
        if (!*p) break;

        spec_t spec = { false, false, 0 };

        while (*p == '0' || *p == '-') {
            if (*p == '0') spec.zero = true;
            else spec.left = true;
            p++;
        }

        while (*p >= '0' && *p <= '9') {
            spec.width = spec.width * 10 + (*p - '0');
            p++;
        }

        if (!*p) break;

        switch (*p) {
            case 's':
                emit_string(va_arg(args, const char*), &spec, color);
                break;
            case 'c':
                emit_char((char)va_arg(args, int), &spec, color);
                break;
            case 'C':
                color = va_arg(args, uint32_t);
                break;
            case 'd':
            case 'i':
                emit_signed(va_arg(args, int), &spec, color);
                break;
            case 'u':
                emit_number(va_arg(args, uint32_t), false, 10, false, &spec, color);
                break;
            case 'x':
                emit_number(va_arg(args, uint32_t), false, 16, false, &spec, color);
                break;
            case 'X':
                emit_number(va_arg(args, uint32_t), false, 16, true, &spec, color);
                break;
            case '%':
                emit('%', color);
                break;
            default:
                emit('%', color);
                emit(*p, color);
                break;
        }
    }

    va_end(args);

    if (gfx_is_open()) {
        cursor_draw();
        gfx_present();
    }
}
