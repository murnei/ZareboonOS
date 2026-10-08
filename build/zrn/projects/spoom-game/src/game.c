#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "map.h"
#include "level.h"
#include "player.h"
#include "keyboard.h"
#include "menu.h"
#include "game.h"

uint8_t cur_lvl = 1;
static bool is_screen_dirty = false;

typedef enum {
    MENU,
    GAME,
} Game_State;

uint8_t get_index_x(uint8_t arr[HEIGHT][WIDTH], uint8_t target) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            if (arr[y][x] == target) {
                return x;
            }
        }
    }
    return 0;
}

uint8_t get_index_y(uint8_t arr[HEIGHT][WIDTH], uint8_t target) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            if (arr[y][x] == target) {
                return y;
            }
        }
    }
    return 0;
}

void defeat(void) {
    print("\nYOU LOSE!\n");
    // EXIT
}

bool game_check(uint8_t arr[HEIGHT][WIDTH]) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            if (arr[y][x] != 0 && arr[y][x] != 2) {
                return false;
            }
        }
    }
    return true;
}

void shoot(uint8_t arr[HEIGHT][WIDTH]) {
    for (int16_t y = player.y; y >= 0; y--) {
        if (arr[y][player.x] == 1) {
            arr[y][player.x] = 0;
            is_screen_dirty = true;
            return;
        }
    }
}

void collision_check(uint8_t arr[HEIGHT][WIDTH]) {
    if (arr[player.y][player.x] == 1) {
        defeat();
    }
}

void update_player(uint8_t arr[HEIGHT][WIDTH]) {
    bool is_player_pos_changed = false;
    uint8_t ch = getch();

    if (ch == 'a') {
        if (player.x > 0) {
            is_player_pos_changed = true;
            player.x--;
        }
    } else if (ch == 'd') {
        if (player.x + 1 < WIDTH) {
            is_player_pos_changed = true;
            player.x++;
        }
    } else if (ch == '\n') {
        shoot(arr);
    } else if (ch == 'r') {
        memcpy(arr, get_lvl(cur_lvl), sizeof(*arr) * HEIGHT);
        player.x = get_index_x(arr, 2);
        player.y = get_index_y(arr, 2);
        is_screen_dirty = true;
    }

    if (is_player_pos_changed) {
        arr[player.y][get_index_x(arr, 2)] = 0;
        arr[player.y][player.x] = 2;
        is_screen_dirty = true;
    }
}

void update_enemies(uint8_t arr[HEIGHT][WIDTH]) {
    static uint16_t tick = 0;
    tick++;
    if (tick < 100) {
        return;
    }

    for (int16_t y = HEIGHT - 1; y >= 0; y--) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            if (arr[y][x] == 1 && y + 1 < HEIGHT) {
                arr[y][x] = 0;
                arr[y + 1][x] = 1;
                is_screen_dirty = true;
            }
        }
    }

    tick = 0;
}

void game_main(void) {
    while (1) {
        uint8_t level_map[HEIGHT][WIDTH];
        const uint8_t (*lvl)[WIDTH] = get_lvl(cur_lvl);

        if (lvl == NULL) {
            print("\nYOU WIN THE GAME!\n");
            break;
        }

        memcpy(level_map, lvl, sizeof(level_map));

        player.x = get_index_x(level_map, 2);
        player.y = get_index_y(level_map, 2);

        is_screen_dirty = true;
        draw_frame(level_map);
        is_screen_dirty = false;

        bool is_level_running = true;

        while (is_level_running) {
            update_player(level_map);
            update_enemies(level_map);
            collision_check(level_map);

            if (game_check(level_map)) {
                print("\nWIN!\n");
                cur_lvl++;
                is_level_running = false;
                break;
            }

            if (is_screen_dirty) {
                draw_frame(level_map);
                is_screen_dirty = false;
            }

            sleep(16);
        }
    }
}

void update(void) {
    Game_State state = MENU;

    main_menu(255);

    while (1) {
        uint8_t ch = getch();

        switch (state) {
            case MENU:
                main_menu(ch);
                break;
            case GAME:
                game_main();
                state = MENU;
                break;
        }
    }
}
