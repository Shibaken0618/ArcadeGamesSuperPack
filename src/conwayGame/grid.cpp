#include "grid.hpp"
#include <raylib.h>

Color navy = {35, 15, 92, 255};
Color skyBlue = {8, 138, 208, 255};

void Grid::Draw() {
    for (int row=0; row<rows; row++) {
        for (int col=0; col<cols; col++) {
            Color color = cells[row][col] ? skyBlue : navy;
            DrawRectangle(col*cellSize, row*cellSize, cellSize-2, cellSize-2, color);
        }
    }
}

void Grid::SetValue(int row, int col, int val) {
    if (IsInBounds(row, col)) {
        cells[row][col] = val;
    }
}

int Grid::GetValue(int row, int col) {
    if (IsInBounds(row, col)) {
        return cells[row][col];
    }
    return 0;
}

bool Grid::IsInBounds(int row, int col) {
    if (row>=0 && row < rows && col >= 0 && col < cols) {
        return true;
    }
    return false;
}

void Grid::FillRandom() {
    for (int row=0; row<rows; row++) {
        for (int col=0; col < cols; col++) {
            int randomValue = GetRandomValue(0, 4);
            cells[row][col] = (randomValue == 4) ? 1 : 0;
        }
    }
}

void Grid::Clear() {
    for (int row=0; row<rows; row++) {
        for (int col=0; col<cols; col++) {
            cells[row][col] = 0;
        }
    }
}

void Grid::ToggleCell(int row, int col) {
    if (IsInBounds(row, col)) {
        cells[row][col] = !cells[row][col];
    }
}
