#ifndef BOARD_H
#define BOARD_H

#include "settings.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NB_MOVES_MAX 32

typedef int Board[8][8];

typedef struct {
    int x;
    int y;
} square;

typedef struct {
    square Moves[NB_MOVES_MAX];
    int len;
} moves_struct;

struct infos_piece {
    square dep;
    int    valb;
    square fin;
};

typedef struct roque {
    bool WlR;
    bool WrR;
    bool BlR;
    bool BrR;
} roque;

void    display_board(const Board board);
void    setup_board(Board board);
void    setup_board_test_mat(Board board);
void    setup_board_test_roi(Board board);
void    copy_board(const Board src, Board dst);
void    fait_coup(const Board board, Board dst, int x, int y, int xf, int yf);
square  find_piece(const Board board, int piece);

#endif
