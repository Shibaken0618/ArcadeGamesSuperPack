#pragma once 
#include <raylib.h>

class Laser {
    private:
        Vector2 position;
        int speed;
    public:
        Laser(Vector2 position, int speed);
        ~Laser();
        void Draw();
        void Update();
        bool active;
        Rectangle getRect();
};