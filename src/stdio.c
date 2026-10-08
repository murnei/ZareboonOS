#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"
#include "string.h"

#include "console.h"
#include "video.h"
#include "cursor.h"

static void print_pad(char ch, int count, uint32_t color) {
    while (count-- > 0) print_char((uint8_t)ch, color);
}

static void print_str(const char* s, int width, bool left, uint32_t color) {
    if (!s) s = "(null)";

    int len = 0;
    while (s[len]) len++;

    int pad = (width > len) ? (width - len) : 0;

    if (!left) print_pad(' ', pad, color);
    while (*s) print_char((uint8_t)*s++, color);
    if (left) print_pad(' ', pad, color);
}

static void print_number(uint32_t uval, bool negative, uint32_t base, bool upper, int width, bool zero, bool left, uint32_t color) {
    char buf[32];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int n = 0;

    if (uval == 0) {
        buf[n++] = '0';
    } else {
        while (uval > 0) {
            buf[n++] = digits[uval % base];
            uval /= base;
        }
    }

    int total = n + (negative ? 1 : 0);
    int pad = (width > total) ? (width - total) : 0;

    if (left) zero = false;

    if (!left && !zero) print_pad(' ', pad, color);
    if (negative) print_char('-', color);
    if (!left && zero) print_pad('0', pad, color);

    while (n > 0) print_char((uint8_t)buf[--n], color);

    if (left) print_pad(' ', pad, color);
}

void print(const char* c, ...) {
    va_list args;
    va_start(args, c);

    uint32_t color = rgb(255, 255, 255);

    uint16_t len = strlen(c);

    for (uint16_t i = 0; i < len; i++) {
        if (c[i] == '\n') {
            new_line();
            continue;
        }

        if (c[i] != '%') {
            print_char((uint8_t)c[i], color);
            continue;
        }

        i++;
        if (c[i] == '\0') break;

        bool zero = false;
        bool left = false;
        while (c[i] == '0' || c[i] == '-') {
            if (c[i] == '0') zero = true;
            else left = true;
            i++;
        }

        int width = 0;
        while (c[i] >= '0' && c[i] <= '9') {
            width = width * 10 + (c[i] - '0');
            i++;
        }

        if (c[i] == '\0') break;

        switch (c[i]) {
            case 's': {
                const char* s = va_arg(args, const char*);
                print_str(s, width, left, color);
                break;
            }
            case 'c': {
                int ch = va_arg(args, int);
                print_pad(' ', left ? 0 : width - 1, color);
                print_char((uint8_t)ch, color);
                if (left) print_pad(' ', width - 1, color);
                break;
            }
            case 'C': {
                color = va_arg(args, uint32_t);
                break;
            }
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                bool neg = val < 0;
                uint32_t uval = neg ? (0u - (uint32_t)val) : (uint32_t)val;
                print_number(uval, neg, 10, false, width, zero, left, color);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                print_number(val, false, 10, false, width, zero, left, color);
                break;
            }
            case 'x': {
                uint32_t val = va_arg(args, uint32_t);
                print_number(val, false, 16, false, width, zero, left, color);
                break;
            }
            case 'X': {
                uint32_t val = va_arg(args, uint32_t);
                print_number(val, false, 16, true, width, zero, left, color);
                break;
            }
            case '%': {
                print_char('%', color);
                break;
            }
            default: {
                print_char('%', color);
                print_char((uint8_t)c[i], color);
                break;
            }
        }
    }

    va_end(args);
    update_cursor();
}

void draw_os_logo(void) {
    print("==========================================================\n");
    print("  #####    ###   #####   #####  #####   ###   ###   #   # \n");
    print("     #    #   #  #    #  #      #    # #   # #   #  ##  # \n");
    print("    #     #####  #####   #####  #####  #   # #   #  # # # \n");
    print("   #      #   #  #   #   #      #    # #   # #   #  #  ## \n");
    print("  #####   #   #  #    #  #####  #####   ###   ###   #   # \n");
    print("==========================================================\n");
    print("               --- ZAREBOON OS v1.2 ---                   \n");
    print("                      by murnei                           \n");
    print("\n");
}
