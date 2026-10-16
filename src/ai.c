#include "ai.h"

/* Piece-Square Tables (tables de valeurs positionnelles par case) */
static const int pawn_table[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0 },
    {  5, 10, 10,-20,-20, 10, 10,  5 },
    {  5, -5,-10,  0,  0,-10, -5,  5 },
    {  0,  0,  0, 20, 20,  0,  0,  0 },
    {  5,  5, 10, 25, 25, 10,  5,  5 },
    { 10, 10, 20, 30, 30, 20, 10, 10 },
    { 50, 50, 50, 50, 50, 50, 50, 50 },
    {  0,  0,  0,  0,  0,  0,  0,  0 }
};

static const int knight_table[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50 },
    {-40,-20,  0,  5,  5,  0,-20,-40 },
    {-30,  5, 10, 15, 15, 10,  5,-30 },
    {-30,  0, 15, 20, 20, 15,  0,-30 },
    {-30,  5, 15, 20, 20, 15,  5,-30 },
    {-30,  0, 10, 15, 15, 10,  0,-30 },
    {-40,-20,  0,  0,  0,  0,-20,-40 },
    {-50,-40,-30,-30,-30,-30,-40,-50 }
};

static const int bishop_table[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20 },
    {-10,  5,  0,  0,  0,  0,  5,-10 },
    {-10, 10, 10, 10, 10, 10, 10,-10 },
    {-10,  0, 10, 10, 10, 10,  0,-10 },
    {-10,  5,  5, 10, 10,  5,  5,-10 },
    {-10, 10,  5, 10, 10,  5, 10,-10 },
    {-10,  5,  0,  0,  0,  0,  5,-10 },
    {-20,-10,-10,-10,-10,-10,-10,-20 }
};

static const int rook_table[8][8] = {
    {  0,  0,  0,  5,  5,  0,  0,  0 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    {  5, 10, 10, 10, 10, 10, 10,  5 },
    {  0,  0,  0,  0,  0,  0,  0,  0 }
};

static const int queen_table[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20 },
    {-10,  0,  5,  0,  0,  0,  0,-10 },
    {-10,  5,  5,  5,  5,  5,  0,-10 },
    {  0,  0,  5,  5,  5,  5,  0, -5 },
    { -5,  0,  5,  5,  5,  5,  0, -5 },
    {-10,  0,  5,  5,  5,  5,  0,-10 },
    {-10,  0,  0,  0,  0,  0,  0,-10 },
    {-20,-10,-10, -5, -5,-10,-10,-20 }
};

static const int king_table[8][8] = {
    { 20, 30, 10,  0,  0, 10, 30, 20 },
    { 20, 20,  0,  0,  0,  0, 20, 20 },
    {-10,-20,-20,-20,-20,-20,-20,-10 },
    {-20,-30,-30,-40,-40,-30,-30,-20 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 }
};

int valeur_board(const Board board)
{
    int val = 0;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int p = board[x][y];
            if (p == EMPTY)
                continue;

            if (p > 0)
            {
                switch (p)
                {
                    case W_PAWN:   val += 100   + pawn_table[y][x]; break;
                    case W_KNIGHT: val += 320   + knight_table[y][x]; break;
                    case W_BISHOP: val += 330   + bishop_table[y][x]; break;
                    case W_ROOK:   val += 500   + rook_table[y][x]; break;
                    case W_QUEEN:  val += 900   + queen_table[y][x]; break;
                    case W_KING:
                        val += 20000 + king_table[y][x];
                        if ((x == 6 || x == 2) && y == 0) val += 60; /* Bonus roi roque */
                        break;
                }
            }
            else
            {
                int rank = 7 - y;
                switch (p)
                {
                    case B_PAWN:   val -= (100   + pawn_table[rank][x]); break;
                    case B_KNIGHT: val -= (320   + knight_table[rank][x]); break;
                    case B_BISHOP: val -= (330   + bishop_table[rank][x]); break;
                    case B_ROOK:   val -= (500   + rook_table[rank][x]); break;
                    case B_QUEEN:  val -= (900   + queen_table[rank][x]); break;
                    case B_KING:
                        val -= (20000 + king_table[rank][x]);
                        if ((x == 6 || x == 2) && y == 7) val -= 60; /* Bonus roi roque */
                        break;
                }
            }
        }
    }
    return val;
}

/*
 * Ordonnancement des coups : evalue les captures en premier.
 * Cela permet a l'elagage Alpha-Beta de trouver des coupures beaucoup plus tot
 * et réduit le temps de calcul sur des positions complexes.
 */
static void order_captures(moves_struct *moves, const Board board)
{
    int write_idx = 0;
    for (int i = 0; i < moves->len; i++)
    {
        if (board[moves->Moves[i].x][moves->Moves[i].y] != EMPTY)
        {
            square temp = moves->Moves[write_idx];
            moves->Moves[write_idx] = moves->Moves[i];
            moves->Moves[i] = temp;
            write_idx++;
        }
    }
}

/*
 * Recherche de tranquillite (Quiescence Search).
 * Continue l'exploration uniquement sur les captures lorsque depth == 0
 * pour resoudre les echanges de pieces en cours
 */
int quiescence(const Board board, int alpha, int beta, int color, int qdepth)
{
    bool in_check = est_echec(board, color);

    if (qdepth <= 0)
    {
        if (in_check)
        {
            if (!has_legal_moves(board, color))
                return (color == 1) ? (-90000 - qdepth) : (90000 + qdepth);
        }
        return valeur_board(board);
    }

    if (in_check)
    {
        int legal_moves_count = 0;

        if (color == 1) /* Blancs en echec (maximise) */
        {
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    if (board[x][y] <= 0)
                        continue;

                    moves_struct moves = get_moves(board, x, y, false);
                    order_captures(&moves, board);

                    for (int k = 0; k < moves.len; k++)
                    {
                        int tx = moves.Moves[k].x;
                        int ty = moves.Moves[k].y;
                        if (board[tx][ty] == B_KING)
                            continue;

                        Board next;
                        fait_coup(board, next, x, y, tx, ty);
                        if (est_echec(next, 1))
                            continue;

                        legal_moves_count++;
                        int score = quiescence(next, alpha, beta, -1, qdepth - 1);
                        if (score > alpha)
                            alpha = score;
                        if (beta <= alpha)
                            return beta; /* Coupure Beta */
                    }
                }
            }

            if (legal_moves_count == 0)
                return -90000 - qdepth; /* Echec et mat des Blancs */

            return alpha;
        }
        else /* Noirs en echec (minimise) */
        {
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    if (board[x][y] >= 0)
                        continue;

                    moves_struct moves = get_moves(board, x, y, false);
                    order_captures(&moves, board);

                    for (int k = 0; k < moves.len; k++)
                    {
                        int tx = moves.Moves[k].x;
                        int ty = moves.Moves[k].y;
                        if (board[tx][ty] == W_KING)
                            continue;

                        Board next;
                        fait_coup(board, next, x, y, tx, ty);
                        if (est_echec(next, -1))
                            continue;

                        legal_moves_count++;
                        int score = quiescence(next, alpha, beta, 1, qdepth - 1);
                        if (score < beta)
                            beta = score;
                        if (beta <= alpha)
                            return alpha; /* Coupure Alpha */
                    }
                }
            }

            if (legal_moves_count == 0)
                return 90000 + qdepth; /* Echec et mat des Noirs */

            return beta;
        }
    }

    /* Hors echec : recherche de tranquillite standard sur les captures */
    int stand_pat = valeur_board(board);

    if (color == 1) /* Blancs (maximise) */
    {
        if (stand_pat >= beta)
            return beta;
        if (stand_pat > alpha)
            alpha = stand_pat;

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (board[x][y] <= 0)
                    continue;

                moves_struct moves = get_moves(board, x, y, false);
                for (int k = 0; k < moves.len; k++)
                {
                    int tx = moves.Moves[k].x;
                    int ty = moves.Moves[k].y;

                    /* On n'evalue QUE les captures et on ne capture jamais de Roi */
                    if (board[tx][ty] == EMPTY || board[tx][ty] == B_KING)
                        continue;

                    Board next;
                    fait_coup(board, next, x, y, tx, ty);
                    if (est_echec(next, 1))
                        continue; /* Coup illegal : roi blanc en echec */

                    int score = quiescence(next, alpha, beta, -1, qdepth - 1);
                    if (score > alpha)
                        alpha = score;
                    if (beta <= alpha)
                        return beta; /* Coupure Beta */
                }
            }
        }
        return alpha;
    }
    else /* Noirs (minimise) */
    {
        if (stand_pat <= alpha)
            return alpha;
        if (stand_pat < beta)
            beta = stand_pat;

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (board[x][y] >= 0)
                    continue;

                moves_struct moves = get_moves(board, x, y, false);
                for (int k = 0; k < moves.len; k++)
                {
                    int tx = moves.Moves[k].x;
                    int ty = moves.Moves[k].y;

                    /* On n'evalue QUE les captures et on ne capture jamais de Roi */
                    if (board[tx][ty] == EMPTY || board[tx][ty] == W_KING)
                        continue;

                    Board next;
                    fait_coup(board, next, x, y, tx, ty);
                    if (est_echec(next, -1))
                        continue; /* Coup illegal : roi noir en echec */

                    int score = quiescence(next, alpha, beta, 1, qdepth - 1);
                    if (score < beta)
                        beta = score;
                    if (beta <= alpha)
                        return alpha; /* Coupure Alpha */
                }
            }
        }
        return beta;
    }
}

/*
 * Minimax avec elagage Alpha-Beta, recherche de tranquillite et detection du mat/pat.
 * color =  1 : Blancs (maximise)
 * color = -1 : Noirs  (minimise)
 * alpha : meilleur score garanti pour le joueur maximisant
 * beta  : meilleur score garanti pour le joueur minimisant
 */
int minimax(const Board board, int depth, int alpha, int beta, int color)
{
    if (depth == 0)
        return quiescence(board, alpha, beta, color, 4);

    int legal_moves_count = 0;

    if (color == 1)
    {
        int max_eval = -100000;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (board[x][y] <= 0)
                    continue;

                moves_struct moves = get_moves(board, x, y, false);
                order_captures(&moves, board);

                for (int k = 0; k < moves.len; k++)
                {
                    int tx = moves.Moves[k].x;
                    int ty = moves.Moves[k].y;
                    if (board[tx][ty] == B_KING)
                        continue;

                    Board next;
                    fait_coup(board, next, x, y, tx, ty);
                    if (est_echec(next, 1))
                        continue; /* Coup illegal : roi blanc en echec */

                    legal_moves_count++;
                    int eval = minimax(next, depth - 1, alpha, beta, -1);
                    if (eval > max_eval)
                        max_eval = eval;
                    if (eval > alpha)
                        alpha = eval;
                    if (beta <= alpha)
                        return max_eval; /* Coupure Beta */
                }
            }
        }

        /* Si aucun coup legal n'a pu etre joue */
        if (legal_moves_count == 0)
        {
            if (est_echec(board, 1))
                return -90000 - depth; /* Blancs en echec et mat */
            return 0; /* Pat */
        }

        return max_eval;
    }
    else
    {
        int min_eval = 100000;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (board[x][y] >= 0)
                    continue;

                moves_struct moves = get_moves(board, x, y, false);
                order_captures(&moves, board);

                for (int k = 0; k < moves.len; k++)
                {
                    int tx = moves.Moves[k].x;
                    int ty = moves.Moves[k].y;
                    if (board[tx][ty] == W_KING)
                        continue;

                    Board next;
                    fait_coup(board, next, x, y, tx, ty);
                    if (est_echec(next, -1))
                        continue; /* Coup illegal : roi noir en echec */

                    legal_moves_count++;
                    int eval = minimax(next, depth - 1, alpha, beta, 1);
                    if (eval < min_eval)
                        min_eval = eval;
                    if (eval < beta)
                        beta = eval;
                    if (beta <= alpha)
                        return min_eval; /* Coupure Alpha */
                }
            }
        }

        /* Si aucun coup legal n'a pu etre joue */
        if (legal_moves_count == 0)
        {
            if (est_echec(board, -1))
                return 90000 + depth; /* Noirs en echec et mat */
            return 0; /* Pat */
        }

        return min_eval;
    }
}

struct infos_piece pos_best_coup(const Board board, int depth, int color)
{
    struct infos_piece best;
    best.dep.x = -1;
    best.dep.y = -1;
    best.fin.x = -1;
    best.fin.y = -1;
    best.valb  = (color == 1) ? -100000 : 100000;

    int alpha = -100000;
    int beta  =  100000;

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            if (board[x][y] * color <= 0)
                continue;

            moves_struct moves = get_moves(board, x, y, true);
            order_captures(&moves, board);

            for (int k = 0; k < moves.len; k++)
            {
                int tx = moves.Moves[k].x;
                int ty = moves.Moves[k].y;
                if (board[tx][ty] == (color == 1 ? B_KING : W_KING))
                    continue;

                Board next;
                fait_coup(board, next, x, y, tx, ty);
                int result = minimax(next, depth - 1, alpha, beta, -color);

                if (color == 1)
                {
                    if (result > best.valb)
                    {
                        best.dep.x = x;
                        best.dep.y = y;
                        best.fin.x = tx;
                        best.fin.y = ty;
                        best.valb  = result;
                    }
                    if (result > alpha)
                        alpha = result;
                }
                else
                {
                    if (result < best.valb)
                    {
                        best.dep.x = x;
                        best.dep.y = y;
                        best.fin.x = tx;
                        best.fin.y = ty;
                        best.valb  = result;
                    }
                    if (result < beta)
                        beta = result;
                }
            }
        }
    }

    if (best.fin.x != -1)
    {
        if (board[best.dep.x][best.dep.y] == B_KING) { Roque.BlR = false; Roque.BrR = false; }
        if (board[best.dep.x][best.dep.y] == W_KING) { Roque.WlR = false; Roque.WrR = false; }
    }

    return best;
}
