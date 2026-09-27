#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"

#include "console.h"
#include "video.h"

uint16_t strlen(const char* c) {
    int i = 0;

    while(c[i] != '\0') {
        i++;
    }

    return i;
}

void print_str(const char* s, uint32_t color) {
    if (!s) s = "(null)";
    while (*s) print_char((uint8_t)*s++, color);
}

static void print_number(int value, int base, bool is_signed, int uppercase, uint32_t color) {
    char buf[32];
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    int negative = 0;

    uint32_t uval;
    if (is_signed && value < 0) {
        negative = 1;
        uval = (uint32_t)(-value);
    } else {
        uval = (uint32_t)value;
    }

    if (uval == 0) {
        buf[i++] = '0';
    } else {
        while (uval > 0) {
            buf[i++] = digits[uval % base];
            uval /= base;
        }
    }

    if (negative) print_char('-', color);
    while (i > 0) print_char(buf[--i], color);
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

        if (c[i] == '%') {
            i++;

            switch (c[i]) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    print_str(s, color);
                    break;
                }
                case 'c': {
                    const uint8_t s = va_arg(args, const uint8_t);
                    print_char(s, color);
                    break;
                }
                case 'C': {
                    color = va_arg(args, uint32_t);
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    print_number(val, 10, false, 0, color);
                    break;
                }
                case '%': {
                    print_char('%', color);
                    break;
                }
                default: {
                    print_char('%', color);
                    print_char(c[i], color);
                    break;
                }
            }
            continue;
        }
        print_char(c[i], color);
    }
}

void draw_os_logo(void) {
    print("====================================================\n");
    print("  #####    ###   #####   #####  #####   ###   ###   #   # \n");
    print("     #    #   #  #    #  #      #    # #   # #   #  ##  # \n");
    print("    #     #####  #####   #####  #####  #   # #   #  # # # \n");
    print("   #      #   #  #   #   #      #    # #   # #   #  #  ## \n");
    print("  #####   #   #  #    #  #####  #####   ###   ###   #   # \n");
    print("====================================================\n");
    print("               --- ZAREBOON OS v1.0 ---             \n");
    print("                      by murnei                     \n");
    print("\n");
}
