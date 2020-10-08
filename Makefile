tetris: tetris.c
	gcc tetris.c -o tetris -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -pedantic -std=c99 -Werror -O3 -lncurses -lm
tetris_dbg: tetris.c
	gcc tetris.c -o tetris_dbg -DDEBUG -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -pedantic -std=c99 -Werror -g -Og -lncurses -lm
