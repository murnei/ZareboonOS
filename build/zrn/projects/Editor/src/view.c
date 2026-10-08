#include "stdint.h"
#include "gfx.h"
#include "font.h"
#include "buffer.h"
#include "view.h"

#define BACKGROUND_COLOR gfx_rgb(0, 0, 0)
#define TEXT_COLOR gfx_rgb(220, 220, 220)
#define ACCENT_COLOR gfx_rgb(90, 160, 255)
#define MUTED_COLOR gfx_rgb(130, 130, 130)
#define CARET_COLOR gfx_rgb(255, 255, 255)
#define MARGIN 1
#define RESERVED_ROWS 2
#define HINT ":wq save+quit  :q quit"
#define HINT_WIDTH 24

typedef struct {
    uint32_t col;
    uint32_t row;
} layout_t;

static uint32_t columns(void) {
    return gfx_width() / font_cell() - 2 * MARGIN;
}

static uint32_t visible_rows(void) {
    return gfx_height() / font_cell() - RESERVED_ROWS;
}

static uint32_t last_row(void) {
    return gfx_height() / font_cell() - 1;
}

static uint32_t room(uint32_t total, uint32_t used) {
    return total > used ? total - used : 0;
}

static uint32_t text_length(const char* text) {
    uint32_t length = 0;
    while (text[length]) length++;
    return length;
}

static void draw_char(uint32_t col, uint32_t row, uint8_t ch, uint32_t color) {
    uint32_t cell = font_cell();
    gfx_glyph((col + MARGIN) * cell, row * cell, ch, color);
}

static void draw_caret(uint32_t col, uint32_t row) {
    uint32_t cell = font_cell();
    gfx_fill((col + MARGIN) * cell, row * cell, cell, cell, CARET_COLOR);
}

static void draw_string(uint32_t col, uint32_t row, const char* text, uint32_t limit, uint32_t color) {
    for (uint32_t i = 0; text[i] && i < limit; i++) {
        draw_char(col + i, row, (uint8_t)text[i], color);
    }
}

static bool layout_next(layout_t* layout, uint8_t ch, uint32_t cols) {
    if (ch == '\n') {
        layout->row++;
        layout->col = 0;
        return false;
    }

    if (layout->col >= cols) {
        layout->row++;
        layout->col = 0;
    }

    return true;
}

static void layout_settle(layout_t* layout, uint32_t cols) {
    if (layout->col >= cols) {
        layout->row++;
        layout->col = 0;
    }
}

static uint32_t total_rows(const buffer_t* buffer, uint32_t cols) {
    layout_t layout = { 0, 0 };

    for (uint32_t i = 0; i < buffer->length; i++) {
        if (layout_next(&layout, buffer->data[i], cols)) layout.col++;
    }

    layout_settle(&layout, cols);
    return layout.row + 1;
}

void view_prompt(const char* prompt, const char* input) {
    uint32_t cols = columns();
    uint32_t prompt_length = text_length(prompt);
    uint32_t input_length = text_length(input);

    gfx_fill(0, 0, gfx_width(), gfx_height(), BACKGROUND_COLOR);
    draw_string(0, 1, prompt, cols, ACCENT_COLOR);
    draw_string(prompt_length, 1, input, room(cols, prompt_length), TEXT_COLOR);
    draw_caret(prompt_length + input_length, 1);
    gfx_present();
}

void view_editor(const buffer_t* buffer, const char* name, const char* status) {
    uint32_t cols = columns();
    uint32_t rows = visible_rows();
    uint32_t total = total_rows(buffer, cols);
    uint32_t first = total > rows ? total - rows : 0;
    layout_t layout = { 0, 0 };

    gfx_fill(0, 0, gfx_width(), gfx_height(), BACKGROUND_COLOR);

    draw_string(0, 0, "EDIT", cols, ACCENT_COLOR);
    draw_string(5, 0, name, room(cols, 5), TEXT_COLOR);

    for (uint32_t i = 0; i < buffer->length; i++) {
        uint8_t ch = buffer->data[i];
        if (!layout_next(&layout, ch, cols)) continue;

        if (layout.row >= first) {
            draw_char(layout.col, layout.row - first + 1, ch, TEXT_COLOR);
        }
        layout.col++;
    }

    layout_settle(&layout, cols);
    draw_caret(layout.col, layout.row - first + 1);

    draw_string(0, last_row(), HINT, cols, MUTED_COLOR);
    draw_string(HINT_WIDTH, last_row(), status, room(cols, HINT_WIDTH), ACCENT_COLOR);

    gfx_present();
}
