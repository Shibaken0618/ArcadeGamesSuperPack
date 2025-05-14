#include <raylib.h>
#include <vector>
#include <string>

// Module headers
#include "pongGame/pong.hpp"
#include "snakeGame/snake.hpp"
#include "asteroidsGame/asteroids.hpp"
#include "conwayGame/conway.hpp"
#include "spaceInvadersGame/invaders.hpp"

// Application states
enum class AppState {
    Menu,
    Pong_Init, Pong_Play,
    Snake_Init, Snake_Play,
    Asteroids_Init, Asteroids_Play,
    Conway_Init, Conway_Play,
    Invaders_Init, Invaders_Play,
    Exit
};

int main() {
    // Choose unified resolution (adjust if needed)
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "ArcadeGames");
    SetTargetFPS(60);

    AppState state = AppState::Menu;
    
    // Build menu buttons
    typedef std::pair<Rectangle, std::string> Button;
    std::vector<Button> menuButtons;
    int btnW = 300, btnH = 50;
    int startY = 150, spacing = 80;
    const char* names[] = {"Pong", "Snake", "Asteroids", "Conway's Game of Life", "Space Invaders"};
    for (int i = 0; i < 5; i++) {
        menuButtons.push_back({
            {(screenWidth - btnW)/2.0f, (float)startY + i*spacing, (float)btnW, (float)btnH},
            names[i]
        });
    }
    // Center-top back button
    Rectangle backBtn = {(screenWidth - 150)/2.0f, 10.0f, 150.0f, 40.0f};

    while (!WindowShouldClose() && state != AppState::Exit) {
        // Update logic
        if (state == AppState::Menu) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mp = GetMousePosition();
                for (int i = 0; i < menuButtons.size(); i++) {
                    if (CheckCollisionPointRec(mp, menuButtons[i].first)) {
                        state = static_cast<AppState>(static_cast<int>(AppState::Pong_Init) + i*2);
                    }
                }
            }
        } else {
            int idx = static_cast<int>(state) - static_cast<int>(AppState::Pong_Init);
            int game = idx / 2; // 0..4
            int phase = idx % 2; // 0=init,1=play
            switch (state) {
                case AppState::Pong_Init:       if (InitPong()) state = AppState::Pong_Play; else state = AppState::Menu; break;
                case AppState::Pong_Play:       UpdatePong();    break;

                case AppState::Snake_Init:      if (InitSnake()) state = AppState::Snake_Play; else state = AppState::Menu; break;
                case AppState::Snake_Play:      UpdateSnake();   break;

                case AppState::Asteroids_Init:  if (InitAsteroids()) state = AppState::Asteroids_Play; else state = AppState::Menu; break;
                case AppState::Asteroids_Play:  UpdateAsteroids();    break;

                case AppState::Conway_Init:     if (InitConway()) state = AppState::Conway_Play; else state = AppState::Menu; break;
                case AppState::Conway_Play:     UpdateConway();   break;

                case AppState::Invaders_Init:   if (InitInvaders()) state = AppState::Invaders_Play; else state = AppState::Menu; break;
                case AppState::Invaders_Play:   UpdateInvaders();  break;
                default: break;
            }
            // Back to menu button click when playing
            if (phase == 1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mp = GetMousePosition();
                if (CheckCollisionPointRec(mp, backBtn)) {
                    // Unload current game and return to menu immediately
                    switch (game) {
                        case 0: UnloadPong(); break;
                        case 1: UnloadSnake(); break;
                        case 2: UnloadAsteroids(); break;
                        case 3: UnloadConway(); break;
                        case 4: UnloadInvaders(); break;
                    }
                    state = AppState::Menu;
                }
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (state == AppState::Menu) {
            DrawText("ArcadeGames", (screenWidth - MeasureText("ArcadeGames", 40))/2, 50, 40, DARKBLUE);
            for (auto &b : menuButtons) {
                DrawRectangleRec(b.first, LIGHTGRAY);
                DrawText(b.second.c_str(), b.first.x + 20, b.first.y + 10, 24, BLACK);
            }
        } else {
            switch (state) {
                case AppState::Pong_Play:        DrawPong();        break;
                case AppState::Snake_Play:       DrawSnake();       break;
                case AppState::Asteroids_Play:   DrawAsteroids();   break;
                case AppState::Conway_Play:      DrawConway();      break;
                case AppState::Invaders_Play:    DrawInvaders();    break;
                default: break;
            }
            // Draw back button at center-top
            DrawRectangleRec(backBtn, RED);
            DrawText("Main Menu", backBtn.x + 15, backBtn.y + 8, 20, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
