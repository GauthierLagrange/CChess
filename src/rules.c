#include "rules.h"

static const square Knight_moves[8] = {
    {-1, -2}, { 1, -2}, {-2, -1}, { 2, -1},
    {-2,  1}, { 2,  1}, {-1,  2}, { 1,  2}
};

roque Roque = {true, true, true, true};

static bool in_check_detection = false;

moves_struct get_moves(const Board board, int x, int y, bool bool_check)
{
    moves_struct output;
    output.len = 0;
    int piece = board[x][y];

    if (piece == EMPTY)
        return output;

    int color = (piece > 0) ? 1 : -1;

    if (piece == W_PAWN || piece == B_PAWN)
    {
        int y_forward = y + color;
        if (y_forward >= 0 && y_forward < 8 && board[x][y_forward] == EMPTY)
        {
            output = add_move(output.len, x, y, x, y_forward, color, board, bool_check, output);
            if (y == ((color == 1) ? 1 : 6) && board[x][y_forward + color] == EMPTY)
                output = add_move(output.len, x, y, x, y_forward + color, color, board, bool_check, output);
        }
        if (x > 0 && board[x-1][y_forward] * color < 0)
            output = add_move(output.len, x, y, x-1, y_forward, color, board, bool_check, output);
        if (x < 7 && board[x+1][y_forward] * color < 0)
            output = add_move(output.len, x, y, x+1, y_forward, color, board, bool_check, output);
    }
    else if (piece == W_KNIGHT || piece == B_KNIGHT)
    {
        for (int i = 0; i < 8; i++)
        {
            int X = x + Knight_moves[i].x;
            int Y = y + Knight_moves[i].y;
            if (X >= 0 && X < 8 && Y >= 0 && Y < 8 && board[X][Y] * color <= 0)
                output = add_move(output.len, x, y, X, Y, color, board, bool_check, output);
        }
    }
    else if (piece == W_KING || piece == B_KING)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                int X = x + dx;
                int Y = y + dy;
                if (X >= 0 && X < 8 && Y >= 0 && Y < 8 && board[X][Y] * color <= 0)
                    output = add_move(output.len, x, y, X, Y, color, board, bool_check, output);
            }
        }
        if (!in_check_detection && x == 4 && (y == 0 || y == 7))
        {
            int row = (color == 1) ? 0 : 7;
            if (can_roque(board, x, y, color, 'r'))
                output = add_move(output.len, x, y, 6, row, color, board, bool_check, output);
            if (can_roque(board, x, y, color, 'l'))
                output = add_move(output.len, x, y, 2, row, color, board, bool_check, output);
        }
    }
    else if (piece == W_BISHOP || piece == B_BISHOP || piece == W_QUEEN || piece == B_QUEEN)
    {
        for (int dx = -1; dx <= 1; dx += 2)
        {
            for (int dy = -1; dy <= 1; dy += 2)
            {
                int d = 1;
                int X = x + d * dx;
                int Y = y + d * dy;
                while (X >= 0 && X < 8 && Y >= 0 && Y < 8 && board[X][Y] == EMPTY)
                {
                    output = add_move(output.len, x, y, X, Y, color, board, bool_check, output);
                    d++;
                    X = x + d * dx;
                    Y = y + d * dy;
                }
                if (X >= 0 && X < 8 && Y >= 0 && Y < 8 && board[X][Y] * color < 0)
                    output = add_move(output.len, x, y, X, Y, color, board, bool_check, output);
            }
        }
    }

    if (piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN)
    {
        for (int dx = -1; dx <= 1; dx += 2)
        {
            int d = 1;
            int X = x + d * dx;
            while (X >= 0 && X < 8 && board[X][y] == EMPTY)
            {
                output = add_move(output.len, x, y, X, y, color, board, bool_check, output);
                d++;
                X = x + d * dx;
            }
            if (X >= 0 && X < 8 && board[X][y] * color < 0)
                output = add_move(output.len, x, y, X, y, color, board, bool_check, output);
        }
        for (int dy = -1; dy <= 1; dy += 2)
        {
            int d = 1;
            int Y = y + d * dy;
            while (Y >= 0 && Y < 8 && board[x][Y] == EMPTY)
            {
                output = add_move(output.len, x, y, x, Y, color, board, bool_check, output);
                d++;
                Y = y + d * dy;
            }
            if (Y >= 0 && Y < 8 && board[x][Y] * color < 0)
                output = add_move(output.len, x, y, x, Y, color, board, bool_check, output);
        }
    }

    return output;
}

moves_struct add_move(int len, int xd, int yd,
                      int x, int y, int color, const Board board,
                      bool bool_check, moves_struct output)
{
    if (bool_check)
    {
        /* Un roi adverse ne peut jamais etre capture */
        if (board[x][y] == W_KING || board[x][y] == B_KING)
            return output;

        Board board2;
        fait_coup(board, board2, xd, yd, x, y);
        if (!est_echec(board2, color))
        {
            output.Moves[len].x = x;
            output.Moves[len].y = y;
            output.len++;
        }
    }
    else
    {
        output.Moves[len].x = x;
        output.Moves[len].y = y;
        output.len++;
    }
    return output;
}

bool est_echec(const Board board, int color)
{
    square coord = find_piece(board, (color == 1) ? W_KING : B_KING);
    if (coord.x == -1) return false;

    in_check_detection = true;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            if ((color == 1 && board[x][y] < 0) || (color == -1 && board[x][y] > 0))
            {
                moves_struct moves = get_moves(board, x, y, false);
                for (int k = 0; k < moves.len; k++)
                {
                    if (moves.Moves[k].x == coord.x && moves.Moves[k].y == coord.y)
                    {
                        in_check_detection = false;
                        return true;
                    }
                }
            }
        }
    }
    in_check_detection = false;
    return false;
}

bool has_legal_moves(const Board board, int color)
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            if (board[x][y] * color > 0)
            {
                moves_struct moves = get_moves(board, x, y, true);
                if (moves.len > 0)
                    return true;
            }
        }
    }
    return false;
}

void deroque(const Board board, int color, int x, int y)
{
    (void)color;
    if (board[x][y] == B_KING) { Roque.BlR = false; Roque.BrR = false; }
    if (board[x][y] == W_KING) { Roque.WlR = false; Roque.WrR = false; }
    if (board[x][y] == W_ROOK && x == 0 && y == 0) Roque.WlR = false;
    if (board[x][y] == W_ROOK && x == 7 && y == 0) Roque.WrR = false;
    if (board[x][y] == B_ROOK && x == 0 && y == 7) Roque.BlR = false;
    if (board[x][y] == B_ROOK && x == 7 && y == 7) Roque.BrR = false;
}

void deroque_dest(int x, int y)
{
    if (x == 0 && y == 0) Roque.WlR = false;
    if (x == 7 && y == 0) Roque.WrR = false;
    if (x == 0 && y == 7) Roque.BlR = false;
    if (x == 7 && y == 7) Roque.BrR = false;
}

bool can_roque(const Board board, int x, int y, int color, char side)
{
    if (est_echec(board, color))
        return false;

    int row = (color == 1) ? 0 : 7;
    if (side == 'r' && x == 4 && y == row)
    {
        bool allowed = (color == 1) ? Roque.WrR : Roque.BrR;
        if (allowed && board[5][row] == EMPTY && board[6][row] == EMPTY)
        {
            /* Verifie que la case de passage (5, row) n'est pas sous echec */
            Board transit;
            fait_coup(board, transit, 4, row, 5, row);
            if (!est_echec(transit, color))
                return true;
        }
    }
    if (side == 'l' && x == 4 && y == row)
    {
        bool allowed = (color == 1) ? Roque.WlR : Roque.BlR;
        if (allowed && board[3][row] == EMPTY && board[2][row] == EMPTY && board[1][row] == EMPTY)
        {
            /* Verifie que la case de passage (3, row) n'est pas sous echec */
            Board transit;
            fait_coup(board, transit, 4, row, 3, row);
            if (!est_echec(transit, color))
                return true;
        }
    }
    return false;
}
