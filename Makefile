tetrominoes: tetrominoes.c
	gcc $< -o $@ -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -pedantic -std=c99 -O3 -lncurses -lm
tetrominoes_dbg: tetrominoes.c
	gcc $< -o $@ -DDEBUG -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -pedantic -std=c99 -Werror -g -Og -lncurses -lm
