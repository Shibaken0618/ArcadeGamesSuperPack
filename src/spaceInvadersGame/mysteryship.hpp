#pragma once
#include <raylib.h>

class MysteryShip {
    private:    
        Vector2 position;
        Texture2D image;
        int speed;

    public:
        MysteryShip();
        ~MysteryShip();
        void Draw();
        void Update();
        void Spawn();
        bool alive;
        Rectangle getRect();
};