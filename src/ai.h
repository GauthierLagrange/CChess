#ifndef AI_H
#define AI_H

#include "rules.h"

int                valeur_board(const Board board);
int                quiescence(const Board board, int alpha, int beta, int color, int qdepth);
int                minimax(const Board board, int depth, int alpha, int beta, int color);
struct infos_piece pos_best_coup(const Board board, int depth, int color);

#endif
