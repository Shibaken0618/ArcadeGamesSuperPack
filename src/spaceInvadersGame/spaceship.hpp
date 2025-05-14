#pragma once
#include <vector>
#include <raylib.h>
#include "laser.hpp"

class Spaceship {
    private:
        Texture2D image;
        Vector2 position;
        double lastFireTime;

    public:
        Spaceship();
        ~Spaceship();
        void Draw();
        void MoveLeft();
        void MoveRight();
        void FireLaser();
        std::vector<Laser> lasers;
        Rectangle getRect();
        void Reset();
};