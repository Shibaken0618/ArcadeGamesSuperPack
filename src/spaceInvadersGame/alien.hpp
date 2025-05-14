#pragma once
#include <raylib.h>

class Alien {
    private:

    public:
        Alien(int type, Vector2 position);
        void Draw();
        void Update(int direction, float speedMul);
        int GetType();
        static void UnloadImages();
        static Texture2D alienImages[3];
        int type;
        Vector2 position;
        Rectangle getRect();
};