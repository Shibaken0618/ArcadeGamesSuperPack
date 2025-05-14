#include "simulation.hpp"
#include <vector>
#include <utility>


void Simulation::Draw() {
    grid.Draw();
}

void Simulation::SetCellValue(int row, int col, int val) {
    grid.SetValue(row, col, val);
}

int Simulation::CountLiveNeighs(int row, int col) {
    int liveNeighs = 0;
    std::vector<std::pair<int, int>> neighborOffsets = 
    {
        {-1,0}, {1,0}, {0,-1}, {0,1}, {-1,-1}, {-1,1}, {1,-1}, {1,1}
    };

    for (const auto& offset : neighborOffsets) {
        int neighRow = (row + offset.first + grid.GetRows()) % grid.GetRows();
        int neighCol = (col + offset.second + grid.GetCols()) % grid.GetCols();
        liveNeighs += grid.GetValue(neighRow, neighCol);
    }
    return liveNeighs;
}

void Simulation::Update() {
    if (IsRunning()) {
        for (int row=0; row<grid.GetRows(); row++) {
            for (int col=0; col < grid.GetCols(); col++) {
                int liveNeighs = CountLiveNeighs(row, col);
                int cellValue = grid.GetValue(row, col);
                if (cellValue == 1) {
                    if (liveNeighs > 3 || liveNeighs < 2) {
                        tempGrid.SetValue(row, col, 0);
                    } else {
                        tempGrid.SetValue(row, col, 1);
                    }
                } else {
                    if (liveNeighs == 3) {
                        tempGrid.SetValue(row, col, 1);
                    } else {
                        tempGrid.SetValue(row, col, 0);
                    }
                }
            }
        }
        grid = tempGrid;
    }
}

void Simulation::ClearGrid() {
    if (!IsRunning()) {
        grid.Clear();
    }
}

void Simulation::CreateRandomState() {
    if (!IsRunning()) {
        grid.FillRandom();
    }
}

void Simulation::ToggleCell(int row, int col) {
    if (!IsRunning()) {
        grid.ToggleCell(row, col);
    }
}