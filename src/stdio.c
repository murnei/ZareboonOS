#include "cursor.h"
#include "video.h"

extern void outb(unsigned short port, unsigned char data);

void itoa(int n, char* s, int b) {
    static char digits[] = "0123456789ABCDEF";
    int i = 0;
    int sign = n;
    if (n < 0) n = -n;
    do {
        s[i++] = digits[n % b];
    } while ((n /= b) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char t = s[j]; s[j] = s[k]; s[k] = t;
    }
}

int strlen(const char* s) {
    int length = 0;
    while (s[length] != '\0') {
        length++;
    }
    return length;
}	

void print(const char* s) {
    int len = strlen(s);
    int inputed_color = 0x0F;

    for (int i = 0; i < len; i++) {
        if (s[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else if (s[i] == '\01') {
            inputed_color = 0x0F;
            continue;
        } else {
            put_char(cursor_x, cursor_y, s[i], inputed_color);
            cursor_x++;
        }

        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }

        scroll();
        update_cursor();
    }
}

void print_int(int n) {
    if (n == 0) {
        print("00");
        return;
    }
    if (n < 10) print("0");
    
    char buf[12];
    itoa(n, buf, 10);
    print(buf);
}

void draw_name_os() {
   clear();
   cursor_y = 0;
   cursor_x = 25;
   print("*** ZAREBOON OS ***\n");
}

void draw_os_logo() {
    print("  ______                          _                        \n");
    print(" |___  /                         | |                       \n");
    print("    / / __ _ _ __ ___  ___   ___ | |__   ___   ___  _ __   \n");
    print("   / / / _` | '__/ _ \\/ _ \\ / _ \\| '_ \\ / _ \\ / _ \\| '_ \\  \n");
    print("  / /_| (_| | | |  __/ (_) | (_) | |_) | (_) | (_) | | | | \n");
    print(" /_____\\__,_|_|  \\___|\\___/ \\___/|_.__/ \\___/ \\___/|_| |_| by murnei\n");
    print("\n              --- ZAREBOON OS v1.0 ---\n");
}
