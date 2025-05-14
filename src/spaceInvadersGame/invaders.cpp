#include "invaders.hpp"
#include "game.hpp"
#include <raylib.h>
#include <string>
#include <cmath>

// Internal state
namespace {
    enum class IState { Start, Countdown, Playing };
    IState state = IState::Start;

    Game* game = nullptr;
    Texture2D shipTex;
    float countdownTimer = 3.0f;
    const int offset = 80;

    std::string FormatWithLeadingZeros(int number, int width) {
        std::string txt = std::to_string(number);
        int pad = width - (int)txt.length();
        return std::string(pad > 0 ? pad : 0, '0') + txt;
    }
}

bool InitInvaders() {
    // assume window already initialized by main launcher
    shipTex = LoadTexture("data/spaceInvaders/Graphics/spaceship.png");
    game = new Game();
    game->InitGame();
    state = IState::Start;
    countdownTimer = 3.0f;
    return true;
}

void UpdateInvaders() {
    // ——— GLOBAL INPUT ———
    if (IsKeyPressed(KEY_BACKSPACE)) {
        // full reset back to Start
        state = IState::Start;
        game->Reset();
        game->InitGame();
        return;
    }

    // ——— STATE‑SPECIFIC INPUT ———
    if (state == IState::Start) {
        if (IsKeyPressed(KEY_ENTER)) {
            state = IState::Countdown;
            countdownTimer = 3.0f;
        }
    }
    else if (state == IState::Playing) {
        // allow 'R' to restart once it's Game Over
        if (!game->run && IsKeyPressed(KEY_R)) {
            game->Reset();
            game->InitGame();
            state = IState::Start;
            return;
        }
        game->HandleInput();
    }

    // ——— UPDATE ———
    if (state == IState::Countdown) {
        countdownTimer -= GetFrameTime();
        if (countdownTimer <= 0.0f) {
            state = IState::Playing;
        }
    }
    else if (state == IState::Playing) {
        game->Update();
    }
}

void DrawInvaders() {
    BeginDrawing();
    ClearBackground(BLACK);

    int winW = GetScreenWidth();
    int winH = GetScreenHeight();

    switch (state) {
        case IState::Start:
            DrawText("SPACE INVADERS",        145, 200, 64, YELLOW);
            DrawText("Press [ENTER] to Start",220, 300, 32, WHITE);
            DrawText("Controls:",             220, 360, 28, WHITE);
            DrawText("- <- / -> to Move",     220, 400, 24, WHITE);
            DrawText("- SPACE to Shoot",      220, 430, 24, WHITE);
            DrawText("- R to Restart after Game Over", 220, 460, 24, WHITE);
            DrawText("- BACKSPACE to Return Here",     220, 490, 24, WHITE);
            break;

        case IState::Countdown: {
            int secs = (int)ceil(countdownTimer);
            DrawText(TextFormat("Starting in %02i", secs), 260, 350, 48, RAYWHITE);
            break;
        }

        case IState::Playing:
            // Border & UI
            DrawRectangleRoundedLines({10, 10, (float)winW - 20, (float)winH - 20}, 0.18f, 20, YELLOW);
            DrawLineEx({(float)offset, (float)(winH - offset)},
                       {(float)(winW - offset), (float)(winH - offset)}, 3, YELLOW);

            // Status text
            if (game->run)
                DrawText("Defending...", 940, 730, 36, YELLOW);
            else
                DrawText("Game Over",    940, 730, 36, YELLOW);

            // Lives
            {
                float x = 75.0f;
                for (int i = 0; i < game->lives; i++) {
                    DrawTextureV(shipTex, { x, 730 }, WHITE);
                    x += shipTex.width + 10;
                }
            }

            // Scores
            DrawText("SCORE",      60,  20, 36, YELLOW);
            DrawText(FormatWithLeadingZeros(game->score,     5).c_str(), 60, 50, 36, YELLOW);
            DrawText("HIGH SCORE", 920, 20, 36, YELLOW);
            DrawText(FormatWithLeadingZeros(game->highscore, 5).c_str(), 1050, 50, 36, YELLOW);

            // Game world
            game->Draw();
            break;
    }

    EndDrawing();
}

void UnloadInvaders() {
    UnloadTexture(shipTex);
    delete game;
    game = nullptr;
}
