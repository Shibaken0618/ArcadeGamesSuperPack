#include "snake.hpp"
#include <raymath.h>
#include <fstream>
#include <string>
using namespace std;

// Colors and layout
static const Color SNAKE_GREEN = {120,230,92,255};
static const Color FOREST_GREEN = {30,102,12,255};
static const int CELL_SIZE = 20;
static const int CELL_COUNT_W = 48;
static const int CELL_COUNT_H = 32;
static const int OFFSET_X = 75;
static const int OFFSET_Y = 75;

// Timing and game state
enum class SState { Start, Countdown, Playing, GameOver };
static SState state;
static double lastUpdateTime;
static double countdownStart;
static int score;
static int highScore;

// Audio
static Sound eatSound;
static Sound collSound;

// Snake and food data
static deque<Vector2> body;
static Vector2 direction;
static bool addSegment;
static Texture2D foodTex;
static Vector2 foodPos;

// Helpers
static bool ElemInDeque(const Vector2& v, const deque<Vector2>& dq) {
    for (auto &e : dq) if (Vector2Equals(e, v)) return true;
    return false;
}
static Vector2 RandCell() {
    return { (float)GetRandomValue(0, CELL_COUNT_W-1), (float)GetRandomValue(0, CELL_COUNT_H-1) };
}
static void PlaceFood() {
    Vector2 p = RandCell();
    while (ElemInDeque(p, body)) p = RandCell();
    foodPos = p;
}
static bool Interval(double interval) {
    double t = GetTime();
    if (t - lastUpdateTime >= interval) { lastUpdateTime = t; return true; }
    return false;
}
static void LoadHighScore() {
    ifstream f("src/snakeGame/highscore.txt"); if (f >> highScore) {};
}
static void SaveHighScore() {
    ofstream f("src/snakeGame/highscore.txt"); if (f.is_open()) f << highScore;
}

bool InitSnake() {
    InitAudioDevice();
    eatSound = LoadSound("src/snakeGame/SE/eat.mp3");
    collSound = LoadSound("src/snakeGame/SE/coll.mp3");
    LoadHighScore(); score = 0;

    // Initialize snake
    body = {{16,15},{15,15},{14,15}};
    direction = {1,0};
    addSegment = false;
    lastUpdateTime = GetTime();

    // Load food
    Image img = LoadImage("src/snakeGame/Graphics/food.png");
    ImageResize(&img, CELL_SIZE, CELL_SIZE);
    foodTex = LoadTextureFromImage(img);
    UnloadImage(img);
    PlaceFood();

    state = SState::Start;
    countdownStart = 0;
    return true;
}

void UpdateSnake() {
    // Global back to menu
    if (IsKeyPressed(KEY_BACKSPACE) && state != SState::Start) {
        state = SState::Start; score = 0; PlaceFood(); return;
    }
    // Input
    if (state == SState::Playing) {
        if (IsKeyPressed(KEY_UP) && direction.y != 1) direction = {0,-1};
        if (IsKeyPressed(KEY_DOWN) && direction.y != -1) direction = {0,1};
        if (IsKeyPressed(KEY_LEFT) && direction.x != 1) direction = {-1,0};
        if (IsKeyPressed(KEY_RIGHT)&& direction.x != -1) direction = {1,0};
    }
    switch (state) {
        case SState::Start:
            if (IsKeyPressed(KEY_ENTER)) { state = SState::Countdown; countdownStart = GetTime(); }
            break;
        case SState::Countdown:
            if (GetTime() - countdownStart >= 3.0) { state = SState::Playing; lastUpdateTime = GetTime(); }
            break;
        case SState::Playing:
            if (Interval(0.1)) {
                Vector2 head = body.front();
                body.push_front(Vector2Add(head, direction));
                if (!addSegment) body.pop_back(); else addSegment = false;
                // Food collision
                if (Vector2Equals(body.front(), foodPos)) {
                    addSegment = true; score++; if (score > highScore) highScore = score;
                    PlaySound(eatSound); PlaceFood();
                }
                // Edge collision
                auto &h = body.front();
                if (h.x < 0 || h.x >= CELL_COUNT_W || h.y < 0 || h.y >= CELL_COUNT_H) {
                    state = SState::GameOver; PlaySound(collSound); SaveHighScore();
                }
                // Self collision
                deque<Vector2> tmp = body; tmp.pop_front();
                if (ElemInDeque(body.front(), tmp)) {
                    state = SState::GameOver; PlaySound(collSound); SaveHighScore();
                }
            }
            break;
        case SState::GameOver:
            if (IsKeyPressed(KEY_R)) {
                body = {{16,15},{15,15},{14,15}}; direction = {1,0}; score = 0; PlaceFood(); state = SState::Start;
            }
            break;
    }
}

void DrawSnake() {
    ClearBackground(SNAKE_GREEN);
    // Inner box
    DrawRectangleLinesEx({(float)OFFSET_X+40,(float)OFFSET_Y-5,
        (float)(CELL_COUNT_W*CELL_SIZE+10),(float)(CELL_COUNT_H*CELL_SIZE+10)},
        5, FOREST_GREEN);
    // Title
    DrawText("Snakey Snake", OFFSET_X+40, OFFSET_Y - 50, 40, FOREST_GREEN);
    switch (state) {
        case SState::Start:
            DrawText("Press ENTER to start", OFFSET_X+60, OFFSET_Y + CELL_COUNT_H*CELL_SIZE/2 - 40, 30, FOREST_GREEN);
            break;
        case SState::Countdown: {
            int cnt = 3 - (int)(GetTime() - countdownStart);
            DrawText(TextFormat("%i", cnt), GetScreenWidth()/2 - 10, OFFSET_Y + CELL_COUNT_H*CELL_SIZE/2 - 20, 60, FOREST_GREEN);
            break;
        }
        case SState::Playing:
            DrawTexture(foodTex, OFFSET_X + foodPos.x*CELL_SIZE, OFFSET_Y + foodPos.y*CELL_SIZE, WHITE);
            for (auto &seg : body) DrawRectangleRounded({OFFSET_X + seg.x*CELL_SIZE, OFFSET_Y + seg.y*CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, 0.7f, 8, FOREST_GREEN);
            break;
        case SState::GameOver:
            DrawText("Game Over! Press R to restart", OFFSET_X+60, OFFSET_Y + CELL_COUNT_H*CELL_SIZE/2 -20, 30, FOREST_GREEN);
            break;
    }
    // Footer: scores
    DrawText(TextFormat("Score: %i", score), OFFSET_X+40, OFFSET_Y + CELL_COUNT_H*CELL_SIZE + 10, 20, FOREST_GREEN);
    DrawText(TextFormat("High: %i", highScore), GetScreenWidth() - OFFSET_X - MeasureText("High: 0000", 20)-20, OFFSET_Y + CELL_COUNT_H*CELL_SIZE + 10, 20, FOREST_GREEN);
}

void UnloadSnake() {
    UnloadTexture(foodTex);
    UnloadSound(eatSound);
    UnloadSound(collSound);
    CloseAudioDevice();
}
