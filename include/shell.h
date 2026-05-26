#ifndef SHELL_H
#define SHELL_H

void format_fat_name(const char* input, char* output);
extern int load_file(char* filename, uint8_t* target_address);
void shell_main();

#endif
