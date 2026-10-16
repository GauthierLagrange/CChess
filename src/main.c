#include "renderer.h"
#include "ai.h"

#define MAX_HISTORY 512

typedef struct {
    Board board;
    roque roque_state;
    int turn;           /* 1 = Blancs, -1 = Noirs */
    square last_coupd;
    square last_coupf;
    bool first_coup;
    bool end;
    int gagnant;
    int eval_score;     /* Score estime par l'IA (positif = Blancs, negatif = Noirs) */
    bool has_eval;
} GameState;

static int evaluer_position(const Board board, int turn, int depth)
{
    if (!has_legal_moves(board, turn))
    {
        if (est_echec(board, turn))
            return (turn == 1) ? -90000 : 90000;
        return 0; /* Pat */
    }
    int d = (depth > 3) ? 3 : depth;
    return minimax(board, d, -100000, 100000, turn);
}

static void push_game_state(GameState history[], int *history_count, int *history_cursor,
                            const Board board, roque roque_state, int turn,
                            square last_coupd, square last_coupf, bool first_coup,
                            bool end, int gagnant, int eval_score, bool has_eval)
{
    /* Si on a recule dans l'historique et qu'on joue un nouveau coup, on bifurque */
    if (*history_cursor < *history_count - 1)
    {
        *history_count = *history_cursor + 1;
    }

    if (*history_count < MAX_HISTORY)
    {
        copy_board(board, history[*history_count].board);
        history[*history_count].roque_state = roque_state;
        history[*history_count].turn        = turn;
        history[*history_count].last_coupd  = last_coupd;
        history[*history_count].last_coupf  = last_coupf;
        history[*history_count].first_coup  = first_coup;
        history[*history_count].end         = end;
        history[*history_count].gagnant     = gagnant;
        history[*history_count].eval_score  = eval_score;
        history[*history_count].has_eval    = has_eval;
        *history_cursor = *history_count;
        (*history_count)++;
    }
}

static void restore_game_state(const GameState history[], int cursor,
                               Board board, roque *roque_state, int *turn,
                               square *last_coupd, square *last_coupf, bool *first_coup,
                               bool *end, int *gagnant, bool *select, bool *ai_thinking)
{
    copy_board(history[cursor].board, board);
    *roque_state = history[cursor].roque_state;
    *turn        = history[cursor].turn;
    *last_coupd  = history[cursor].last_coupd;
    *last_coupf  = history[cursor].last_coupf;
    *first_coup  = history[cursor].first_coup;
    *end         = history[cursor].end;
    *gagnant     = history[cursor].gagnant;
    *select      = false;
    *ai_thinking = false;
}

static void reinitialiser_partie(Board board, int player_color, bool vs_ai,
                                 int *turn, bool *end, int *gagnant,
                                 bool *select, bool *first_coup, bool *ai_thinking,
                                 square *last_coupd, square *last_coupf,
                                 GameState history[], int *history_count, int *history_cursor)
{
    setup_board(board);
    Roque = (roque){true, true, true, true};
    *turn        = 1;
    *end         = false;
    *gagnant     = 0;
    *select      = false;
    *first_coup  = false;
    *last_coupd  = (square){0, 0};
    *last_coupf  = (square){0, 0};
    *ai_thinking = (vs_ai && player_color == -1);

    *history_count  = 0;
    *history_cursor = 0;
    copy_board(board, history[0].board);
    history[0].roque_state = Roque;
    history[0].turn        = *turn;
    history[0].last_coupd  = *last_coupd;
    history[0].last_coupf  = *last_coupf;
    history[0].first_coup  = false;
    history[0].end         = false;
    history[0].gagnant     = 0;
    history[0].eval_score  = 0;
    history[0].has_eval    = true;
    *history_count         = 1;
}

int main(void)
{
    Board Board_Game;
    setup_board(Board_Game);

    int  depth         = 3;
    bool depth_editing = false;
    bool select        = false;
    square vect        = {0, 0};
    square selec_piece = {0, 0};
    struct infos_piece info;
    Vector2 mouse_pos  = {0, 0};
    moves_struct moves_piece_selec;

    bool vs_ai        = true;
    int  player_color = 1; /* 1 = Blancs, -1 = Noirs */
    int  turn         = 1; /* 1 = Blancs, -1 = Noirs */
    bool end          = false;
    int  gagnant      = 0;
    bool ai_thinking  = false;

    bool   first_coup = false;
    square last_coupd = {0, 0};
    square last_coupf = {0, 0};

    /* Gestion de l'historique */
    static GameState history[MAX_HISTORY];
    int history_count  = 0;
    int history_cursor = 0;
    float left_held_time  = 0.0f;
    float right_held_time = 0.0f;

    /* Enregistrement de l'etat initial */
    copy_board(Board_Game, history[0].board);
    history[0].roque_state = Roque;
    history[0].turn        = turn;
    history[0].last_coupd  = last_coupd;
    history[0].last_coupf  = last_coupf;
    history[0].first_coup  = false;
    history[0].end         = false;
    history[0].gagnant     = 0;
    history[0].eval_score  = 0;
    history[0].has_eval    = true;
    history_count          = 1;
    history_cursor         = 0;

    /* Definitions des rectangles de la barre laterale */
    const Rectangle btn_mode       = { BOARD_SIZE + 30, 145, 450, 52 };
    const Rectangle btn_color      = { BOARD_SIZE + 30, 255, 450, 52 };
    const Rectangle depth_box      = { BOARD_SIZE + 30, 365, 170, 52 };
    const Rectangle btn_minus      = { BOARD_SIZE + 220, 365, 65, 52 };
    const Rectangle btn_plus       = { BOARD_SIZE + 300, 365, 65, 52 };
    const Rectangle btn_restart    = { BOARD_SIZE + 30, 465, 450, 52 };

    const Rectangle btn_hist_first = { BOARD_SIZE + 30, 810, 102, 48 };
    const Rectangle btn_hist_prev  = { BOARD_SIZE + 146, 810, 102, 48 };
    const Rectangle btn_hist_next  = { BOARD_SIZE + 262, 810, 102, 48 };
    const Rectangle btn_hist_last  = { BOARD_SIZE + 378, 810, 102, 48 };

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CChess - Jeu d'echecs Minimax");
    SetWindowPosition((GetMonitorWidth(0) - WINDOW_WIDTH) / 2, (GetMonitorHeight(0) - WINDOW_HEIGHT) / 2);
    SetTargetFPS(60);
    init();

    while (!WindowShouldClose())
    {
        mouse_pos = GetMousePosition();

        /* Orientation de l'echiquier :
         * - En mode vs IA : le camp du joueur humain est en bas (inversion si joueur noir).
         * - En mode 1v1 Local : rotation 180 automatique pour que le joueur actif soit en bas.
         */
        bool flip_board = (vs_ai && player_color == -1) || (!vs_ai && turn == -1);

        /* Lecture de tous les caracteres saisis (supporte AZERTY et QWERTY sans distinction) */
        bool key_m_pressed = false;
        bool key_c_pressed = false;
        bool key_r_pressed = false;

        int char_code = GetCharPressed();
        while (char_code > 0)
        {
            if (char_code == 'm' || char_code == 'M')
                key_m_pressed = true;
            else if (char_code == 'c' || char_code == 'C')
                key_c_pressed = true;
            else if (char_code == 'r' || char_code == 'R')
                key_r_pressed = true;
            else if (vs_ai && char_code >= '1' && char_code <= '6')
                depth = char_code - '0';

            char_code = GetCharPressed();
        }

        /* 1. Raccourci M : Changer de Mode (vs IA / 1v1) */
        if (key_m_pressed || IsKeyPressed(KEY_M) || IsKeyPressed(KEY_SEMICOLON))
        {
            vs_ai = !vs_ai;
            depth_editing = false;
            select = false;
            if (vs_ai)
            {
                player_color = turn; /* L'humain prend la main sur le trait actuel */
                ai_thinking = false;
            }
            else
            {
                ai_thinking = false;
            }
        }

        /* 2. Raccourci C : Inverser les camps (en mode vs IA) */
        if (key_c_pressed || IsKeyPressed(KEY_C))
        {
            if (vs_ai)
            {
                player_color = -player_color;
                select = false;
                depth_editing = false;
                if (turn == -player_color && !end)
                    ai_thinking = true;
            }
        }

        /* 3. Raccourci R : Relancer la partie */
        if (key_r_pressed || IsKeyPressed(KEY_R))
        {
            reinitialiser_partie(Board_Game, player_color, vs_ai,
                                 &turn, &end, &gagnant, &select, &first_coup, &ai_thinking,
                                 &last_coupd, &last_coupf, history, &history_count, &history_cursor);
        }

        /* 4. Modification directe de la profondeur de calcul (Fleches Haut / Bas et touches 1 a 6) */
        if (vs_ai)
        {
            if (IsKeyPressed(KEY_UP) && depth < 6)
                depth++;
            if (IsKeyPressed(KEY_DOWN) && depth > 1)
                depth--;

            /* Touches numeriques (rangee superieure et pave numerique) */
            for (int k = 1; k <= 6; k++)
            {
                if (IsKeyPressed(KEY_ONE + (k - 1)) || IsKeyPressed(KEY_KP_1 + (k - 1)))
                    depth = k;
            }

            if (depth_editing)
            {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_ESCAPE))
                    depth_editing = false;
            }
        }

        /* 5. Navigation dans l'historique au clavier (Fleches Gauche / Droite) */
        if (!ai_thinking)
        {
            if (IsKeyPressed(KEY_LEFT))
            {
                if (history_cursor > 0)
                {
                    history_cursor--;
                    restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                       &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                }
            }
            if (IsKeyDown(KEY_LEFT))
            {
                left_held_time += GetFrameTime();
                if (left_held_time > 0.4f && history_cursor > 0)
                {
                    history_cursor = 0;
                    restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                       &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                }
            }
            else
            {
                left_held_time = 0.0f;
            }

            if (IsKeyPressed(KEY_RIGHT))
            {
                if (history_cursor < history_count - 1)
                {
                    history_cursor++;
                    restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                       &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                }
            }
            if (IsKeyDown(KEY_RIGHT))
            {
                right_held_time += GetFrameTime();
                if (right_held_time > 0.4f && history_cursor < history_count - 1)
                {
                    history_cursor = history_count - 1;
                    restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                       &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                }
            }
            else
            {
                right_held_time = 0.0f;
            }
        }

        /* Gestion des clics souris si l'IA n'est pas en train de calculer */
        if (!ai_thinking)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                /* 1. Bouton : Changer de Mode */
                if (CheckCollisionPointRec(mouse_pos, btn_mode))
                {
                    vs_ai = !vs_ai;
                    depth_editing = false;
                    select = false;
                    if (vs_ai)
                    {
                        player_color = turn;
                        ai_thinking = false;
                    }
                    else
                    {
                        ai_thinking = false;
                    }
                }
                /* 2. Bouton : Changer de camp (en mode vs IA) sans recommencer */
                else if (vs_ai && CheckCollisionPointRec(mouse_pos, btn_color))
                {
                    player_color = -player_color;
                    depth_editing = false;
                    select = false;
                    if (turn == -player_color && !end)
                        ai_thinking = true;
                }
                /* 3. Bouton : Relancer la partie */
                else if (CheckCollisionPointRec(mouse_pos, btn_restart))
                {
                    depth_editing = false;
                    reinitialiser_partie(Board_Game, player_color, vs_ai,
                                         &turn, &end, &gagnant, &select, &first_coup, &ai_thinking,
                                         &last_coupd, &last_coupf, history, &history_count, &history_cursor);
                }
                /* 4. Boutons historique */
                else if (CheckCollisionPointRec(mouse_pos, btn_hist_first))
                {
                    if (history_cursor > 0)
                    {
                        history_cursor = 0;
                        restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                           &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                    }
                }
                else if (CheckCollisionPointRec(mouse_pos, btn_hist_prev))
                {
                    if (history_cursor > 0)
                    {
                        history_cursor--;
                        restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                           &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                    }
                }
                else if (CheckCollisionPointRec(mouse_pos, btn_hist_next))
                {
                    if (history_cursor < history_count - 1)
                    {
                        history_cursor++;
                        restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                           &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                    }
                }
                else if (CheckCollisionPointRec(mouse_pos, btn_hist_last))
                {
                    if (history_cursor < history_count - 1)
                    {
                        history_cursor = history_count - 1;
                        restore_game_state(history, history_cursor, Board_Game, &Roque, &turn,
                                           &last_coupd, &last_coupf, &first_coup, &end, &gagnant, &select, &ai_thinking);
                    }
                }
                /* 5. Controles de profondeur */
                else if (vs_ai && CheckCollisionPointRec(mouse_pos, btn_minus))
                {
                    if (depth > 1) depth--;
                    depth_editing = false;
                }
                else if (vs_ai && CheckCollisionPointRec(mouse_pos, btn_plus))
                {
                    if (depth < 6) depth++;
                    depth_editing = false;
                }
                else if (vs_ai && CheckCollisionPointRec(mouse_pos, depth_box))
                {
                    depth_editing = true;
                }
                /* 6. Clic sur l'echiquier */
                else
                {
                    depth_editing = false;
                    vect = pos_vers_coord(mouse_pos, flip_board);

                    if (vect.x != -1 && vect.y != -1 && !end)
                    {
                        if (!select)
                        {
                            if (Board_Game[vect.x][vect.y] != EMPTY)
                            {
                                bool piece_valide = false;
                                if (vs_ai)
                                    piece_valide = (turn == player_color) &&
                                                   ((player_color == 1 && Board_Game[vect.x][vect.y] > 0) ||
                                                    (player_color == -1 && Board_Game[vect.x][vect.y] < 0));
                                else
                                    piece_valide = (turn == 1 ? Board_Game[vect.x][vect.y] > 0 : Board_Game[vect.x][vect.y] < 0);

                                if (piece_valide)
                                {
                                    select            = true;
                                    selec_piece       = vect;
                                    moves_piece_selec = get_moves(Board_Game, vect.x, vect.y, true);
                                }
                            }
                        }
                        else
                        {
                            bool coup_joue = false;
                            for (int k = 0; k < moves_piece_selec.len; k++)
                            {
                                if (moves_piece_selec.Moves[k].x != vect.x ||
                                    moves_piece_selec.Moves[k].y != vect.y)
                                    continue;

                                deroque(Board_Game, turn, selec_piece.x, selec_piece.y);
                                deroque_dest(vect.x, vect.y);

                                Board Board_Game2;
                                fait_coup(Board_Game, Board_Game2, selec_piece.x, selec_piece.y, vect.x, vect.y);
                                copy_board(Board_Game2, Board_Game);

                                last_coupd = selec_piece;
                                last_coupf = vect;
                                first_coup = true;
                                select     = false;
                                coup_joue  = true;

                                int next_color = -turn;

                                if (!has_legal_moves(Board_Game, next_color))
                                {
                                    end     = true;
                                    gagnant = est_echec(Board_Game, next_color) ? turn : 0;
                                }
                                else
                                {
                                    turn = next_color;
                                }

                                /* Enregistrement dans l'historique (avec bifurcation si on etait revenu en arriere) */
                                int quick_eval = evaluer_position(Board_Game, turn, depth);
                                push_game_state(history, &history_count, &history_cursor,
                                                Board_Game, Roque, turn, last_coupd, last_coupf, first_coup,
                                                end, gagnant, quick_eval, true);

                                if (vs_ai && turn == -player_color && !end)
                                    ai_thinking = true; /* L'IA calculera juste apres l'affichage de ce coup ! */

                                break;
                            }

                            if (!coup_joue)
                            {
                                bool piece_valide = false;
                                if (Board_Game[vect.x][vect.y] != EMPTY)
                                {
                                    if (vs_ai)
                                        piece_valide = (turn == player_color) &&
                                                       ((player_color == 1 && Board_Game[vect.x][vect.y] > 0) ||
                                                        (player_color == -1 && Board_Game[vect.x][vect.y] < 0));
                                    else
                                        piece_valide = (turn == 1 ? Board_Game[vect.x][vect.y] > 0 : Board_Game[vect.x][vect.y] < 0);
                                }

                                if (piece_valide)
                                {
                                    moves_piece_selec = get_moves(Board_Game, vect.x, vect.y, true);
                                    selec_piece       = vect;
                                }
                                else
                                {
                                    select = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* --- RENDU GRAPHIQUE --- */
        BeginDrawing();
        ClearBackground((Color){24, 26, 32, 255});

        /* 1. Affichage de l'echiquier et des pieces (avec orientation flip_board) */
        display_board_CIG(Board_Game, last_coupd.x, last_coupd.y, last_coupf.x, last_coupf.y, first_coup, flip_board);

        if (select)
            display_coup_possible(moves_piece_selec, flip_board);

        /* 2. Barre laterale */
        DrawRectangle(BOARD_SIZE, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT, (Color){30, 33, 40, 255});
        DrawLine(BOARD_SIZE, 0, BOARD_SIZE, WINDOW_HEIGHT, (Color){55, 60, 72, 255});

        /* Titre CCHESS */
        DrawText("C", BOARD_SIZE + 130, 28, 44, (Color){0, 140, 235, 255});
        DrawText(" C H E S S", BOARD_SIZE + 162, 28, 44, RAYWHITE);
        DrawLine(BOARD_SIZE + 30, 85, BOARD_SIZE + SIDEBAR_WIDTH - 30, 85, (Color){55, 60, 72, 255});

        /* Section : Mode */
        DrawText("Mode de jeu :", BOARD_SIZE + 30, 110, 22, LIGHTGRAY);
        bool hover_mode = CheckCollisionPointRec(mouse_pos, btn_mode);
        DrawRectangleRec(btn_mode, hover_mode ? (Color){55, 60, 75, 255} : (Color){42, 45, 55, 255});
        DrawRectangleLinesEx(btn_mode, 2, hover_mode ? SKYBLUE : GRAY);
        DrawText(vs_ai ? "Humain vs IA [M]" : "1v1 Local [M]", BOARD_SIZE + 100, 158, 24, WHITE);

        /* Section : Couleur du joueur */
        DrawText("Orientation / Camp :", BOARD_SIZE + 30, 220, 22, LIGHTGRAY);
        bool hover_color = CheckCollisionPointRec(mouse_pos, btn_color);
        DrawRectangleRec(btn_color, hover_color ? (Color){55, 60, 75, 255} : (Color){42, 45, 55, 255});
        DrawRectangleLinesEx(btn_color, 2, hover_color ? GOLD : GRAY);
        if (vs_ai)
            DrawText(player_color == 1 ? "Blancs (Vous) [C]" : "Noirs (Vous) [C]", BOARD_SIZE + 100, 268, 24, (player_color == 1 ? RAYWHITE : GOLD));
        else
            DrawText("Rotation auto par tour", BOARD_SIZE + 90, 268, 24, SKYBLUE);

        /* Section : Profondeur IA */
        if (vs_ai)
        {
            DrawText("Profondeur Minimax :", BOARD_SIZE + 30, 330, 22, LIGHTGRAY);

            /* Boite de saisie */
            DrawRectangleRec(depth_box, depth_editing ? (Color){45, 50, 75, 255} : (Color){42, 45, 55, 255});
            DrawRectangleLinesEx(depth_box, 2, depth_editing ? SKYBLUE : GRAY);
            DrawText(TextFormat("Prof. %d", depth), BOARD_SIZE + 55, 377, 24, depth_editing ? SKYBLUE : WHITE);

            /* Boutons - et + */
            bool hover_m = CheckCollisionPointRec(mouse_pos, btn_minus);
            bool hover_p = CheckCollisionPointRec(mouse_pos, btn_plus);

            DrawRectangleRec(btn_minus, hover_m ? DARKGRAY : (Color){45, 45, 55, 255});
            DrawRectangleLinesEx(btn_minus, 2, GRAY);
            DrawText("-", (int)btn_minus.x + 25, (int)btn_minus.y + 7, 34, WHITE);

            DrawRectangleRec(btn_plus, hover_p ? DARKGRAY : (Color){45, 45, 55, 255});
            DrawRectangleLinesEx(btn_plus, 2, GRAY);
            DrawText("+", (int)btn_plus.x + 23, (int)btn_plus.y + 7, 34, WHITE);
        }

        DrawLine(BOARD_SIZE + 30, 440, BOARD_SIZE + SIDEBAR_WIDTH - 30, 440, (Color){55, 60, 72, 255});

        /* Bouton Relancer */
        bool hover_restart = CheckCollisionPointRec(mouse_pos, btn_restart);
        DrawRectangleRec(btn_restart, hover_restart ? (Color){60, 90, 60, 255} : (Color){38, 55, 40, 255});
        DrawRectangleLinesEx(btn_restart, 2, hover_restart ? GREEN : (Color){60, 120, 70, 255});
        DrawText("Nouvelle Partie [R]", BOARD_SIZE + 115, 478, 24, RAYWHITE);

        /* 1. Boite d'etat de la partie (Tour, Echec, Fin) */
        Rectangle status_rect = { BOARD_SIZE + 30, 530, 450, 68 };
        DrawRectangleRec(status_rect, (Color){20, 22, 28, 255});
        DrawRectangleLinesEx(status_rect, 2, (Color){50, 55, 65, 255});

        if (end)
        {
            if (gagnant == 1)
                DrawText("Victoire des Blancs !", BOARD_SIZE + 85, 550, 26, WHITE);
            else if (gagnant == -1)
                DrawText("Victoire des Noirs !", BOARD_SIZE + 95, 550, 26, GOLD);
            else
                DrawText("Partie Nulle (Pat)", BOARD_SIZE + 105, 550, 26, LIGHTGRAY);
        }
        else if (ai_thinking)
        {
            DrawText("IA en reflexion...", BOARD_SIZE + 90, 542, 24, SKYBLUE);
            DrawText(TextFormat("Analyse prof. %d...", depth), BOARD_SIZE + 90, 568, 18, GRAY);
        }
        else
        {
            DrawText(turn == 1 ? "Trait aux Blancs" : "Trait aux Noirs", BOARD_SIZE + 90, 542, 24, (turn == 1 ? RAYWHITE : GOLD));
            if (est_echec(Board_Game, turn))
                DrawText("Roi en echec !", BOARD_SIZE + 270, 544, 20, RED);
        }

        /* 2. Boite d'Evaluation de l'IA (Score et Jauge visuelle) */
        Rectangle eval_box = { BOARD_SIZE + 30, 608, 450, 78 };
        DrawRectangleRec(eval_box, (Color){20, 22, 28, 255});
        DrawRectangleLinesEx(eval_box, 2, (Color){50, 55, 65, 255});

        char eval_str[32];
        char eval_label[32];
        Color eval_color;
        float eval_pct = 0.5f;

        int display_eval = history[history_cursor].eval_score;
        if (!history[history_cursor].has_eval)
        {
            display_eval = evaluer_position(history[history_cursor].board, history[history_cursor].turn, depth);
            history[history_cursor].eval_score = display_eval;
            history[history_cursor].has_eval   = true;
        }

        if (end)
        {
            if (gagnant == 1)      { snprintf(eval_str, sizeof(eval_str), "+Mat"); snprintf(eval_label, sizeof(eval_label), "(Blancs 1-0)"); eval_color = RAYWHITE; eval_pct = 1.0f; }
            else if (gagnant == -1){ snprintf(eval_str, sizeof(eval_str), "-Mat"); snprintf(eval_label, sizeof(eval_label), "(Noirs 0-1)");  eval_color = GOLD;     eval_pct = 0.0f; }
            else                   { snprintf(eval_str, sizeof(eval_str), "0.00"); snprintf(eval_label, sizeof(eval_label), "(Nulle 1/2)");  eval_color = LIGHTGRAY;eval_pct = 0.5f; }
        }
        else if (display_eval >= 80000)
        {
            snprintf(eval_str, sizeof(eval_str), "+Mat");
            snprintf(eval_label, sizeof(eval_label), "(Mat Blancs)");
            eval_color = RAYWHITE;
            eval_pct   = 1.0f;
        }
        else if (display_eval <= -80000)
        {
            snprintf(eval_str, sizeof(eval_str), "-Mat");
            snprintf(eval_label, sizeof(eval_label), "(Mat Noirs)");
            eval_color = GOLD;
            eval_pct   = 0.0f;
        }
        else
        {
            float pawns = (float)display_eval / 100.0f;
            if (display_eval > 20)
            {
                snprintf(eval_str, sizeof(eval_str), "+%.2f", pawns);
                snprintf(eval_label, sizeof(eval_label), "(Avantage Blancs)");
                eval_color = RAYWHITE;
            }
            else if (display_eval < -20)
            {
                snprintf(eval_str, sizeof(eval_str), "%.2f", pawns);
                snprintf(eval_label, sizeof(eval_label), "(Avantage Noirs)");
                eval_color = GOLD;
            }
            else
            {
                snprintf(eval_str, sizeof(eval_str), "0.00");
                snprintf(eval_label, sizeof(eval_label), "(Egalite)");
                eval_color = LIGHTGRAY;
            }

            eval_pct = 0.5f + (pawns / 20.0f);
            if (eval_pct < 0.04f) eval_pct = 0.04f;
            if (eval_pct > 0.96f) eval_pct = 0.96f;
        }

        DrawText("Estimation IA :", BOARD_SIZE + 45, 618, 21, LIGHTGRAY);
        if (ai_thinking)
        {
            DrawText("Calcul...", BOARD_SIZE + 205, 616, 24, SKYBLUE);
        }
        else
        {
            DrawText(eval_str, BOARD_SIZE + 200, 614, 26, eval_color);
            DrawText(eval_label, BOARD_SIZE + 300, 620, 18, GRAY);
        }

        /* Jauge visuelle horizontale avec reperes Blancs (B) et Noirs (N) */
        int gauge_x = BOARD_SIZE + 55;
        int gauge_y = 654;
        int gauge_w = 400;
        int gauge_h = 16;

        DrawText("B", gauge_x - 17, gauge_y, 16, RAYWHITE);
        DrawText("N", gauge_x + gauge_w + 6, gauge_y, 16, GOLD);

        /* Fond noir (camp noir) */
        DrawRectangle(gauge_x, gauge_y, gauge_w, gauge_h, (Color){35, 38, 48, 255});
        /* Portion blanche (camp blanc) */
        int white_w = (int)(gauge_w * eval_pct);
        DrawRectangle(gauge_x, gauge_y, white_w, gauge_h, RAYWHITE);
        /* Bordure */
        DrawRectangleLines(gauge_x, gauge_y, gauge_w, gauge_h, (Color){70, 75, 90, 255});
        /* Curseur central d'egalite (50%) */
        DrawLine(gauge_x + gauge_w / 2, gauge_y - 2, gauge_x + gauge_w / 2, gauge_y + gauge_h + 2, (Color){0, 140, 235, 255});

        /* 3. Dernier coup joue */
        Rectangle last_box = { BOARD_SIZE + 30, 696, 450, 48 };
        DrawRectangleRec(last_box, (Color){25, 28, 35, 255});
        DrawRectangleLinesEx(last_box, 2, (Color){45, 50, 60, 255});
        if (first_coup)
        {
            DrawText(TextFormat("Dernier coup : %c%d -> %c%d",
                                'a' + last_coupd.x, 1 + last_coupd.y,
                                'a' + last_coupf.x, 1 + last_coupf.y),
                     BOARD_SIZE + 70, 709, 21, YELLOW);
        }
        else
        {
            DrawText("Dernier coup : (Debut de partie)", BOARD_SIZE + 70, 709, 20, GRAY);
        }

        /* Section : Historique / Timeline */
        DrawLine(BOARD_SIZE + 30, 758, BOARD_SIZE + SIDEBAR_WIDTH - 30, 758, (Color){55, 60, 72, 255});
        DrawText(TextFormat("Historique : Coup %d / %d", history_cursor, history_count - 1),
                 BOARD_SIZE + 30, 774, 22, (history_cursor < history_count - 1) ? GOLD : RAYWHITE);

        /* Boutons de navigation */
        bool h_first = CheckCollisionPointRec(mouse_pos, btn_hist_first) && history_cursor > 0;
        bool h_prev  = CheckCollisionPointRec(mouse_pos, btn_hist_prev)  && history_cursor > 0;
        bool h_next  = CheckCollisionPointRec(mouse_pos, btn_hist_next)  && history_cursor < history_count - 1;
        bool h_last  = CheckCollisionPointRec(mouse_pos, btn_hist_last)  && history_cursor < history_count - 1;

        DrawRectangleRec(btn_hist_first, h_first ? (Color){55, 60, 80, 255} : (Color){38, 40, 50, 255});
        DrawRectangleLinesEx(btn_hist_first, 2, (history_cursor > 0) ? GRAY : DARKGRAY);
        DrawText("|<", (int)btn_hist_first.x + 38, (int)btn_hist_first.y + 11, 24, (history_cursor > 0) ? WHITE : DARKGRAY);

        DrawRectangleRec(btn_hist_prev, h_prev ? (Color){55, 60, 80, 255} : (Color){38, 40, 50, 255});
        DrawRectangleLinesEx(btn_hist_prev, 2, (history_cursor > 0) ? GRAY : DARKGRAY);
        DrawText("<", (int)btn_hist_prev.x + 44, (int)btn_hist_prev.y + 11, 24, (history_cursor > 0) ? WHITE : DARKGRAY);

        DrawRectangleRec(btn_hist_next, h_next ? (Color){55, 60, 80, 255} : (Color){38, 40, 50, 255});
        DrawRectangleLinesEx(btn_hist_next, 2, (history_cursor < history_count - 1) ? GRAY : DARKGRAY);
        DrawText(">", (int)btn_hist_next.x + 44, (int)btn_hist_next.y + 11, 24, (history_cursor < history_count - 1) ? WHITE : DARKGRAY);

        DrawRectangleRec(btn_hist_last, h_last ? (Color){55, 60, 80, 255} : (Color){38, 40, 50, 255});
        DrawRectangleLinesEx(btn_hist_last, 2, (history_cursor < history_count - 1) ? GRAY : DARKGRAY);
        DrawText(">|", (int)btn_hist_last.x + 38, (int)btn_hist_last.y + 11, 24, (history_cursor < history_count - 1) ? WHITE : DARKGRAY);

        if (history_cursor < history_count - 1)
        {
            DrawText("Relecture : jouez un coup pour bifurquer", BOARD_SIZE + 30, 868, 17, GOLD);
        }

        /* Pied de page : raccourcis */
        DrawLine(BOARD_SIZE + 30, 896, BOARD_SIZE + SIDEBAR_WIDTH - 30, 896, (Color){55, 60, 72, 255});
        DrawText("Raccourcis clavier :", BOARD_SIZE + 30, 914, 20, LIGHTGRAY);
        DrawText("<- / -> : Naviguer (maintenir: saut debut/fin)", BOARD_SIZE + 30, 940, 18, GRAY);
        DrawText("C : Inverser les camps (vs IA)", BOARD_SIZE + 30, 968, 18, DARKGRAY);
        DrawText("M : Changer de mode (vs IA / 1v1)", BOARD_SIZE + 30, 996, 18, DARKGRAY);
        DrawText("R : Relancer la partie", BOARD_SIZE + 30, 1024, 18, DARKGRAY);
        DrawText("1-6 ou Haut/Bas : Profondeur IA", BOARD_SIZE + 30, 1052, 18, DARKGRAY);

        EndDrawing(); /* Rendu de la frame termine */

        /* 3. Calcul de l'IA */
        if (ai_thinking && !end)
        {
            int ai_color = -player_color;
            info = pos_best_coup(Board_Game, depth, ai_color);

            /* On met a jour l'evaluation de la position precedente */
            if (history_count > 0)
            {
                history[history_count - 1].eval_score = info.valb;
                history[history_count - 1].has_eval   = true;
            }

            if (info.fin.x == -1)
            {
                end     = true;
                gagnant = est_echec(Board_Game, ai_color) ? player_color : 0;
            }
            else
            {
                deroque(Board_Game, ai_color, info.dep.x, info.dep.y);
                deroque_dest(info.fin.x, info.fin.y);

                Board new_board;
                fait_coup(Board_Game, new_board, info.dep.x, info.dep.y, info.fin.x, info.fin.y);
                copy_board(new_board, Board_Game);
                last_coupd = info.dep;
                last_coupf = info.fin;
                first_coup = true;

                if (!has_legal_moves(Board_Game, player_color))
                {
                    end     = true;
                    gagnant = est_echec(Board_Game, player_color) ? ai_color : 0;
                }
                else
                {
                    turn = player_color;
                }

                push_game_state(history, &history_count, &history_cursor,
                                Board_Game, Roque, turn, last_coupd, last_coupf, first_coup,
                                end, gagnant, info.valb, true);
            }
            ai_thinking = false;
        }
    }

    cleanup();
    CloseWindow();
    return 0;
}
