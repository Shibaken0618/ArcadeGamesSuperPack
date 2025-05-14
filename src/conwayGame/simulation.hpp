#pragma once
#include "grid.hpp"

class Simulation {
    private:
        Grid grid;
        Grid tempGrid;
        bool run;

    public:
        Simulation(int width, int height, int cellSize)
        :grid(width, height, cellSize), tempGrid(width, height, cellSize), run(false) {};
        void Draw();
        void SetCellValue(int row, int col, int val);
        int CountLiveNeighs(int row, int col);
        void Update();
        bool IsRunning() {return run;}
        void Start() {run = true;}
        void Stop() {run = false;}
        void ClearGrid();
        void CreateRandomState();
        void ToggleCell(int row, int col);
};