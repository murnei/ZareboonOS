#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_handler_main(char* command_buffer);
void command_manager(char* cmd);
unsigned char inb(unsigned short port);
extern volatile char last_key_pressed;
extern int is_editing;
char get_char();
void keyboard_set_edit_mode(int mode);

#endif
