#ifndef VIEW_H
#define VIEW_H

#include "buffer.h"

void view_prompt(const char* prompt, const char* input);
void view_editor(const buffer_t* buffer, const char* name, const char* status);

#endif
