#pragma once
#include "spaceship.hpp"
#include "obstacle.hpp"
#include "alien.hpp"
#include "mysteryship.hpp"

class Game {
    private:
        Spaceship spaceship;
        void DeleteInactivelasers();
        std::vector<Obstacle> CreateObstacles();
        std::vector<Obstacle> obstacles;
        std::vector<Alien> CreateAliens();
        std::vector<Alien> aliens;
        void MoveAliens();
        int aliensDirection;
        void MoveDownAliens(int distance);
        std::vector<Laser> alienLasers;
        void AliensShootLaser();
        constexpr static float alienLaserInterval = 0.35;
        float timeLastAlienFired;
        MysteryShip mysteryship;
        float mysteryShipSpawnInterval;
        float timeLastSpawn;
        float alienSpeedMul;
        void CheckForCollisions();
        void GameOver();
        void CheckHighScore();
        void SaveHighscoreToFile(int highscore);
        int LoadHighScore();
    public:
        Game();
        ~Game();
        void Draw();
        void Update();
        void HandleInput();
        void Reset();
        void InitGame();
        int lives;
        int score;
        int highscore;
        bool run;
};