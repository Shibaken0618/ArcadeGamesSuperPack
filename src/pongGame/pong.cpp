#include "pong.hpp"
#include <cmath>

// Game settings
enum class PongState { Title, Countdown, Playing, GameOver };
static const int WIN_SCORE = 6;

// Module state
namespace {
    PongState state;
    int player1Score;
    int player2Score;
    double countdownStart;

    Sound hitSound;
    Sound scoreSound;

    struct Ball {
        Vector2 position;
        Vector2 velocity;
        float speed;
        int radius;

        void Init(int w, int h) {
            radius = 20;
            position = { w/2.0f, h/2.0f };
            speed = 300.0f;
            float angle = GetRandomValue(-45, 45) * DEG2RAD;
            int dir = GetRandomValue(0,1) ? 1 : -1;
            velocity = { dir * cosf(angle), sinf(angle) };
        }
        void Reset(int w, int h) { Init(w,h); }
        void Update(float dt, int w, int h) {
            position.x += velocity.x * speed * dt;
            position.y += velocity.y * speed * dt;
            // top/bottom bounce
            if (position.y - radius <= 0) {
                position.y = radius; velocity.y *= -1; PlaySound(hitSound);
            }
            if (position.y + radius >= h) {
                position.y = h - radius; velocity.y *= -1; PlaySound(hitSound);
            }
            // score
            if (position.x - radius <= 0) {
                player2Score++; PlaySound(scoreSound); Reset(w,h);
            }
            if (position.x + radius >= w) {
                player1Score++; PlaySound(scoreSound); Reset(w,h);
            }
        }
        void Draw() const { DrawCircleV(position, radius, RED); }
        void Collide(const Rectangle& paddleRect) {
            PlaySound(hitSound);
            velocity.x *= -1;
            float center = paddleRect.y + paddleRect.height/2.0f;
            float diff = (position.y - center) / (paddleRect.height/2.0f);
            float angle = diff * 45.0f * DEG2RAD;
            int dir = velocity.x < 0 ? -1 : 1;
            velocity.x = dir * cosf(angle);
            velocity.y = sinf(angle);
            speed *= 1.05f;
        }
    } ball;

    struct Paddle {
        float x,y,width,height,speed;
        bool isPlayer;
        void Init(int w, int h, bool player) {
            width=25; height=120; speed=400;
            y = h/2.0f - height/2.0f;
            x = player ? w - width - 10 : 10;
            isPlayer = player;
        }
        void Update(float dt, int h, float ballY=0) {
            if (isPlayer) {
                if (IsKeyDown(KEY_UP)) y -= speed*dt;
                if (IsKeyDown(KEY_DOWN)) y += speed*dt;
            } else {
                float center = y + height/2.0f;
                if (center < ballY) y += speed*dt; else y -= speed*dt;
            }
            if (y < 10) y = 10;
            if (y + height > h - 10) y = h - height - 10;
        }
        void Draw() const { DrawRectangleRounded({x,y,width,height}, 0.8f, 0, WHITE); }
        Rectangle Rect() const { return { x,y,width,height }; }
    } player1, player2;
}

bool InitPong() {
    // reset scores and state
    player1Score = player2Score = 0;
    state = PongState::Title;
    // load audio
    InitAudioDevice();
    hitSound = LoadSound("src/pongGame/SE/tap.mp3");
    scoreSound = LoadSound("src/pongGame/SE/score.mp3");
    // init entities
    int w = GetScreenWidth(), h = GetScreenHeight();
    ball.Init(w,h);
    player1.Init(w,h,true);
    player2.Init(w,h,false);
    countdownStart = 0;
    return true;
}

void UpdatePong() {
    float dt = GetFrameTime();
    int w = GetScreenWidth(), h = GetScreenHeight();
    // global back to menu
    if (IsKeyPressed(KEY_BACKSPACE) && state != PongState::Title) {
        state = PongState::Title;
        player1Score = player2Score = 0;
        ball.Reset(w,h);
    }
    switch (state) {
        case PongState::Title:
            if (IsKeyPressed(KEY_ENTER)) { state = PongState::Countdown; countdownStart = GetTime(); }
            break;
        case PongState::Countdown: {
            if (GetTime() - countdownStart >= 3.0) state = PongState::Playing;
            break;
        }
        case PongState::Playing:
            ball.Update(dt,w,h);
            player1.Update(dt,h, ball.position.y);
            player2.Update(dt,h, ball.position.y);
            if (CheckCollisionCircleRec(ball.position, ball.radius, player1.Rect()))
                ball.Collide(player1.Rect());
            if (CheckCollisionCircleRec(ball.position, ball.radius, player2.Rect()))
                ball.Collide(player2.Rect());
            if (player1Score >= WIN_SCORE || player2Score >= WIN_SCORE)
                state = PongState::GameOver;
            break;
        case PongState::GameOver:
            if (IsKeyPressed(KEY_R)) {
                player1Score = player2Score = 0;
                ball.Reset(w,h);
                state = PongState::Title;
            }
            break;
    }
}

void DrawPong() {
    int w = GetScreenWidth(), h = GetScreenHeight();
    // draw background and UI
    ClearBackground(BLACK);
    BeginDrawing();
    switch (state) {
        case PongState::Title:
            DrawText("PONG PONG", w/2 - 170, h/2 - 80, 60, YELLOW);
            DrawText("Press ENTER to Start", w/2 - 220, h/2 - 20, 40, WHITE);
            DrawText("Controls:", w/2 - 200, h/2 + 40, 30, WHITE);
            DrawText("- UP/DOWN to move", w/2 - 200, h/2 + 80, 20, WHITE);
            DrawText("- BACKSPACE to main menu", w/2 - 200, h/2 + 110, 20, WHITE);
            break;
        case PongState::Countdown: {
            int cnt = 3 - (int)(GetTime() - countdownStart);
            DrawText(TextFormat("%i", cnt), w/2 - 20, h/2 - 40, 80, WHITE);
            break;
        }
        case PongState::Playing:
            DrawCircle(w/2, h/2, 150, GRAY);
            DrawLine(w/2, 0, w/2, h, WHITE);
            ball.Draw(); player1.Draw(); player2.Draw();
            DrawText(TextFormat("%i", player2Score), w/4 - 20, 20, 80, WHITE);
            DrawText(TextFormat("%i", player1Score), 3*w/4 - 20, 20, 80, WHITE);
            break;
        case PongState::GameOver:
            DrawText(TextFormat("%s Wins!", (player1Score>player2Score)?"Player 1":"Player 2"),
                     w/2 - 200, h/2 - 60, 60, YELLOW);
            DrawText("Press R to Restart", w/2 - 180, h/2 + 20, 40, WHITE);
            DrawText("BACKSPACE for Menu", w/2 - 180, h/2 + 60, 20, WHITE);
            break;
    }
    EndDrawing();
}

void UnloadPong() {
    UnloadSound(hitSound);
    UnloadSound(scoreSound);
    CloseAudioDevice();
}
