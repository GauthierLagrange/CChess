#ifndef RULES_H
#define RULES_H

#include "board.h"

extern roque Roque;

moves_struct get_moves(const Board board, int x, int y, bool bool_check);
moves_struct add_move(int len, int xd, int yd,
                      int x, int y, int color, const Board board,
                      bool bool_check, moves_struct output);
bool est_echec(const Board board, int color);
bool has_legal_moves(const Board board, int color);
void deroque(const Board board, int color, int x, int y);
void deroque_dest(int x, int y);
bool can_roque(const Board board, int x, int y, int color, char side);

#endif
