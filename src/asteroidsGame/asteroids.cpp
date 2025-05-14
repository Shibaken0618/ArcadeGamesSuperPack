#include "asteroids.hpp"
#include <vector>
#include <fstream>
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

using namespace std;

// --- Constants and Enums ---
enum class ShipState { ALIVE, EXPLODED, INVINCIBLE };
enum class GameState { START_SCREEN, COUNTDOWN, PLAYING, GAME_OVER };
enum class AstSize { LARGE = 0, MED = 1, SMALL = 2 };

static const int ASTEROID_COUNT = 15;
static const int INITIAL_LIVES = 3;
static const char* HIGHSCORE_FILE = "src/asteroidsGame/highscore.txt";

// --- Utility ---
static Vector2 WrapPosition(const Vector2& pos) {
    Vector2 r = pos;
    if (r.x < 0) r.x += GetScreenWidth();
    if (r.x > GetScreenWidth()) r.x -= GetScreenWidth();
    if (r.y < 0) r.y += GetScreenHeight();
    if (r.y > GetScreenHeight()) r.y -= GetScreenHeight();
    return r;
}
static bool CheckOverlap(const Vector2& a, float ra, const Vector2& b, float rb) {
    return Vector2Distance(a, b) < (ra + rb);
}

// --- Ship ---
struct Ship {
    Vector2 pos;
    float rot;
    Vector2 vel;
    ShipState state;
    float explosionTimer;
    float invincibleTimer;
    int lives;

    Ship() { Reset(); }
    void Reset() {
        pos = {(float)GetScreenWidth()/2, (float)GetScreenHeight()/2};
        rot = 0;
        vel = {0,0};
        state = ShipState::ALIVE;
        explosionTimer = invincibleTimer = 0;
        lives = INITIAL_LIVES;
    }
    void Update() {
        float dt = GetFrameTime();
        if (state == ShipState::ALIVE || state == ShipState::INVINCIBLE) {
            if (IsKeyDown(KEY_LEFT)) rot -= 300*dt;
            if (IsKeyDown(KEY_RIGHT)) rot += 300*dt;
            if (IsKeyDown(KEY_UP)) {
                Vector2 fwd = Vector2Rotate({0,-1}, rot*DEG2RAD);
                vel = Vector2Add(vel, Vector2Scale(fwd, 30*dt));
            }
            vel = Vector2Scale(vel, 0.99f);
            pos = WrapPosition(Vector2Add(pos, vel));
            if (state == ShipState::INVINCIBLE) {
                invincibleTimer -= dt;
                if (invincibleTimer <= 0) state = ShipState::ALIVE;
            }
        } else if (state == ShipState::EXPLODED) {
            explosionTimer -= dt;
            if (explosionTimer <= 0) {
                state = ShipState::INVINCIBLE;
                pos = {(float)GetScreenWidth()/2, (float)GetScreenHeight()/2};
                vel = {0,0};
                invincibleTimer = 2.0f;
            }
        }
    }
    void Draw() const {
        if (state == ShipState::EXPLODED) {
            DrawCircleV(pos, 30*explosionTimer, ORANGE);
            return;
        }
        bool drawShip = true;
        if (state == ShipState::INVINCIBLE) drawShip = ((int)(invincibleTimer*10) % 2) == 0;
        if (!drawShip) return;
        Vector2 pts[4] = {
            Vector2Add(pos, Vector2Rotate({0,-16}, rot*DEG2RAD)),
            Vector2Add(pos, Vector2Rotate({-12,8}, rot*DEG2RAD)),
            Vector2Add(pos, Vector2Rotate({0,2}, rot*DEG2RAD)),
            Vector2Add(pos, Vector2Rotate({12,8}, rot*DEG2RAD))
        };
        for (int i = 0; i < 4; ++i) DrawLineV(pts[i], pts[(i+1)%4], WHITE);
    }
    void Explode() {
        if (state == ShipState::ALIVE || state == ShipState::INVINCIBLE) {
            state = ShipState::EXPLODED;
            explosionTimer = 1.0f;
            lives--;
        }
    }
};

// --- Asteroid ---
struct Asteroid {
    Vector2 pos;
    Vector2 vel;
    AstSize size;
    bool active;

    Asteroid(AstSize s, const Vector2& spawn, const Ship& ship): size(s), active(true) {
        // avoid spawning on top of ship
        do {
            pos = spawn.x < 0 ? Vector2{(float)GetRandomValue(0,GetScreenWidth()), (float)GetRandomValue(0,GetScreenHeight())} : spawn;
        } while (CheckOverlap(pos, Radius(), ship.pos, 20));
        float ang = GetRandomValue(0,360)*DEG2RAD;
        float speed = (float)GetRandomValue(20,70);
        vel = {cosf(ang)*speed, sinf(ang)*speed};
    }
    Asteroid(AstSize s, const Ship& ship): Asteroid(s, { -1,-1 }, ship) {}

    float Radius() const { return size == AstSize::LARGE ? 50 : size == AstSize::MED ? 30 : 15; }
    void Update() { if (active) pos = WrapPosition(Vector2Add(pos, Vector2Scale(vel, GetFrameTime()))); }
    void Draw() const { if (active) DrawCircleV(pos, Radius(), GRAY); }
};

// --- Bullet ---
struct Bullet {
    Vector2 pos;
    Vector2 vel;
    float life;
    bool active;
    Bullet(const Vector2& p, float rot) : pos(p), life(2.0f), active(true) {
        Vector2 dir = Vector2Rotate({0,-1}, rot*DEG2RAD);
        vel = Vector2Scale(dir, 400);
    }
    void Update() {
        pos = WrapPosition(Vector2Add(pos, Vector2Scale(vel, GetFrameTime())));
        life -= GetFrameTime();
        if (life <= 0) active = false;
    }
    void Draw() const { if (active) DrawCircleV(pos, 2, RED); }
};

// --- Globals ---
static Ship ship;
static vector<Asteroid> asteroids;
static vector<Bullet> bullets;
static int score = 0;
static int highScore = 0;
static bool gameOverSoundPlayed = false;
static double countdownStart = 0;
static Sound shootSfx, explodeSfx, gameOverSfx;
static GameState gameState;

// --- Highscore I/O ---
static void LoadHighScore() {
    ifstream f(HIGHSCORE_FILE);
    if (!(f >> highScore)) highScore = 0;
}
static void SaveHighScore() {
    ofstream f(HIGHSCORE_FILE);
    f << highScore;
}

// --- Spawn & Cleanup ---
static void SpawnAsteroids() {
    asteroids.clear();
    for (int i = 0; i < ASTEROID_COUNT; ++i)
        asteroids.emplace_back(AstSize::LARGE, ship);
}

// cleanup bullets
static void Cleanup(vector<Bullet>& v) {
    v.erase(remove_if(v.begin(), v.end(), [](auto& b){ return !b.active; }), v.end());
}
// cleanup asteroids and respawn if none left
static void Cleanup(vector<Asteroid>& v) {
    v.erase(remove_if(v.begin(), v.end(), [](auto& a){ return !a.active; }), v.end());
    if (v.empty()) SpawnAsteroids();
}

// --- Collisions ---
static void HandleCollisions() {
    // ship-asteroid
    if ((ship.state == ShipState::ALIVE) || (ship.state == ShipState::INVINCIBLE)) {
        for (auto& a : asteroids) {
            if (a.active && CheckOverlap(ship.pos, 12, a.pos, a.Radius())) {
                ship.Explode();
                PlaySound(explodeSfx);
                if (ship.lives <= 0) gameState = GameState::GAME_OVER;
                break;
            }
        }
    }
    // bullet-asteroid
    for (auto& b : bullets) if (b.active) {
        for (auto& a : asteroids) if (a.active) {
            if (CheckOverlap(b.pos, 2, a.pos, a.Radius())) {
                b.active = a.active = false;
                score += (3 - (int)a.size) * 10;
                if (a.size != AstSize::SMALL) {
                    Vector2 base = a.pos;
                    asteroids.emplace_back((AstSize)((int)a.size + 1), base + Vector2{10,10}, ship);
                    asteroids.emplace_back((AstSize)((int)a.size + 1), base - Vector2{10,10}, ship);
                }
                break;
            }
        }
    }
}

// --- Initialization ---
bool InitAsteroids() {
    InitAudioDevice();
    shootSfx    = LoadSound("src/asteroidsGame/SE/shoot.mp3");
    explodeSfx  = LoadSound("src/asteroidsGame/SE/explode.mp3");
    gameOverSfx = LoadSound("src/asteroidsGame/SE/gameOver.mp3");
    LoadHighScore(); score = 0; gameOverSoundPlayed = false;
    ship.Reset(); bullets.clear(); SpawnAsteroids();
    gameState = GameState::START_SCREEN;
    return true;
}

// --- Update ---
void UpdateAsteroids() {
    if (IsKeyPressed(KEY_BACKSPACE)) gameState = GameState::START_SCREEN;
    switch (gameState) {
        case GameState::START_SCREEN:
            if (IsKeyPressed(KEY_ENTER)) { gameState = GameState::COUNTDOWN; countdownStart = GetTime(); }
            break;
        case GameState::COUNTDOWN: {
            double elapsed = GetTime() - countdownStart;
            if (elapsed >= 3.0) gameState = GameState::PLAYING;
            break;
        }
        case GameState::PLAYING:
            if (IsKeyPressed(KEY_SPACE) && (ship.state == ShipState::ALIVE || ship.state == ShipState::INVINCIBLE)) {
                bullets.emplace_back(ship.pos, ship.rot);
                PlaySound(shootSfx);
            }
            ship.Update();
            for (auto& a : asteroids) a.Update();
            for (auto& b : bullets) b.Update();
            HandleCollisions();
            Cleanup(bullets); Cleanup(asteroids);
            if (gameState == GameState::GAME_OVER && !gameOverSoundPlayed) {
                PlaySound(gameOverSfx); gameOverSoundPlayed = true;
                if (score > highScore) { highScore = score; SaveHighScore(); }
            }
            break;
        case GameState::GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                ship.Reset(); bullets.clear(); SpawnAsteroids(); score = 0; gameOverSoundPlayed = false;
                gameState = GameState::START_SCREEN;
            }
            break;
    }
}

// --- Draw ---
void DrawAsteroids() {
    BeginDrawing(); ClearBackground(BLACK);
    switch (gameState) {
        case GameState::START_SCREEN:
            DrawText("ASTEROIDS", GetScreenWidth()/2-200, GetScreenHeight()/2-80, 60, WHITE);
            DrawText("Press ENTER to Start", GetScreenWidth()/2-200, GetScreenHeight()/2-20, 40, WHITE);
            DrawText("Backspace to menu", GetScreenWidth()/2-200, GetScreenHeight()/2+40, 20, WHITE);
            break;
        case GameState::COUNTDOWN: {
            int count = 3 - (int)(GetTime() - countdownStart);
            if (count < 1) count = 1;
            DrawText(TextFormat("%i", count), GetScreenWidth()/2-20, GetScreenHeight()/2-20, 60, WHITE);
            break;
        }
        case GameState::PLAYING:
            ship.Draw(); for (auto& a : asteroids) a.Draw(); for (auto& b : bullets) b.Draw();
            DrawText(TextFormat("Score:%04i", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Lives:%i", ship.lives), 10, 40, 20, WHITE);
            DrawText(TextFormat("High:%04i", highScore), GetScreenWidth()-140, 10, 20, WHITE);
            break;
        case GameState::GAME_OVER:
            DrawText("GAME OVER", GetScreenWidth()/2-200, GetScreenHeight()/2-40, 60, RED);
            DrawText("Press R to Restart", GetScreenWidth()/2-200, GetScreenHeight()/2+20, 40, WHITE);
            break;
    }
    EndDrawing();
}

// --- Unload ---
void UnloadAsteroids() {
    UnloadSound(shootSfx);
    UnloadSound(explodeSfx);
    UnloadSound(gameOverSfx);
    CloseAudioDevice();
}
