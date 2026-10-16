#include "renderer.h"

static Texture2D bk, bn, bp, br, bb, bq;
static Texture2D wk, wn, wp, wr, wb, wq;
static Texture2D board_texture;

void init(void)
{
    bk = LoadTexture("img/chess_pieces/bK.png");
    bn = LoadTexture("img/chess_pieces/bN.png");
    bp = LoadTexture("img/chess_pieces/bP.png");
    br = LoadTexture("img/chess_pieces/bR.png");
    bb = LoadTexture("img/chess_pieces/bB.png");
    bq = LoadTexture("img/chess_pieces/bQ.png");
    wk = LoadTexture("img/chess_pieces/wK.png");
    wn = LoadTexture("img/chess_pieces/wN.png");
    wp = LoadTexture("img/chess_pieces/wP.png");
    wr = LoadTexture("img/chess_pieces/wR.png");
    wb = LoadTexture("img/chess_pieces/wB.png");
    wq = LoadTexture("img/chess_pieces/wQ.png");
    board_texture = LoadTexture("img/chess_board.png");

    SetTextureFilter(board_texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bk, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bn, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bp, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(br, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bb, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(bq, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wk, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wn, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wp, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wr, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wb, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(wq, TEXTURE_FILTER_BILINEAR);
}

void cleanup(void)
{
    UnloadTexture(bk);
    UnloadTexture(bn);
    UnloadTexture(bp);
    UnloadTexture(br);
    UnloadTexture(bb);
    UnloadTexture(bq);
    UnloadTexture(wk);
    UnloadTexture(wn);
    UnloadTexture(wp);
    UnloadTexture(wr);
    UnloadTexture(wb);
    UnloadTexture(wq);
    UnloadTexture(board_texture);
}

void display_board_CIG(const Board Board, int xd, int yd, int xf, int yf, bool first_coup, bool flip_board)
{
    /* Affichage du fond de l'echiquier (mise a l'echelle fluide) */
    DrawTexturePro(board_texture,
                   (Rectangle){0, 0, (float)board_texture.width, (float)board_texture.height},
                   (Rectangle){0, 0, (float)BOARD_SIZE, (float)BOARD_SIZE},
                   (Vector2){0, 0}, 0.0f, WHITE);

    if (first_coup)
        display_last_coup(xd, yd, xf, yf, flip_board);

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int piece = Board[x][y];
            if (piece == EMPTY)
                continue;

            Texture2D tex;
            switch (piece)
            {
                case W_PAWN:   tex = wp; break;
                case W_QUEEN:  tex = wq; break;
                case W_BISHOP: tex = wb; break;
                case W_ROOK:   tex = wr; break;
                case W_KNIGHT: tex = wn; break;
                case W_KING:   tex = wk; break;
                case B_PAWN:   tex = bp; break;
                case B_QUEEN:  tex = bq; break;
                case B_BISHOP: tex = bb; break;
                case B_ROOK:   tex = br; break;
                case B_KNIGHT: tex = bn; break;
                case B_KING:   tex = bk; break;
                default:       continue;
            }

            float draw_x = !flip_board ? (float)(x * CELL_SIZE) : (float)((7 - x) * CELL_SIZE);
            float draw_y = !flip_board ? (float)((7 - y) * CELL_SIZE) : (float)(y * CELL_SIZE);

            DrawTexturePro(tex,
                           (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                           (Rectangle){draw_x, draw_y, (float)CELL_SIZE, (float)CELL_SIZE},
                           (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
}

void display_coup_possible(moves_struct moves_piece_selec, bool flip_board)
{
    for (int k = 0; k < moves_piece_selec.len; k++)
    {
        float cx = (!flip_board ? moves_piece_selec.Moves[k].x : (7 - moves_piece_selec.Moves[k].x)) * CELL_SIZE + CELL_SIZE * 0.5f;
        float cy = (!flip_board ? (7 - moves_piece_selec.Moves[k].y) : moves_piece_selec.Moves[k].y) * CELL_SIZE + CELL_SIZE * 0.5f;
        DrawCircle((int)cx, (int)cy, 21, (Color){33, 150, 243, 200});
        DrawCircleLines((int)cx, (int)cy, 21, (Color){13, 71, 161, 240});
    }
}

void display_last_coup(int xd, int yd, int xf, int yf, bool flip_board)
{
    Color highlight = (Color){255, 235, 59, 110};
    int sx_d = (!flip_board ? xd : (7 - xd)) * CELL_SIZE;
    int sy_d = (!flip_board ? (7 - yd) : yd) * CELL_SIZE;
    int sx_f = (!flip_board ? xf : (7 - xf)) * CELL_SIZE;
    int sy_f = (!flip_board ? (7 - yf) : yf) * CELL_SIZE;
    DrawRectangle(sx_d, sy_d, CELL_SIZE, CELL_SIZE, highlight);
    DrawRectangle(sx_f, sy_f, CELL_SIZE, CELL_SIZE, highlight);
}

square pos_vers_coord(Vector2 pos, bool flip_board)
{
    if (pos.x < 0 || pos.x >= BOARD_SIZE || pos.y < 0 || pos.y >= BOARD_SIZE)
        return (square){-1, -1};

    int col = (int)(pos.x / CELL_SIZE);
    int row = (int)(pos.y / CELL_SIZE);

    if (col < 0 || col >= 8 || row < 0 || row >= 8)
        return (square){-1, -1};

    if (!flip_board)
        return (square){col, 7 - row};
    else
        return (square){7 - col, row};
}
