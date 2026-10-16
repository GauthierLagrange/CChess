#ifndef RENDERER_H
#define RENDERER_H

#include <raylib.h>
#include "rules.h"

#define BOARD_SIZE 1200
#define CELL_SIZE 150
#define SIDEBAR_WIDTH 510
#define WINDOW_WIDTH (BOARD_SIZE + SIDEBAR_WIDTH)
#define WINDOW_HEIGHT BOARD_SIZE

void   init(void);
void   cleanup(void);
void   display_board_CIG(const Board board, int xd, int yd, int xf, int yf, bool first_coup, bool flip_board);
void   display_coup_possible(moves_struct moves_piece_selec, bool flip_board);
void   display_last_coup(int xd, int yd, int xf, int yf, bool flip_board);
square pos_vers_coord(Vector2 pos, bool flip_board);

#endif
