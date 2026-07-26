#ifndef CURSOR_H
#define CURSOR_H

void update_cursor(void);

void set_cursor(int x, int y);
void move_cursor(int dx, int dy);
void new_line(void);

int get_cursor_x();
int get_cursor_y();

#endif
