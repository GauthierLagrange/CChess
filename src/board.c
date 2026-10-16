#include "board.h"
#include <string.h>

/* Rq: dans [x][y] x represente les colonnes et y les lignes (0 = bas cote blanc) */
void display_board(const Board board)
{
    for (int y = 7; y >= 0; y--)
    {
        for (int x = 0; x < 8; x++)
            printf("%9d", board[x][y]);
        printf("\n");
    }
    printf("\n");
}

void setup_board(Board board)
{
    board[0][0] = W_ROOK;   board[1][0] = W_KNIGHT; board[2][0] = W_BISHOP;
    board[3][0] = W_QUEEN;  board[4][0] = W_KING;   board[5][0] = W_BISHOP;
    board[6][0] = W_KNIGHT; board[7][0] = W_ROOK;

    board[0][7] = B_ROOK;   board[1][7] = B_KNIGHT; board[2][7] = B_BISHOP;
    board[3][7] = B_QUEEN;  board[4][7] = B_KING;   board[5][7] = B_BISHOP;
    board[6][7] = B_KNIGHT; board[7][7] = B_ROOK;

    for (int x = 0; x < 8; x++)
    {
        board[x][1] = W_PAWN;
        board[x][6] = B_PAWN;
    }
    for (int y = 2; y < 6; y++)
        for (int x = 0; x < 8; x++)
            board[x][y] = EMPTY;
}

void setup_board_test_mat(Board board)
{
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++)
            board[x][y] = EMPTY;

    board[1][7] = B_KING;
    board[7][6] = W_ROOK;
    board[7][0] = W_QUEEN;
    board[0][0] = W_QUEEN;
    board[1][0] = W_KING;
}

void setup_board_test_roi(Board board)
{
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++)
            board[x][y] = EMPTY;

    board[1][7] = B_KING;
    board[0][7] = B_ROOK;
    board[0][0] = W_QUEEN;
    board[1][0] = W_KING;
}

void copy_board(const Board src, Board dst)
{
    memcpy(dst, src, sizeof(Board));
}

void fait_coup(const Board board, Board dst, int x, int y, int xf, int yf)
{
    memcpy(dst, board, sizeof(Board));

    int piece = board[x][y];
    if (piece == W_PAWN && yf == 7)
        dst[xf][yf] = W_QUEEN;
    else if (piece == B_PAWN && yf == 0)
        dst[xf][yf] = B_QUEEN;
    else
        dst[xf][yf] = piece;

    dst[x][y] = EMPTY;

    /* Deplacement automatique de la tour lors d'un roque */
    if (piece == W_KING && x == 4 && y == 0)
    {
        if (xf == 6)      { dst[5][0] = W_ROOK; dst[7][0] = EMPTY; } /* Petit roque blanc */
        else if (xf == 2) { dst[3][0] = W_ROOK; dst[0][0] = EMPTY; } /* Grand roque blanc */
    }
    else if (piece == B_KING && x == 4 && y == 7)
    {
        if (xf == 6)      { dst[5][7] = B_ROOK; dst[7][7] = EMPTY; } /* Petit roque noir */
        else if (xf == 2) { dst[3][7] = B_ROOK; dst[0][7] = EMPTY; } /* Grand roque noir */
    }
}

square find_piece(const Board board, int piece)
{
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++)
            if (board[x][y] == piece)
                return (square){x, y};
    return (square){-1, -1};
}
