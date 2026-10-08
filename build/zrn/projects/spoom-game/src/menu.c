#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "video.h"
#include "game.h"

static uint8_t selected = 1;
static bool in_help = false;

void menu_help(void)
{
    clear();

    print("===========\n");
    print("   HELP    \n");
    print("===========\n");
    print("A/D - Move\n");
    print("ENTER - Shoot\n");
    print("R - Restart\n");
    print("\n> EXIT\n");
}

void menu_text(void)
{
    clear();

    print("===========\n");
    print("   SPOOM   \n");
    print("===========\n");

    print(selected == 1 ? " > START\n" : "   START\n");
    print(selected == 2 ? " > HELP\n"  : "   HELP\n");
    print(selected == 3 ? " > EXIT\n"  : "   EXIT\n");
}

void main_menu(uint8_t ch) {
    bool is_menu_dirty = (ch == 255);

    if (in_help) {
        if (ch == '\n') {
            in_help = false;
            is_menu_dirty = true;
        }
    } else {
        if (ch == 'w') {
            if (selected > 1) {
                selected--;
            } else {
                selected = 3;
            }
            is_menu_dirty = true;
        }

        else if (ch == 's') {
            if (selected < 3) {
                selected++;
            } else {
                selected = 1;
            }
            is_menu_dirty = true;
        }

        else if (ch == '\n') {
            if (selected == 1) {
                game_main();
                is_menu_dirty = true;
            }
            else if (selected == 2) {
                in_help = true;
                is_menu_dirty = true;
            }
            else if (selected == 3) {
                clear();
                // EXIT
            }
        }
    }

    if (is_menu_dirty) {
        if (in_help) {
            menu_help();
        } else {
            menu_text();
        }
    }
}
