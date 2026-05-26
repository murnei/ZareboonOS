#ifndef MY_STDIO_H
#define MY_STDIO_H

// Прототипы функций
void put_char(int x, int y, char c, int color);
int strlen(const char* s);
void print(const char* s);
void print_int(int n);
void clear();
void draw_name_os();
void draw_os_logo();
void update_cursor(int x, int y);

extern int cursor_x;
extern int cursor_y;
extern unsigned char get_rtc_register(int reg);
extern unsigned char bcd_to_bin(unsigned char bcd);
extern void itoa(int n, char* s, int b);
extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

extern void read_sector(uint32_t lba, uint8_t* buffer);


#endif
