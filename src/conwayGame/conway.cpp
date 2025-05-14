#include "conway.hpp"
#include "simulation.hpp" // your existing Simulation class
#include <string>

// Colors
static const Color INDIGO = {12, 4, 64, 255};

// Internal state
namespace {
    enum class CState { Menu, Running };
    CState state;

    int windowWidth = 0;
    int windowHeight = 0;
    int fps = 12;
    int cellSize = 15;

    Simulation* sim = nullptr;
}

bool InitConway() {
    // Setup window dims to match main launcher
    windowWidth = GetScreenWidth();
    windowHeight = GetScreenHeight();
    fps = 12;
    cellSize = 15;

    // Initialize simulation
    sim = new Simulation(windowWidth, windowHeight, cellSize);
    state = CState::Menu;
    SetTargetFPS(fps);
    SetWindowTitle("Conway's Game of Life");
    return true;
}

void UpdateConway() {
    // Return to menu if BACKSPACE
    if (IsKeyPressed(KEY_BACKSPACE)) {
        state = CState::Menu;
        delete sim;
        sim = new Simulation(windowWidth, windowHeight, cellSize);
        SetWindowTitle("Conway's Game of Life");
        return;
    }

    if (state == CState::Menu) {
        if (IsKeyPressed(KEY_RIGHT) && cellSize < 50) cellSize++;
        if (IsKeyPressed(KEY_LEFT) && cellSize > 1)  cellSize--;
        if (IsKeyPressed(KEY_ENTER)) {
            delete sim;
            sim = new Simulation(windowWidth, windowHeight, cellSize);
            state = CState::Running;
            SetWindowTitle("Running Game of Life...");
        }
    } else {
        // Running state controls
        if (IsKeyPressed(KEY_SPACE)) {
            sim->Stop();
            SetWindowTitle("Stopped Game of Life...");
        } else if (IsKeyPressed(KEY_ENTER)) {
            sim->Start();
            SetWindowTitle("Running Game of Life...");
        } else if (IsKeyPressed(KEY_UP) && fps < 60) {
            fps += 2; SetTargetFPS(fps);
        } else if (IsKeyPressed(KEY_DOWN) && fps > 5) {
            fps -= 2; SetTargetFPS(fps);
        } else if (IsKeyPressed(KEY_R)) {
            sim->CreateRandomState();
        } else if (IsKeyPressed(KEY_C)) {
            sim->ClearGrid();
        }
        // Mouse toggle
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            int row = mp.y / cellSize;
            int col = mp.x / cellSize;
            sim->ToggleCell(row, col);
        }
        sim->Update();
    }
}

void DrawConway() {
    BeginDrawing();
    ClearBackground(INDIGO);

    if (state == CState::Menu) {
        DrawText("Conway's Game of Life", 295, 200, 40, LIGHTGRAY);
        DrawText("Controls:", 300, 300, 30, LIGHTGRAY);
        DrawText("<- / ->   : Adjust cell size (1 - 50)", 300, 340, 20, LIGHTGRAY);
        DrawText("Enter     : Start simulation", 300, 370, 20, LIGHTGRAY);
        DrawText("Space     : Pause / Resume", 300, 400, 20, LIGHTGRAY);
        DrawText("R         : Randomize", 300, 430, 20, LIGHTGRAY);
        DrawText("C         : Clear grid", 300, 460, 20, LIGHTGRAY);
        DrawText("Click on cells to manually select", 300, 490, 20, LIGHTGRAY);
        DrawText("Backspace : Back to Menu", 300, 520, 20, LIGHTGRAY);
        DrawText(TextFormat("Cell Size: %d", cellSize), 300, 580, 25, YELLOW);
        DrawText("Press Enter to begin", 300, 610, 25, GREEN);
    } else {
        sim->Draw();
    }

    EndDrawing();
}

void UnloadConway() {
    delete sim;
    sim = nullptr;
}
