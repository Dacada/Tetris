#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DELAY_US 30000L
#define STEP_TIME_US 500000L
#define INPUT_TIME_US 100000L

#define TETRIS_PLAYFIELD_X 10
#define TETRIS_PLAYFIELD_Y 40 // this will be drawn as half this value but internally as this value

#define NEXT_RECTANGLE_DRAW_X 12
#define NEXT_RECTANGLE_DRAW_Y 16

#define SCORE_RECTANGLE_DRAW_X 12
#define SCORE_RECTANGLE_DRAW_Y 2

#define HOLD_RECTANGLE_DRAW_X 12
#define HOLD_RECTANGLE_DRAW_Y 6

#define DRAWING_CHAR '#'

#define SINGLE_SCORE 100;
#define DOUBLE_SCORE 300;
#define T_SPIN_SCORE 400;
#define TRIPLE_SCORE 500;
#define TETRIS_SCORE 800;

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

struct coord {
        int x;
        int y;
};

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

enum tetrimino spawn_order[] = {
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

int spawn_next_i = 0;

static void shuffle(enum tetrimino *arr, int size) {
        for (int i=0; i<size; i++) {
                int j = random() % size;
                enum tetrimino tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
        }
}

static void init_shuffle_pieces(void) {
        shuffle(spawn_order, 7);
}

/*
static enum tetrimino char_to_piece(char c) {
        switch (c) {
        case 'I':
                return TETRIMINO_I;
        case 'O':
                return TETRIMINO_O;
        case 'T':
                return TETRIMINO_T;
        case 'S':
                return TETRIMINO_S;
        case 'Z':
                return TETRIMINO_Z;
        case 'J':
                return TETRIMINO_J;
        case 'L':
                return TETRIMINO_L;
        default:
                return TETRIMINO_TEST;
        }
}
*/

static enum tetrimino current_held_piece = TETRIMINO_TEST;

static enum tetrimino next_random_piece(void) {
        if (spawn_next_i == 7) {
                spawn_next_i = 0;
                memcpy(spawn_order, spawn_order+7, 7*sizeof(enum tetrimino));
        }
                
        if (spawn_next_i == 0)
                shuffle(spawn_order+7, 7);
                
        return spawn_order[spawn_next_i++];
}

static void draw(int stx, int edx, int sty, int edy, enum tetris_color c) {
        enable_color(c, false);
        for (int x=stx; x<edx; x++) {
                for (int y=sty; y<edy; y++) {
                        mvaddch(y, x, DRAWING_CHAR);
                }
        }
        disable_color(c, false);
}

static void draw_background(int max_x, int max_y) {
        draw(0, max_x, 0, max_y, TETRIS_COLOR_WHITE);
}

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

static void decide_rotation_offset_draw_tetrimino(enum tetrimino t,
                                                  enum tetrimino_rotation *r, int *ox, int *oy) {
        switch(t) {
        case TETRIMINO_TEST:
                *r = SPAWN_ROTATED;
                *ox = 0;
                *oy = 0;
                break;
        case TETRIMINO_I:
                *r = SPAWN_ROTATED;
                *ox = -2;
                *oy = 1;
                break;
        case TETRIMINO_O:
                *r = SPAWN_ROTATED;
                *ox = -2;
                *oy = 1;
                break;
        case TETRIMINO_T:
                *r = SPAWN_ROTATED;
                *ox = -1;
                *oy = 1;
                break;
        case TETRIMINO_S:
                *r = SPAWN_ROTATED;
                *ox = -1;
                *oy = 1;
                break;
        case TETRIMINO_Z:
                *r = SPAWN_ROTATED;
                *ox = -1;
                *oy = 1;
                break;
        case TETRIMINO_J:
                *r = SPAWN_ROTATED;
                *ox = -1;
                *oy = 1;
                break;
        case TETRIMINO_L:
                *r = SPAWN_ROTATED;
                *ox = -1;
                *oy = 1;
                break;
        }
}

static void draw_tetrimino(enum tetrimino t,int x, int y) {
        enable_color(piece_color(t), false);
        
        enum tetrimino_rotation r = SPAWN_ROTATED;
        int ox = 0;
        int oy = 0;
        decide_rotation_offset_draw_tetrimino(t, &r, &ox, &oy);
        
        for (int j=0; j<4; j++) {
                for (int i=0; i<4; i++) {
                        if (piece_shapes[t][r][j][i]) {
                                mvaddch(y+j+oy, x+i*2+ox, DRAWING_CHAR);
                                mvaddch(y+j+oy, x+i*2+1+ox, DRAWING_CHAR);
                        }
                        
                }
        }

        disable_color(piece_color(t), false);
}

static long score;

static enum tetris_color playfield[TETRIS_PLAYFIELD_Y][TETRIS_PLAYFIELD_X];

static enum tetrimino current_piece = TETRIMINO_TEST;
static enum tetrimino_rotation current_piece_rotation = SPAWN_ROTATED;
static struct coord current_piece_location = { 5, 20 };

static struct coord current_shadow_location;

static bool can_hold = true;

static bool collision(enum tetrimino piece, enum tetrimino_rotation rotation, struct coord location) {
        for (int j=0; j<4; j++) {
                for (int i=0; i<4; i++) {
                        int x = location.x + i;
                        int y = location.y + j;
                        
                        if (piece_shapes[piece][rotation][j][i]) {
                                if (x < 0 || x >= TETRIS_PLAYFIELD_X)
                                        return true;
                                if (y < 0 || y >= TETRIS_PLAYFIELD_Y)
                                        return true;
                                
                                enum tetris_color c = playfield[y][x];
                        
                                if (c != TETRIS_COLOR_BLACK)
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

static bool last_movement_was_spin = false;
static bool paused = false;

static long us_until_next_step = STEP_TIME_US;
static void step(void) {
        if (paused)
                return;
        
        if (us_until_next_step <= 0) {
                us_until_next_step = STEP_TIME_US;
                current_piece_location.y += 1;
                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
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
                                        score += T_SPIN_SCORE;
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

                        while (full_lines_count > 0) {
                                if (full_lines_count == 1) {
                                        score += SINGLE_SCORE;
                                        break;
                                } else if (full_lines_count == 2) {
                                        score += DOUBLE_SCORE;
                                        break;
                                } else if (full_lines_count == 3) {
                                        score += TRIPLE_SCORE;
                                        break;
                                } else if (full_lines_count >= 4) {
                                        score += TETRIS_SCORE;
                                        full_lines_count -= 4;
                                }
                        }
                        
                        current_piece = next_random_piece();
                        current_piece_rotation = SPAWN_ROTATED;
                        current_piece_location.x = 5;
                        current_piece_location.y = 20;
                        can_hold = true;
                        update_shadow_location();

                        if (collision(current_piece, current_piece_rotation, current_piece_location))
                                exit(0); // Perfect Game Over screen right here
                }
        } else {
                us_until_next_step -= DELAY_US;
        }
}

static struct coord wall_kicks[2][8][5] = {
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

        struct coord curr_coords = current_piece_location;
        for (int k=0; k<5; k++) {
                struct coord c = wall_kicks[i][j][k];

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
        
        switch(t) {
        case INPUT_CLOCKWISE_ROTATION:
                if (rotate((current_piece_rotation + 1) % 4)) {
                        last_movement_was_spin = true;
                        us_until_next_step = STEP_TIME_US;
                        update_shadow_location();
                }
                break;
                
        case INPUT_HARD_DROP:
                while (!collision(current_piece, current_piece_rotation, current_piece_location)) {
                        current_piece_location.y++;
                }
                current_piece_location.y--;
                us_until_next_step = STEP_TIME_US;
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
                        us_until_next_step = STEP_TIME_US;
                        can_hold = false;
                        last_movement_was_spin = false;
                        update_shadow_location();
                }
                break;
                
        case INPUT_COUNTERCLOCKWISE_ROTATION:
                if (rotate((current_piece_rotation - 1) % 4)) {
                        last_movement_was_spin = true;
                        us_until_next_step = STEP_TIME_US;
                        update_shadow_location();
                }
                break;
                
        case INPUT_SOFT_DROP:
                current_piece_location.y += 1;
                if (collision(current_piece, current_piece_rotation, current_piece_location))
                        current_piece_location.y -= 1;
                else
                        us_until_next_step = STEP_TIME_US;
                break;
                
        case INPUT_LEFT:
                current_piece_location.x -= 1;
                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                        current_piece_location.x += 1;
                } else {
                        us_until_next_step = STEP_TIME_US;
                        update_shadow_location();
                }
                break;
                
        case INPUT_RIGHT:
                current_piece_location.x += 1;
                if (collision(current_piece, current_piece_rotation, current_piece_location)) {
                        current_piece_location.x -= 1;
                } else {
                        us_until_next_step = STEP_TIME_US;
                        update_shadow_location();
                }
                break;
                
        default:
                return;
        }

        us_until_next_read = INPUT_TIME_US;
}

static void draw_playfield(int stx, int edx, int sty, int edy) {
        (void)edx;
        (void)edy;
        
        for (int j=TETRIS_PLAYFIELD_Y/2; j<TETRIS_PLAYFIELD_Y; j++) {
                for (int i=0; i<TETRIS_PLAYFIELD_X; i++) {
                        int x = i - current_piece_location.x;
                        int y = j - current_piece_location.y;

                        int xs = i - current_shadow_location.x;
                        int ys = j - current_shadow_location.y;

                        bool piece_in_range = (x >= 0 && x < 4 && y >= 0 && y < 4);
                        bool shadow_in_range = (xs >= 0 && xs < 4 && ys >= 0 && ys < 4);
                        
                        enum tetris_color color;
                        bool shadow;
                        if (piece_in_range && piece_shapes[current_piece][current_piece_rotation][y][x]) {
                                color = piece_color(current_piece);
                                shadow = false;
                        } else if (shadow_in_range && piece_shapes[current_piece][current_piece_rotation][ys][xs]) {
                                color = piece_color(current_piece);
                                shadow = true;
                        } else {
                                color = playfield[j][i];
                                shadow = false;
                        }
                        
                        enable_color(color, shadow);
                        mvaddch(sty+j, stx+i*2, DRAWING_CHAR);
                        mvaddch(sty+j, stx+i*2+1, DRAWING_CHAR);
                        disable_color(color, shadow);
                }
        }
}

static void draw_nextarea(int stx, int edx, int sty, int edy) {
        draw(stx, edx, sty, edy, TETRIS_COLOR_BLACK);
        
        mvprintw(sty+0, stx+1, "Next");

        int midx = stx + (edx - stx)/2;
        int midy = sty + (edy - sty)/2;
        int midy1 = sty + (midy - sty)/2;
        int midy2 = midy;
        int midy3 = midy + (edy - midy)/2;

        int tstx = midx - 2;
        int tsty1 = midy1 - 2;
        int tsty2 = midy2 - 2;
        int tsty3 = midy3 - 2;

        draw_tetrimino(spawn_order[spawn_next_i], tstx, tsty1);
        draw_tetrimino(spawn_order[spawn_next_i+1], tstx, tsty2);
        draw_tetrimino(spawn_order[spawn_next_i+2], tstx, tsty3);
}

static void draw_scorearea(int stx, int edx, int sty, int edy) {
        draw(stx, edx, sty, edy, TETRIS_COLOR_BLACK);

        mvprintw(sty+0, stx+1, "Score");
        mvprintw(sty+1, stx+1, "%08ld", score);
}

static void draw_holdarea(int stx, int edx, int sty, int edy) {
        draw(stx, edx, sty, edy, TETRIS_COLOR_BLACK);

        mvprintw(sty+0, stx+1, "Hold");
        if (current_held_piece != TETRIMINO_TEST) {
                int midx = stx + (edx - stx)/2;
                int midy = sty + (edy - sty)/2;
                int tstx = midx - 2;
                int tsty = midy - 2;
                draw_tetrimino(current_held_piece, tstx, tsty);
        }
}

static void draw_controlsarea(int stx, int edx, int sty, int edy) {
        int msglen = 32; // x/z:rotate c:hold p:pause q:quit
        int total_space = edx-stx;
        int spare_space = total_space - msglen;
        int margin = spare_space / 2;
                
        draw(stx, edx, sty, edy, TETRIS_COLOR_BLACK);

        int i=0;

        // TODO: DRY this thing
        
        attron(A_BOLD);
        mvaddch(sty, stx + margin + i, 'x'); i++;
        attroff(A_BOLD);
        
        mvaddch(sty, stx + margin + i, '/'); i++;
        
        attron(A_BOLD);
        mvaddch(sty, stx + margin + i, 'z'); i++;
        attroff(A_BOLD);
        
        mvprintw(sty, stx + margin + i, ":rotate "); i+=strlen(":rotate ");
        
        attron(A_BOLD);
        mvaddch(sty, stx + margin + i, 'c'); i++;
        attroff(A_BOLD);
        
        mvprintw(sty, stx + margin + i, ":hold "); i+=strlen(":hold ");
        
        attron(A_BOLD);
        mvaddch(sty, stx + margin + i, 'p'); i++;
        attroff(A_BOLD);
        
        mvprintw(sty, stx + margin + i, ":pause "); i+=strlen(":pause ");
        
        attron(A_BOLD);
        mvaddch(sty, stx + margin + i, 'q'); i++;
        attroff(A_BOLD);
        
        mvprintw(sty, stx + margin + i, ":quit "); i+=strlen(":quit");
}

static void endwin_wrapper(void) {
        endwin();
}

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

/*
static void print_playfield(void) {
        for (int y=TETRIS_PLAYFIELD_Y-7; y<TETRIS_PLAYFIELD_Y; y++) {
                for (int x=0; x<TETRIS_PLAYFIELD_X; x++) {
                        if (playfield[y][x] == TETRIS_COLOR_BLACK)
                                fprintf(stderr, "X");
                        else
                                fprintf(stderr, "_");
                }
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
}
*/

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

int main(void) {
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
        
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_CYAN);
        init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(4, COLOR_GREEN, COLOR_GREEN);
        init_pair(5, COLOR_RED, COLOR_RED);
        init_pair(6, COLOR_BLUE, COLOR_BLUE);
        init_pair(7, COLOR_RED, COLOR_YELLOW);
        
        init_pair(8, COLOR_BLACK, COLOR_BLACK);
        init_pair(9, COLOR_WHITE, COLOR_WHITE);
        
        init_pair(10, COLOR_CYAN, COLOR_BLACK);
        init_pair(11, COLOR_YELLOW, COLOR_BLACK);
        init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(13, COLOR_GREEN, COLOR_BLACK);
        init_pair(14, COLOR_RED, COLOR_BLACK);
        init_pair(15, COLOR_BLUE, COLOR_BLACK);
        init_pair(16, COLOR_RED, COLOR_BLACK);
        
        noecho();
        raw();
        nodelay(stdscr, true);
        keypad(stdscr, true);
        curs_set(0);
        
        for (;;) {
                int max_x, max_y;
                getmaxyx(stdscr, max_y, max_x); // it's a macro

                // center horizontally as if it was double its actual width
                int pf_stx = max_x/2 - TETRIS_PLAYFIELD_X;
                int pf_edx = max_x/2 + TETRIS_PLAYFIELD_X;
                
                // center vertically as if it was half its actual length
                int pf_sty = max_y/2 - TETRIS_PLAYFIELD_Y/2 - TETRIS_PLAYFIELD_Y/4;
                int pf_edy = max_y/2 + TETRIS_PLAYFIELD_Y/2 - TETRIS_PLAYFIELD_Y/4;

                int pf_sty_visible = pf_sty + TETRIS_PLAYFIELD_Y/2;

                // Take into account that playfield is drawn as twice its actual size
                int sc_stx = pf_edx + 3;
                int sc_edx = sc_stx + SCORE_RECTANGLE_DRAW_X;
                int sc_sty = pf_sty_visible;
                int sc_edy = sc_sty + SCORE_RECTANGLE_DRAW_Y;

                int nx_stx = sc_stx;
                int nx_edx = nx_stx + NEXT_RECTANGLE_DRAW_X;
                int nx_sty = sc_edy + 2;
                int nx_edy = nx_sty + NEXT_RECTANGLE_DRAW_Y;

                int hd_edx = pf_stx - 3;
                int hd_stx = hd_edx - HOLD_RECTANGLE_DRAW_X;
                int hd_sty = pf_sty_visible;
                int hd_edy = hd_sty + HOLD_RECTANGLE_DRAW_Y;

                int cs_stx = hd_stx;
                int cs_edx = nx_edx;
                int cs_sty = pf_edy + 1;
                int cs_edy = cs_sty + 1;

                draw_background(max_x, max_y);
                draw_playfield(pf_stx, pf_edx, pf_sty, pf_edy);
                draw_nextarea(nx_stx, nx_edx, nx_sty, nx_edy);
                draw_scorearea(sc_stx, sc_edx, sc_sty, sc_edy);
                draw_holdarea(hd_stx, hd_edx, hd_sty, hd_edy);
                draw_controlsarea(cs_stx, cs_edx, cs_sty, cs_edy);

                process_input();
                step();

                usleep(DELAY_US);
                refresh();
        }

        return EXIT_SUCCESS;
}

// TODO: Game over screen
// TODO: Instructions when calling with -h or help or --help
// TODO: Version information and other memes (-v --version)
// TODO: Add paused screen (should hide game state)
// TODO: Better colors
// TODO: High scores system
// TODO: A hard drop should not let you keep moving the piece
// TODO: Keep falling despite moving laterally? (check standard)
