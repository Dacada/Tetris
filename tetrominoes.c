// Includes

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



// Defines

#define DELAY_US 30000L
#define INPUT_TIME_US 100000L

#define TETRIS_PLAYFIELD_X 10
#define TETRIS_PLAYFIELD_Y 40 // this will be drawn as half this value but internally as this value

#define NEXT_RECTANGLE_DRAW_X 12
#define NEXT_RECTANGLE_DRAW_Y 16

#define SCORE_RECTANGLE_DRAW_X 12
#define SCORE_RECTANGLE_DRAW_Y 2

#define LEVEL_RECTANGLE_DRAW_X 12
#define LEVEL_RECTANGLE_DRAW_Y 2

#define HOLD_RECTANGLE_DRAW_X 12
#define HOLD_RECTANGLE_DRAW_Y 6

#define GAMEOVER_RECTANGLE_DRAW_X 60
#define GAMEOVER_RECTANGLE_DRAW_Y 8

#define DRAWING_CHAR '#'

#define SINGLE_SCORE 100
#define DOUBLE_SCORE 300
#define T_SPIN_SCORE 400
#define TRIPLE_SCORE 500
#define TETRIS_SCORE 800
#define SOFT_DROP_SCORE 1
#define HARD_DROP_SCORE 2

#define SINGLE_LEVEL_SCORE 1
#define DOUBLE_LEVEL_SCORE 3
#define TRIPLE_LEVEL_SCORE 5
#define TETRIS_LEVEL_SCORE 8

#define MAX_TOTAL_HISCORE_FILEPATH_LENGTH 1024
#define HISCORE_FILE "tetrominoes/hiscore.txt"



// enums, structs

enum tetris_color {
        TETRIS_COLOR_BLACK,
        TETRIS_COLOR_CYAN,
        TETRIS_COLOR_YELLOW,
        TETRIS_COLOR_PURPLE,
        TETRIS_COLOR_GREEN,
        TETRIS_COLOR_RED,
        TETRIS_COLOR_BLUE,
        TETRIS_COLOR_ORANGE,
        TETRIS_COLOR_WHITE
};

enum tetrimino {
        TETRIMINO_TEST,
        TETRIMINO_I,
        TETRIMINO_O,
        TETRIMINO_T,
        TETRIMINO_S,
        TETRIMINO_Z,
        TETRIMINO_J,
        TETRIMINO_L
};

enum tetrimino_rotation {
        SPAWN_ROTATED,
        CLOCKWISE_ROTATED,
        TWICE_ROTATED,
        COUNTER_ROTATED
};

enum input_type {
        INPUT_NONE,
        INPUT_CLOCKWISE_ROTATION,
        INPUT_HARD_DROP,
        INPUT_HOLD,
        INPUT_COUNTERCLOCKWISE_ROTATION,
        INPUT_SOFT_DROP,
        INPUT_LEFT,
        INPUT_RIGHT,
        INPUT_PAUSE,
        INPUT_EXIT
};

struct point {
        int x;
        int y;
};



// Globals (constants)

static const bool piece_shapes[8][4][4][4] = {
        {
                {
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1}
                },
                {
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1}
                },
                {
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1}
                },
                {
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1},
                        {1,1,1,1}
                }
        },
        {
                {
                        {0,0,0,0},
                        {1,1,1,1},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,0,1,0},
                        {0,0,1,0},
                        {0,0,1,0},
                        {0,0,1,0}
                },
                {
                        {0,0,0,0},
                        {0,0,0,0},
                        {1,1,1,1},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {0,1,0,0},
                        {0,1,0,0},
                        {0,1,0,0}
                }
        },
        {
                {
                        {0,1,1,0},
                        {0,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,1,0},
                        {0,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,1,0},
                        {0,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,1,0},
                        {0,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                }
        },
        {
                {
                        {0,1,0,0},
                        {1,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {0,1,1,0},
                        {0,1,0,0},
                        {0,0,0,0}
                },
                {
                        {0,0,0,0},
                        {1,1,1,0},
                        {0,1,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {1,1,0,0},
                        {0,1,0,0},
                        {0,0,0,0}
                }
        },
        {
                {
                        {0,1,1,0},
                        {1,1,0,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {0,1,1,0},
                        {0,0,1,0},
                        {0,0,0,0}
                },
                {
                        {0,0,0,0},
                        {0,1,1,0},
                        {1,1,0,0},
                        {0,0,0,0}
                },
                {
                        {1,0,0,0},
                        {1,1,0,0},
                        {0,1,0,0},
                        {0,0,0,0}
                }
        },
        {
                {
                        {1,1,0,0},
                        {0,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,0,1,0},
                        {0,1,1,0},
                        {0,1,0,0},
                        {0,0,0,0}
                },
                {
                        {0,0,0,0},
                        {1,1,0,0},
                        {0,1,1,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {1,1,0,0},
                        {1,0,0,0},
                        {0,0,0,0}
                }
        },
        {
                {
                        {1,0,0,0},
                        {1,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,1,0},
                        {0,1,0,0},
                        {0,1,0,0},
                        {0,0,0,0}
                },
                {
                        {0,0,0,0},
                        {1,1,1,0},
                        {0,0,1,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {0,1,0,0},
                        {1,1,0,0},
                        {0,0,0,0}
                }
        },
        {
                {
                        {0,0,1,0},
                        {1,1,1,0},
                        {0,0,0,0},
                        {0,0,0,0}
                },
                {
                        {0,1,0,0},
                        {0,1,0,0},
                        {0,1,1,0},
                        {0,0,0,0}
                },
                {
                        {0,0,0,0},
                        {1,1,1,0},
                        {1,0,0,0},
                        {0,0,0,0}
                },
                {
                        {1,1,0,0},
                        {0,1,0,0},
                        {0,1,0,0},
                        {0,0,0,0}
                }
        }
};

static const struct point wall_kicks[2][8][5] = {
        // All pieces except O and I
        {
                // 0 => 1
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {-1,1},
                        
                        // Test 4
                        {0,-2},
                        
                        // Test 5
                        {-1,-2}
                },
                
                // 1 => 0
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {1,-1},
                        
                        // Test 4
                        {0,2},
                        
                        // Test 5
                        {1,2}
                },
                
                // 1 => 2
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {1,-1},
                        
                        // Test 4
                        {0,2},
                        
                        // Test 5
                        {1,2}
                },
                
                // 2 => 1
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {-1,1},
                        
                        // Test 4
                        {0,-2},
                        
                        // Test 5
                        {-1,-2}
                },
                
                // 2 => 3
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {1,1},
                        
                        // Test 4
                        {0,-2},
                        
                        // Test 5
                        {1,-2}
                },
                
                // 3 => 2
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {-1,-1},
                        
                        // Test 4
                        {0,2},
                        
                        // Test 5
                        {-1,2}
                },
                
                // 3 => 0
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {-1,-1},
                        
                        // Test 4
                        {0,2},
                        
                        // Test 5
                        {-1,2}
                },
                
                // 0 => 3
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {1,1},
                        
                        // Test 4
                        {0,-2},
                        
                        // Test 5
                        {1,-2}
                }
        },

        // I piece
        {
                // 0 => 1
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-2,0},
                        
                        // Test 3
                        {1,0},
                        
                        // Test 4
                        {-2,-1},
                        
                        // Test 5
                        {1,2}
                },
                
                // 1 => 0
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {2,0},
                        
                        // Test 3
                        {-1,0},
                        
                        // Test 4
                        {2,1},
                        
                        // Test 5
                        {-1,-2}
                },
                
                // 1 => 2
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {2,0},
                        
                        // Test 4
                        {-1,2},
                        
                        // Test 5
                        {2,-1}
                },
                
                // 2 => 1
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {-2,0},
                        
                        // Test 4
                        {1,-2},
                        
                        // Test 5
                        {-2,1}
                },
                
                // 2 => 3
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {2,0},
                        
                        // Test 3
                        {-1,0},
                        
                        // Test 4
                        {2,1},
                        
                        // Test 5
                        {-1,-2}
                },
                
                // 3 => 2
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-2,0},
                        
                        // Test 3
                        {1,0},
                        
                        // Test 4
                        {-2,-1},
                        
                        // Test 5
                        {1,2}
                },
                
                // 3 => 0
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {1,0},
                        
                        // Test 3
                        {-2,0},
                        
                        // Test 4
                        {1,-2},
                        
                        // Test 5
                        {-2,1}
                },
                
                // 0 => 3
                {
                        // Test 1
                        {0,0},
                        
                        // Test 2
                        {-1,0},
                        
                        // Test 3
                        {2,0},
                        
                        // Test 4
                        {-1,2},
                        
                        // Test 5
                        {2,-1}
                }
        }
};



// Globals (game state)

static enum tetrimino spawn_order[] = {
        TETRIMINO_I,
        TETRIMINO_O,
        TETRIMINO_T,
        TETRIMINO_S,
        TETRIMINO_Z,
        TETRIMINO_J,
        TETRIMINO_L,
        
        TETRIMINO_I,
        TETRIMINO_O,
        TETRIMINO_T,
        TETRIMINO_S,
        TETRIMINO_Z,
        TETRIMINO_J,
        TETRIMINO_L
};

static int spawn_next_i = 0;

static long score;
static long hiscore;

static enum tetrimino current_held_piece = TETRIMINO_TEST;

static enum tetris_color playfield[TETRIS_PLAYFIELD_Y][TETRIS_PLAYFIELD_X];

static enum tetrimino current_piece = TETRIMINO_TEST;
static enum tetrimino_rotation current_piece_rotation = SPAWN_ROTATED;
static struct point current_piece_location = { 5, 20 };

static struct point current_shadow_location;

static bool paused = false;

static bool can_hold = true;
static bool hard_dropped = false;

static bool last_movement_was_spin = false;

static long us_until_next_step = 1000000L;

static unsigned level = 1;
static int goal = 5;



// Utils functions

static void shuffle(enum tetrimino *arr, int size) {
        for (int i=0; i<size; i++) {
                int j = random() % size;
                enum tetrimino tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
        }
}

// ensure we don't wrte *dest beyond *n characters and at the end modify *n to
// indicate how many characters of src we wrote. If we didn't write fully dest,
// return false otherwise true
static bool mystrncpy(char **dest, const char *src, size_t *n) {
        while (*src != '\0') {
                if (*n <= 0) {
                        return false;
                }
                
                **dest = *src;
                
                (*dest)++;
                src++;
                
                (*n)--;
        }

        return true;
}

static bool create_directory_if_not_exists(const char *dir) {
        struct stat st;
        
        if (stat(dir, &st) == -1) {
                if (mkdir(dir, 0700) == -1) {
                        perror(dir);
                        return false;
                }
        }

        return true;
}

static bool make_directory_exist(const char *filename) {
        char *directory = strdup(filename);
        
        size_t i = 0;
        for (;;) {
                while (directory[i] != '/' && directory[i] != '\0') {
                        i++;
                }
                
                if (directory[i] == '\0' || directory[i+1] == '\0') {
                        break;
                }
                
                i++;
                char c = directory[i];
                directory[i] = '\0';
                if (!create_directory_if_not_exists(directory)) {
                        return false;
                }
                directory[i] = c;
        }
        
        free(directory);
        return true;
}

static void endwin_wrapper(void) {
        endwin();
}



// Colors functions

static void setup_colors(void) {
        start_color();
        
        init_pair(1, COLOR_CYAN, COLOR_CYAN);
        init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(4, COLOR_GREEN, COLOR_GREEN);
        init_pair(5, COLOR_RED, COLOR_RED);
        init_pair(6, COLOR_BLUE, COLOR_BLUE);
        init_pair(7, COLOR_YELLOW, COLOR_YELLOW); // This should be orange, but we don't have this color.
        
        init_pair(8, COLOR_BLACK, COLOR_BLACK);
        init_pair(9, COLOR_WHITE, COLOR_WHITE);
        
        init_pair(10, COLOR_CYAN, COLOR_BLACK);
        init_pair(11, COLOR_YELLOW, COLOR_BLACK);
        init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(13, COLOR_GREEN, COLOR_BLACK);
        init_pair(14, COLOR_RED, COLOR_BLACK);
        init_pair(15, COLOR_BLUE, COLOR_BLACK);
        init_pair(16, COLOR_YELLOW, COLOR_BLACK); // This should be orange, but we don't have this color.
}

static void enable_color(enum tetris_color c, bool shadow) {
        int add;
        if (shadow)
                add = 9;
        else
                add = 0;
        
        switch (c) {
        case TETRIS_COLOR_CYAN:
                attron(COLOR_PAIR(1 + add)); break;
        case TETRIS_COLOR_YELLOW:
                attron(COLOR_PAIR(2 + add)); break;
        case TETRIS_COLOR_PURPLE:
                attron(COLOR_PAIR(3 + add)); break;
        case TETRIS_COLOR_GREEN:
                attron(COLOR_PAIR(4 + add)); break;
        case TETRIS_COLOR_RED:
                attron(COLOR_PAIR(5 + add)); break;
        case TETRIS_COLOR_BLUE:
                attron(COLOR_PAIR(6 + add)); break;
        case TETRIS_COLOR_ORANGE:
                attron(COLOR_PAIR(7 + add)); break;
        case TETRIS_COLOR_BLACK:
                attron(COLOR_PAIR(8 + add)); break;
        case TETRIS_COLOR_WHITE:
                attron(COLOR_PAIR(9 + add)); break;
        }
}

static void disable_color(enum tetris_color c, bool shadow) {
        int add;
        if (shadow)
                add = 9;
        else
                add = 0;
        
        switch (c) {
        case TETRIS_COLOR_CYAN:
                attroff(COLOR_PAIR(1 + add)); break;
        case TETRIS_COLOR_YELLOW:
                attroff(COLOR_PAIR(2 + add)); break;
        case TETRIS_COLOR_PURPLE:
                attroff(COLOR_PAIR(3 + add)); break;
        case TETRIS_COLOR_GREEN:
                attroff(COLOR_PAIR(4 + add)); break;
        case TETRIS_COLOR_RED:
                attroff(COLOR_PAIR(5 + add)); break;
        case TETRIS_COLOR_BLUE:
                attroff(COLOR_PAIR(6 + add)); break;
        case TETRIS_COLOR_ORANGE:
                attroff(COLOR_PAIR(7 + add)); break;
        case TETRIS_COLOR_BLACK:
                attroff(COLOR_PAIR(8 + add)); break;
        case TETRIS_COLOR_WHITE:
                attroff(COLOR_PAIR(9 + add)); break;
        }
}

static enum tetris_color piece_color(enum tetrimino t) {
        switch (t) {
        case TETRIMINO_TEST:
                return TETRIS_COLOR_PURPLE;
        case TETRIMINO_I:
                return TETRIS_COLOR_CYAN;
        case TETRIMINO_O:
                return TETRIS_COLOR_YELLOW;
        case TETRIMINO_T:
                return TETRIS_COLOR_PURPLE;
        case TETRIMINO_S:
                return TETRIS_COLOR_GREEN;
        case TETRIMINO_Z:
                return TETRIS_COLOR_RED;
        case TETRIMINO_J:
                return TETRIS_COLOR_BLUE;
        case TETRIMINO_L:
                return TETRIS_COLOR_ORANGE;
        default:
                return TETRIS_COLOR_WHITE;
        }
}



// Drawing functions

static void decide_rotation_offset_draw_tetrimino(enum tetrimino t,
                                                  enum tetrimino_rotation *r, struct point *o) {
        switch(t) {
        case TETRIMINO_TEST:
                *r = SPAWN_ROTATED;
                o->x = 0;
                o->y = 0;
                break;
        case TETRIMINO_I:
                *r = SPAWN_ROTATED;
                o->x = -2;
                o->y = 1;
                break;
        case TETRIMINO_O:
                *r = SPAWN_ROTATED;
                o->x = -2;
                o->y = 1;
                break;
        case TETRIMINO_T:
                *r = SPAWN_ROTATED;
                o->x = -1;
                o->y = 1;
                break;
        case TETRIMINO_S:
                *r = SPAWN_ROTATED;
                o->x = -1;
                o->y = 1;
                break;
        case TETRIMINO_Z:
                *r = SPAWN_ROTATED;
                o->x = -1;
                o->y = 1;
                break;
        case TETRIMINO_J:
                *r = SPAWN_ROTATED;
                o->x = -1;
                o->y = 1;
                break;
        case TETRIMINO_L:
                *r = SPAWN_ROTATED;
                o->x = -1;
                o->y = 1;
                break;
        }
}

static void draw(struct point st, struct point ed, enum tetris_color c) {
        enable_color(c, false);
        for (int x=st.x; x<ed.x; x++) {
                for (int y=st.y; y<ed.y; y++) {
                        mvaddch(y, x, DRAWING_CHAR);
                }
        }
        disable_color(c, false);
}

static void draw_tetrimino(enum tetrimino t, struct point c) {
        enable_color(piece_color(t), false);
        
        enum tetrimino_rotation r = SPAWN_ROTATED;
        struct point o = {0, 0};
        decide_rotation_offset_draw_tetrimino(t, &r, &o);
        
        for (int j=0; j<4; j++) {
                for (int i=0; i<4; i++) {
                        if (piece_shapes[t][r][j][i]) {
                                mvaddch(c.y+j+o.y, c.x+i*2+o.x, DRAWING_CHAR);
                                mvaddch(c.y+j+o.y, c.x+i*2+1+o.x, DRAWING_CHAR);
                        }
                        
                }
        }

        disable_color(piece_color(t), false);
}

static void draw_background(struct point max) {
        draw((struct point){0, 0}, max, TETRIS_COLOR_WHITE);
}

static void draw_playfield(struct point st, struct point ed) {
        if (paused) {
                struct point bst;
                bst.x = st.x;
                bst.y = st.y + TETRIS_PLAYFIELD_Y/2;
                draw(bst, ed, TETRIS_COLOR_BLACK);
                
                const char text[] = "PAUSED";
                const unsigned textlen = sizeof(text);
                int x = st.x + (ed.x - st.x)/2 - textlen/2;
                int y = st.y + 3*(ed.y - st.y)/4;
                mvprintw(y, x, text);
                return;
        }
        
        for (int j=TETRIS_PLAYFIELD_Y/2; j<TETRIS_PLAYFIELD_Y; j++) {
                for (int i=0; i<TETRIS_PLAYFIELD_X; i++) {
                        struct point c;
                        c.x = i - current_piece_location.x;
                        c.y = j - current_piece_location.y;

                        struct point s;
                        s.x = i - current_shadow_location.x;
                        s.y = j - current_shadow_location.y;

                        bool piece_in_range = (c.x >= 0 && c.x < 4 && c.y >= 0 && c.y < 4);
                        bool shadow_in_range = (s.x >= 0 && s.x < 4 && s.y >= 0 && s.y < 4);
                        
                        enum tetris_color color;
                        bool shadow;
                        if (piece_in_range && piece_shapes[current_piece][current_piece_rotation][c.y][c.x]) {
                                color = piece_color(current_piece);
                                shadow = false;
                        } else if (shadow_in_range && piece_shapes[current_piece][current_piece_rotation][s.y][s.x]) {
                                color = piece_color(current_piece);
                                shadow = true;
                        } else {
                                color = playfield[j][i];
                                shadow = false;
                        }
                        
                        enable_color(color, shadow);
                        mvaddch(st.y+j, st.x+i*2, DRAWING_CHAR);
                        mvaddch(st.y+j, st.x+i*2+1, DRAWING_CHAR);
                        disable_color(color, shadow);
                }
        }
}

static void draw_nextarea(struct point st, struct point ed) {
        draw(st, ed, TETRIS_COLOR_BLACK);
        mvprintw(st.y+0, st.x+1, "Next");

        if (paused) {
                return;
        }

        int midx = st.x + (ed.x - st.x)/2;
        int midy = st.y + (ed.y - st.y)/2;
        int midy1 = st.y + (midy - st.y)/2;
        int midy2 = midy;
        int midy3 = midy + (ed.y - midy)/2;

        struct point tst1, tst2, tst3;
        tst1.x = tst2.x = tst3.x = midx - 2;
        tst1.y = midy1 - 2;
        tst2.y = midy2 - 2;
        tst3.y = midy3 - 2;

        draw_tetrimino(spawn_order[spawn_next_i], tst1);
        draw_tetrimino(spawn_order[spawn_next_i+1], tst2);
        draw_tetrimino(spawn_order[spawn_next_i+2], tst3);
}

static void draw_scorearea(struct point st, struct point ed) {
        draw(st, ed, TETRIS_COLOR_BLACK);

        mvprintw(st.y+0, st.x+1, "Score");
        mvprintw(st.y+1, st.x+1, "%010ld", score);
}

static void draw_hiscorearea(struct point st, struct point ed) {
        draw(st, ed, TETRIS_COLOR_BLACK);

        mvprintw(st.y+0, st.x+1, "Hi-Score");
        mvprintw(st.y+1, st.x+1, "%010ld", hiscore);
}

static void draw_levelarea(struct point st, struct point ed) {
        draw(st, ed, TETRIS_COLOR_BLACK);
        
        mvprintw(st.y+0, st.x+1, "Level");
        mvprintw(st.y+1, st.x+1, "%10d", level);
}

static void draw_holdarea(struct point st, struct point ed) {
        draw(st, ed, TETRIS_COLOR_BLACK);
        mvprintw(st.y+0, st.x+1, "Hold");

        if (paused) {
                return;
        }
        
        if (current_held_piece != TETRIMINO_TEST) {
                struct point tst;
                tst.x = st.x + (ed.x - st.x)/2 - 2;
                tst.y = st.y + (ed.y - st.y)/2 - 2;
                draw_tetrimino(current_held_piece, tst);
        }
}

static void draw_controlsarea(struct point st, struct point ed) {
        int msglen = 32; // x/z:rotate c:hold p:pause q:quit
        int total_space = ed.x-st.x;
        int spare_space = total_space - msglen;
        int margin = spare_space / 2;
                
        draw(st, ed, TETRIS_COLOR_BLACK);

        int i=0;
        
        attron(A_BOLD);
        mvaddch(st.y, st.x + margin + i, 'x'); i++;
        attroff(A_BOLD);
        
        mvaddch(st.y, st.x + margin + i, '/'); i++;
        
        attron(A_BOLD);
        mvaddch(st.y, st.x + margin + i, 'z'); i++;
        attroff(A_BOLD);
        
        mvprintw(st.y, st.x + margin + i, ":rotate "); i+=strlen(":rotate ");
        
        attron(A_BOLD);
        mvaddch(st.y, st.x + margin + i, 'c'); i++;
        attroff(A_BOLD);
        
        mvprintw(st.y, st.x + margin + i, ":hold "); i+=strlen(":hold ");
        
        attron(A_BOLD);
        mvaddch(st.y, st.x + margin + i, 'p'); i++;
        attroff(A_BOLD);
        
        mvprintw(st.y, st.x + margin + i, ":pause "); i+=strlen(":pause ");
        
        attron(A_BOLD);
        mvaddch(st.y, st.x + margin + i, 'q'); i++;
        attroff(A_BOLD);
        
        mvprintw(st.y, st.x + margin + i, ":quit "); i+=strlen(":quit");
}

static void draw_screen(void) {
        struct point max;
        getmaxyx(stdscr, max.y, max.x); // it's a macro
        
        // center horizontally as if it was double its actual width
        struct point pf_st, pf_ed;
        pf_st.x = max.x/2 - TETRIS_PLAYFIELD_X;
        pf_ed.x = max.x/2 + TETRIS_PLAYFIELD_X;
        
        // center vertically as if it was half its actual length
        pf_st.y = max.y/2 - TETRIS_PLAYFIELD_Y/2 - TETRIS_PLAYFIELD_Y/4;
        pf_ed.y = max.y/2 + TETRIS_PLAYFIELD_Y/2 - TETRIS_PLAYFIELD_Y/4;
        
        int pf_sty_visible = pf_st.y + TETRIS_PLAYFIELD_Y/2;

        struct point sc_st, sc_ed;
        sc_st.x = pf_ed.x + 3;
        sc_ed.x = sc_st.x + SCORE_RECTANGLE_DRAW_X;
        sc_st.y = pf_sty_visible;
        sc_ed.y = sc_st.y + SCORE_RECTANGLE_DRAW_Y;

        struct point nx_st, nx_ed;
        nx_st.x = sc_st.x;
        nx_ed.x = nx_st.x + NEXT_RECTANGLE_DRAW_X;
        nx_st.y = sc_ed.y + 2;
        nx_ed.y = nx_st.y + NEXT_RECTANGLE_DRAW_Y;

        struct point hd_st, hd_ed;
        hd_ed.x = pf_st.x - 3;
        hd_st.x = hd_ed.x - HOLD_RECTANGLE_DRAW_X;
        hd_st.y = pf_sty_visible;
        hd_ed.y = hd_st.y + HOLD_RECTANGLE_DRAW_Y;

        struct point hs_st, hs_ed;
        hs_st.x = hd_st.x;
        hs_ed.x = hs_st.x + SCORE_RECTANGLE_DRAW_X;
        hs_ed.y = pf_ed.y;
        hs_st.y = hs_ed.y - SCORE_RECTANGLE_DRAW_Y;

        struct point cs_st, cs_ed;
        cs_st.x = hd_st.x;
        cs_ed.x = nx_ed.x;
        cs_st.y = pf_ed.y + 1;
        cs_ed.y = cs_st.y + 1;

        struct point lv_st, lv_ed;
        lv_st.x = hs_st.x;
        lv_ed.x = lv_st.x + LEVEL_RECTANGLE_DRAW_X;
        lv_ed.y = hs_st.y - 2;
        lv_st.y = lv_ed.y - LEVEL_RECTANGLE_DRAW_Y;
        
        draw_background(max);
        draw_playfield(pf_st, pf_ed);
        draw_nextarea(nx_st, nx_ed);
        draw_scorearea(sc_st, sc_ed);
        draw_hiscorearea(hs_st, hs_ed);
        draw_holdarea(hd_st, hd_ed);
        draw_controlsarea(cs_st, cs_ed);
        draw_levelarea(lv_st, lv_ed);
}

static void draw_gameover_screen(void) {
        struct point max;
        getmaxyx(stdscr, max.y, max.x);

        struct point st, ed;
        st.x = max.x/2 - GAMEOVER_RECTANGLE_DRAW_X/2;
        ed.x = max.x/2 + GAMEOVER_RECTANGLE_DRAW_X/2;

        st.y = max.y/2 - GAMEOVER_RECTANGLE_DRAW_Y/2;
        ed.y = max.y/2 + GAMEOVER_RECTANGLE_DRAW_Y/2;

        draw(st, ed, TETRIS_COLOR_BLACK);

        struct point st1, ed1;
        st1.x = st.x + 1;
        st1.y = st.y + 1;
        ed1.x = st.x - 1;
        ed1.y = st.y - 1;
        
        draw(st1, ed1, TETRIS_COLOR_RED);

        struct point st2, ed2;
        st2.x = st.x + 2;
        st2.y = st.y + 2;
        ed2.x = st.x - 2;
        ed2.y = st.y - 2;
        
        draw(st2, ed2, TETRIS_COLOR_BLACK);

        static const char text[] = "GAME OVER";
        static const unsigned textlen = sizeof(text) - 1;

        struct point t;
        t.x = st.x + (ed.x - st.x)/2 - textlen/2;
        t.y = st.y + (ed.y - st.y)/2;
        mvprintw(t.y, t.x, text);
}

static void draw_gameover(void) {
        draw_screen();
        draw_gameover_screen();
}



// Input functions

static enum input_type get_player_input(int c) {
        switch(c) {
        case ERR:
                return INPUT_NONE;
                
        case KEY_UP:
        case 'X':
        case 'x':
                return INPUT_CLOCKWISE_ROTATION;

        case ' ':
                return INPUT_HARD_DROP;

        case 'C':
        case 'c':
                return INPUT_HOLD;

        case 'Z':
        case 'z':
                return INPUT_COUNTERCLOCKWISE_ROTATION;

        case KEY_DOWN:
                return INPUT_SOFT_DROP;

        case KEY_LEFT:
                return INPUT_LEFT;

        case KEY_RIGHT:
                return INPUT_RIGHT;
                
        case 'P':
        case 'p':
                return INPUT_PAUSE;

        case 'Q':
        case 'q':
                return INPUT_EXIT;

        default:
                return INPUT_NONE;
        }
}



static bool gameover_wait_input(void) {
        static long us_until_next_read = INPUT_TIME_US;
        
        int c = getch();
        if (us_until_next_read > 0) {
                us_until_next_read -= DELAY_US;
                return false;
        }
        return c != ERR;
}



// Hiscore functions

static bool get_hiscore_filename(char *result, size_t maxsize) {
        const char *base = getenv("XDG_DATA_HOME");
        if (base == NULL) {
                const char *home = getenv("HOME");
                if (home == NULL) {
                        return false;
                }

                if (!mystrncpy(&result, home, &maxsize)) {
                        return false;
                }
                if (!mystrncpy(&result, "/.local/share", &maxsize)) {
                        return false;
                }
        } else {
                if (!mystrncpy(&result, base, &maxsize)) {
                        return false;
                }
        }

        if (!mystrncpy(&result, "/" HISCORE_FILE, &maxsize)) {
                return false;
        }
        return true;
}

static void init_hiscore(void) {
        hiscore = 0;
        
        static char hiscore_filename[MAX_TOTAL_HISCORE_FILEPATH_LENGTH];
        if (get_hiscore_filename(hiscore_filename, MAX_TOTAL_HISCORE_FILEPATH_LENGTH)) {
                FILE *f = fopen(hiscore_filename, "r");
                if (f != NULL) {
                        fscanf(f, "%ld", &hiscore);
                        fclose(f);
                } else {
                        perror("Hiscore was not loaded");
                }
        } else {
                fprintf(stderr, "Hiscore was not loaded: Could not safely determine path.\n");
        }
}

static void save_hiscore(void) {
        static char hiscore_filename[MAX_TOTAL_HISCORE_FILEPATH_LENGTH];
        if (get_hiscore_filename(hiscore_filename, MAX_TOTAL_HISCORE_FILEPATH_LENGTH)) {
                if (make_directory_exist(hiscore_filename)) {
                        FILE *f = fopen(hiscore_filename, "w");
                        if (f != NULL) {
                                fprintf(f, "%ld", hiscore);
                                fclose(f);
                        } else {
                                perror("Hiscore was not saved");
                        }
                } else {
                        fprintf(stderr, "Hiscore was not saved: Could not create parent directory.\n");
                }
        } else {
                fprintf(stderr, "Hiscore was not saved: Could not safely determine path.\n");
        }
}

static void update_score(long value) {
        if (value == SOFT_DROP_SCORE || value == HARD_DROP_SCORE) {
                score += value;
        } else {
                score += value * level;
        }
        
        if (score > hiscore) {
                hiscore = score;
        }
}



// Piece generation functions

static void init_shuffle_pieces(void) {
        shuffle(spawn_order, 7);
}

static enum tetrimino next_random_piece(void) {
        if (spawn_next_i == 7) {
                spawn_next_i = 0;
                memcpy(spawn_order, spawn_order+7, 7*sizeof(enum tetrimino));
        }
                
        if (spawn_next_i == 0)
                shuffle(spawn_order+7, 7);
                
        return spawn_order[spawn_next_i++];
}



// Gameplay functions

static long get_step_time(void) {
        double time = pow(0.8 - ((double)level - 1) * 0.007, (double)level - 1) * 1e6;
        return (long)time;
}

static void update_level(int lines_score) {
        goal -= lines_score;
        while (goal <= 0) {
                goal += level * 5;
                level++;
        }
}

static bool collision(enum tetrimino piece, enum tetrimino_rotation rotation, struct point location) {
        for (int j=0; j<4; j++) {
                for (int i=0; i<4; i++) {
                        struct point c;
                        c.x = location.x + i;
                        c.y = location.y + j;
                        
                        if (piece_shapes[piece][rotation][j][i]) {
                                if (c.x < 0 || c.x >= TETRIS_PLAYFIELD_X)
                                        return true;
                                if (c.y < 0 || c.y >= TETRIS_PLAYFIELD_Y)
                                        return true;
                                
                                enum tetris_color color = playfield[c.y][c.x];
                        
                                if (color != TETRIS_COLOR_BLACK)
                                        return true;
                        }
                }
        }

        return false;
}

static void update_shadow_location(void) {
        current_shadow_location = current_piece_location;
        while (!collision(current_piece, current_piece_rotation, current_shadow_location)) {
                current_shadow_location.y++;
        }
        current_shadow_location.y--;
}

static bool has_full_lines(int *line) {
        bool has_full_lines;
        for (int y=0; y<TETRIS_PLAYFIELD_Y; y++) {
                has_full_lines = true;
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        if (playfield[y][x] == TETRIS_COLOR_BLACK) {
                                has_full_lines = false;
                                break;
                        }
                }
                if (has_full_lines) {
                        *line = y;
                        return true;
                }
        }
        return false;
}

static int clear_full_lines(void) {
        int full_lines_count = 0;

        int y;
        while (has_full_lines(&y)) {
                full_lines_count++;
                memmove(playfield[1], playfield[0], sizeof(enum tetris_color)*TETRIS_PLAYFIELD_X*y);
        }
        
        return full_lines_count;
}

__attribute__((noreturn))
static void gameover_loop(void) {
        for (;;) {
                draw_gameover();
                if (gameover_wait_input()) {
                        exit(EXIT_SUCCESS);
                }
                usleep(DELAY_US);
        }
}

static void step(void) {
        if (paused)
                return;
        
        if (us_until_next_step <= 0) {
                us_until_next_step = get_step_time();
                current_piece_location.y += 1;
                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                        hard_dropped = false;
                        
                        // Detect T-spin
                        if (current_piece == TETRIMINO_T && last_movement_was_spin) {
                                int x = current_piece_location.x;
                                int y = current_piece_location.y;

                                int count = 0;
                                if (playfield[y][x] != TETRIS_COLOR_BLACK)
                                        count++;
                                if (playfield[y][x+2] != TETRIS_COLOR_BLACK)
                                        count++;
                                if (playfield[y+2][x+2] != TETRIS_COLOR_BLACK)
                                        count++;
                                if (playfield[y+2][x] != TETRIS_COLOR_BLACK)
                                        count++;

                                if (count >= 3)
                                        update_score(T_SPIN_SCORE);
                        }
                        last_movement_was_spin = false;

                        // Add piece to playfield
                        for (int j=0; j<4; j++) {
                                for (int i=0; i<4; i++) {
                                        if (piece_shapes[current_piece][current_piece_rotation][j][i]) {
                                                int x = current_piece_location.x + i;
                                                int y = current_piece_location.y + j - 1;
                                                playfield[y][x] = piece_color(current_piece);
                                        }
                                }
                        }

                        int full_lines_count = clear_full_lines();
                        switch (full_lines_count) {
                        case 1:
                                update_score(SINGLE_SCORE);
                                update_level(SINGLE_LEVEL_SCORE);
                                break;
                        case 2:
                                update_score(DOUBLE_SCORE);
                                update_level(DOUBLE_LEVEL_SCORE);
                                break;
                        case 3:
                                update_score(TRIPLE_SCORE);
                                update_level(TRIPLE_LEVEL_SCORE);
                                break;
                        case 4:
                                update_score(TETRIS_SCORE);
                                update_level(TETRIS_LEVEL_SCORE);
                                break;
                        }
                        
                        current_piece = next_random_piece();
                        current_piece_rotation = SPAWN_ROTATED;
                        current_piece_location.x = 5;
                        current_piece_location.y = 20;
                        can_hold = true;
                        update_shadow_location();

                        if (collision(current_piece, current_piece_rotation, current_piece_location))
                                gameover_loop();
                }
        } else {
                us_until_next_step -= DELAY_US;
        }
}

static bool rotate(enum tetrimino_rotation next) {
        enum tetrimino_rotation curr = current_piece_rotation;

        int i;
        switch(current_piece) {
        case TETRIMINO_TEST:
        case TETRIMINO_O:
                current_piece_rotation = next;
                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                        current_piece_rotation = curr;
                        return false;
                } else {
                        return true;
                }
        case TETRIMINO_I:
                i = 1;
                break;
                
        case TETRIMINO_T:
        case TETRIMINO_S:
        case TETRIMINO_Z:
        case TETRIMINO_J:
        case TETRIMINO_L:
                i = 0;
                break;

        default:
                return false;
        }

        int j;
        if (curr == SPAWN_ROTATED && next == CLOCKWISE_ROTATED)
                j = 0;
        else if (curr == CLOCKWISE_ROTATED && next == SPAWN_ROTATED)
                j = 1;
        else if (curr == CLOCKWISE_ROTATED && next == TWICE_ROTATED)
                j = 2;
        else if (curr == TWICE_ROTATED && next == CLOCKWISE_ROTATED)
                j = 3;
        else if (curr == TWICE_ROTATED && next == COUNTER_ROTATED)
                j = 4;
        else if (curr == COUNTER_ROTATED && next == TWICE_ROTATED)
                j = 5;
        else if (curr == COUNTER_ROTATED && next == SPAWN_ROTATED)
                j = 6;
        else if (curr == SPAWN_ROTATED && next == COUNTER_ROTATED)
                j = 7;
        else
                return false;

        struct point curr_coords = current_piece_location;
        for (int k=0; k<5; k++) {
                struct point c = wall_kicks[i][j][k];

                current_piece_rotation = next;
                current_piece_location.x += c.x;
                current_piece_location.y -= c.y; // in the copy pasted data, up is positive but here down is positive

                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                        current_piece_rotation = curr;
                        current_piece_location = curr_coords;
                } else {
                        return true;
                }
        }

        return false;
}

static void process_input(void) {
        static long us_until_next_read = INPUT_TIME_US;
        
        if (us_until_next_read > 0)
                us_until_next_read -= DELAY_US;

        int c = getch();
        if (us_until_next_read > 0) {
                return;
        }

        enum input_type t = get_player_input(c);

        if (t == INPUT_EXIT)
                exit(EXIT_SUCCESS);

        if (t == INPUT_PAUSE)
                paused = !paused;

        if (paused)
                return;

        if (!hard_dropped) {
                switch(t) {
                case INPUT_CLOCKWISE_ROTATION:
                        if (rotate((current_piece_rotation + 1) % 4)) {
                                last_movement_was_spin = true;
                                //us_until_next_step = STEP_TIME_US;
                                update_shadow_location();
                        }
                        break;
                
                case INPUT_HARD_DROP:
                        while (!collision(current_piece, current_piece_rotation, current_piece_location)) {
                                current_piece_location.y++;
                                update_score(HARD_DROP_SCORE);
                        }
                        current_piece_location.y--;
                        us_until_next_step = get_step_time();
                        hard_dropped = true;
                        break;
                
                case INPUT_HOLD:
                        if (can_hold) {
                                enum tetrimino held = current_held_piece;
                                current_held_piece = current_piece;
                                if (held == TETRIMINO_TEST)
                                        current_piece = next_random_piece();
                                else
                                        current_piece = held;
                                current_piece_rotation = SPAWN_ROTATED;
                                current_piece_location.x = 5;
                                current_piece_location.y = 20;
                                us_until_next_step = get_step_time();
                                can_hold = false;
                                last_movement_was_spin = false;
                                update_shadow_location();
                        }
                        break;
                
                case INPUT_COUNTERCLOCKWISE_ROTATION:
                        if (rotate((current_piece_rotation - 1) % 4)) {
                                last_movement_was_spin = true;
                                //us_until_next_step = get_step_time();
                                update_shadow_location();
                        }
                        break;
                
                case INPUT_SOFT_DROP:
                        current_piece_location.y += 1;
                        update_score(SOFT_DROP_SCORE);
                        if (collision(current_piece, current_piece_rotation, current_piece_location))
                                current_piece_location.y -= 1;
                        else
                                us_until_next_step = get_step_time();
                        break;
                
                case INPUT_LEFT:
                        current_piece_location.x -= 1;
                        if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                                current_piece_location.x += 1;
                        } else {
                                //us_until_next_step = get_step_time();
                                update_shadow_location();
                        }
                        break;
                
                case INPUT_RIGHT:
                        current_piece_location.x += 1;
                        if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                                current_piece_location.x -= 1;
                        } else {
                                //us_until_next_step = get_step_time();
                                update_shadow_location();
                        }
                        break;
                
                default:
                        return;
                }
        }

        us_until_next_read = INPUT_TIME_US;
}



// Tests

#ifdef DEBUG
static void test_assert_eq(int expected, int got, char *msg) {
        if (expected != got) {
                fprintf(stderr, "assert failed: expected %d but got %d (%s)\n", expected, got, msg);
                exit(EXIT_FAILURE);
        }
}

static void test_assert_diff(int expected, int got, char *msg) {
        if (expected == got) {
                fprintf(stderr, "assert failed: expected anything but %d but we got %d (%s)\n", expected, got, msg);
                exit(EXIT_FAILURE);
        }
}

static void test_rng(void) {
        int tetriminos[8];
        memset(tetriminos, 0, sizeof(tetriminos));

        for (int i=0; i<7000; i++) {
                tetriminos[current_piece]++;
                current_piece = next_random_piece();
        }

        test_assert_eq(0, tetriminos[0], "Test tetrimino");
        test_assert_eq(1000, tetriminos[1], "Tetrimino 1");
        test_assert_eq(1000, tetriminos[2], "Tetrimino 2");
        test_assert_eq(1000, tetriminos[3], "Tetrimino 3");
        test_assert_eq(1000, tetriminos[4], "Tetrimino 4");
        test_assert_eq(1000, tetriminos[5], "Tetrimino 5");
        test_assert_eq(1000, tetriminos[6], "Tetrimino 6");
        test_assert_eq(1000, tetriminos[7], "Tetrimino 7");
        fprintf(stderr, "RNG is correct.\n");
}

static void reset_playfield(void) {
        memset(playfield, 0, TETRIS_PLAYFIELD_Y*TETRIS_PLAYFIELD_X*sizeof(enum tetris_color));
}

static void test_single_row() {
        reset_playfield();
        
        int y = TETRIS_PLAYFIELD_Y-1;
        for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                playfield[y][x] = piece_color(current_piece);
                current_piece = next_random_piece();
        }

        int nlines = clear_full_lines();
        test_assert_eq(1, nlines, "Single row, nlines");

        for (y = 0; y < TETRIS_PLAYFIELD_Y; y++) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Single row, playfield");
                }
        }
}

static void test_double_row() {
        reset_playfield();
        
        for (int y = TETRIS_PLAYFIELD_Y-1; y>=TETRIS_PLAYFIELD_Y-2; y--) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        playfield[y][x] = piece_color(current_piece);
                        current_piece = next_random_piece();
                }
        }

        int nlines = clear_full_lines();
        test_assert_eq(2, nlines, "Double row, nlines");

        for (int y = 0; y < TETRIS_PLAYFIELD_Y; y++) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Double row, playfield");
                }
        }
}

static void test_triple_row() {
        reset_playfield();
        
        for (int y = TETRIS_PLAYFIELD_Y-1; y>=TETRIS_PLAYFIELD_Y-3; y--) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        playfield[y][x] = piece_color(current_piece);
                        current_piece = next_random_piece();
                }
        }

        int nlines = clear_full_lines();
        test_assert_eq(3, nlines, "Triple row, nlines");


        for (int y = 0; y < TETRIS_PLAYFIELD_Y; y++) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Triple row, playfield");
                }
        }
}

static void test_tetris_row() {
        reset_playfield();
        
        for (int y = TETRIS_PLAYFIELD_Y-1; y>=TETRIS_PLAYFIELD_Y-4; y--) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        playfield[y][x] = piece_color(current_piece);
                        current_piece = next_random_piece();
                }
        }

        int nlines = clear_full_lines();
        test_assert_eq(4, nlines, "Tetris row, nlines");


        for (int y = 0; y < TETRIS_PLAYFIELD_Y; y++) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Tetris row, playfield");
                }
        }
}

static void test_irregular_rows(void) {
        reset_playfield();

        int x,y;
        //XXXXXXXXXX
        //XXX_XXXXXX
        //XXX_XXXXX_
        //__________
        //__XXX__X_X
        x = 0;
        y = TETRIS_PLAYFIELD_Y-1;
        
        playfield[y-0][x+0] = piece_color(current_piece);
        playfield[y-0][x+1] = piece_color(current_piece);
        playfield[y-0][x+5] = piece_color(current_piece);
        playfield[y-0][x+6] = piece_color(current_piece);
        playfield[y-0][x+8] = piece_color(current_piece);
        
        playfield[y-1][x+0] = piece_color(current_piece);
        playfield[y-1][x+1] = piece_color(current_piece);
        playfield[y-1][x+2] = piece_color(current_piece);
        playfield[y-1][x+3] = piece_color(current_piece);
        playfield[y-1][x+4] = piece_color(current_piece);
        playfield[y-1][x+5] = piece_color(current_piece);
        playfield[y-1][x+6] = piece_color(current_piece);
        playfield[y-1][x+7] = piece_color(current_piece);
        playfield[y-1][x+8] = piece_color(current_piece);
        playfield[y-1][x+9] = piece_color(current_piece);
        
        playfield[y-2][x+3] = piece_color(current_piece);
        playfield[y-2][x+9] = piece_color(current_piece);
        
        playfield[y-3][x+3] = piece_color(current_piece);
        
        int nlines = clear_full_lines();
        test_assert_eq(1, nlines, "Irregular rows, nlines");

        for (y=0; y<TETRIS_PLAYFIELD_Y-4; y++) {
                for (x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Irreglar rows, empty");
                }
        }
        
        x = 0;
        y = TETRIS_PLAYFIELD_Y-1;

        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+0], "Irregular rows, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+1], "Irregular rows, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+2], "Irregular rows, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+3], "Irregular rows, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+4], "Irregular rows, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+5], "Irregular rows, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+6], "Irregular rows, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+7], "Irregular rows, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+8], "Irregular rows, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+9], "Irregular rows, 1");
        
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+0], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+1], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+2], "Irregular rows, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+3], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+4], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+5], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+6], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+7], "Irregular rows, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+8], "Irregular rows, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+9], "Irregular rows, 2");
        
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+0], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+1], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+2], "Irregular rows, 3");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-2][x+3], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+4], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+5], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+6], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+7], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+8], "Irregular rows, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+9], "Irregular rows, 3");
}

static void test_irregular_rows_2(void) {
        reset_playfield();

        int x,y;
        //XXXXXXXXXX
        //XXXXXX_XXX
        //XXXXXX_XXX
        //X_XX____XX
        //___X______
        //__________
        //__________
        x = 0;
        y = TETRIS_PLAYFIELD_Y-1;

        for (x=0; x<TETRIS_PLAYFIELD_X; x++)
                playfield[y-0][x] = TETRIS_COLOR_RED;
        for (x=0; x<TETRIS_PLAYFIELD_X; x++)
                playfield[y-1][x] = TETRIS_COLOR_RED;
        for (x=0; x<TETRIS_PLAYFIELD_X; x++)
                playfield[y-2][x] = TETRIS_COLOR_RED;
        x=0;
        playfield[y-2][x+3] = TETRIS_COLOR_BLACK;
        playfield[y-3][x+1] = TETRIS_COLOR_RED;
        playfield[y-3][x+4] = TETRIS_COLOR_RED;
        playfield[y-3][x+5] = TETRIS_COLOR_RED;
        playfield[y-3][x+6] = TETRIS_COLOR_RED;
        playfield[y-3][x+7] = TETRIS_COLOR_RED;
        playfield[y-4][x+6] = TETRIS_COLOR_RED;
        playfield[y-5][x+6] = TETRIS_COLOR_RED;
        
        int nlines = clear_full_lines();
        test_assert_eq(2, nlines, "Irregular rows, nlines");

        for (y=0; y<TETRIS_PLAYFIELD_Y-4; y++) {
                for (x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y][x], "Irreglar rows, empty");
                }
        }
        
        x = 0;
        y = TETRIS_PLAYFIELD_Y-1;

        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+0], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+1], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+2], "Irregular rows 2, 1");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-0][x+3], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+4], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+5], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+6], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+7], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+8], "Irregular rows 2, 1");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-0][x+9], "Irregular rows 2, 1");
        
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+0], "Irregular rows 2, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+1], "Irregular rows 2, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+2], "Irregular rows 2, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+3], "Irregular rows 2, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+4], "Irregular rows 2, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+5], "Irregular rows 2, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+6], "Irregular rows 2, 2");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-1][x+7], "Irregular rows 2, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+8], "Irregular rows 2, 2");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-1][x+9], "Irregular rows 2, 2");
        
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+0], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+1], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+2], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+3], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+4], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+5], "Irregular rows 2, 3");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-2][x+6], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+7], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+8], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-2][x+9], "Irregular rows 2, 3");
        
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+0], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+1], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+2], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+3], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+4], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+5], "Irregular rows 2, 3");
        test_assert_diff(TETRIS_COLOR_BLACK, playfield[y-3][x+6], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+7], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+8], "Irregular rows 2, 3");
        test_assert_eq(TETRIS_COLOR_BLACK, playfield[y-3][x+9], "Irregular rows 2, 3");
}

static void test_row_clear(void) {
        test_single_row();
        test_double_row();
        test_triple_row();
        test_tetris_row();
        test_irregular_rows();
        test_irregular_rows_2();
        
        fprintf(stderr, "Row clearing is correct\n");
}
#endif /* DEBUG */



// Main

int main(void) {
        init_hiscore();
        atexit(save_hiscore);
        
        srandom(time(NULL));
        init_shuffle_pieces();
        current_piece = next_random_piece();
        update_shadow_location();

#ifdef DEBUG
        test_rng();
        test_row_clear();
        return EXIT_SUCCESS;
#endif
        
        initscr();
        atexit(endwin_wrapper);
        
        setup_colors();
        
        noecho();
        raw();
        nodelay(stdscr, true);
        keypad(stdscr, true);
        curs_set(0);
        
        for (;;) {
                draw_screen();
                process_input();
                step();

                usleep(DELAY_US);
                refresh();
        }

        return EXIT_SUCCESS;
}
