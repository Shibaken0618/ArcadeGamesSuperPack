#include "game.hpp"
#include <fstream>
#include <iostream>


Game::Game()
{
    InitGame();
}

Game::~Game() {
    Alien::UnloadImages();
}

void Game::Draw() {
    spaceship.Draw();
    for (auto& laser: spaceship.lasers) {
        laser.Draw();
    }
    for (auto& obstacle : obstacles) {
        obstacle.Draw();
    }
    for (auto& alien : aliens) {
        alien.Draw();
    }
    for (auto& laser : alienLasers) {
        laser.Draw();
    }
    mysteryship.Draw();
}

void Game::Update() {
    if (run) {
        double currentTime = GetTime();
        if (currentTime - timeLastSpawn > mysteryShipSpawnInterval) {
            mysteryship.Spawn();
            timeLastSpawn = GetTime();
            mysteryShipSpawnInterval = GetRandomValue(10, 20); 
        }

        for (auto& laser: spaceship.lasers) {
            laser.Update();
        }
        for (auto& laser : alienLasers) {
            laser.Update();
        }

        MoveAliens();
        AliensShootLaser();
        DeleteInactivelasers();
        mysteryship.Update();

        CheckForCollisions();

        if (run && aliens.empty()) {
            alienSpeedMul *= 1.2f;
            aliensDirection = 1;
            alienLasers.clear();
            timeLastAlienFired = GetTime();
            timeLastSpawn = GetTime();
            aliens = CreateAliens();
        }

    } else {
        if(IsKeyDown(KEY_R)) {
            Reset();
            InitGame();
        }
    }

}

void Game::HandleInput() {
    if (run) {
        if (IsKeyDown(KEY_LEFT)) {
            spaceship.MoveLeft();
        } else if (IsKeyDown(KEY_RIGHT)) {
            spaceship.MoveRight();
        } else if (IsKeyDown(KEY_SPACE)) {
            spaceship.FireLaser();
        }
    }
}

void Game::DeleteInactivelasers() {
    for (auto it = spaceship.lasers.begin(); it != spaceship.lasers.end();) {
        if (!it -> active) {
            it = spaceship.lasers.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = alienLasers.begin(); it != alienLasers.end();) {
        if (!it -> active) {
            it = alienLasers.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<Obstacle> Game::CreateObstacles()
{
    int obstacleWidth = Obstacle::grid[0].size() * 4;
    float gap = (GetScreenWidth() - (4 * obstacleWidth)) / 5;

    for (int i=0; i<4; i++) {
        float offset_x = (i + 1) * gap + i * obstacleWidth;
        obstacles.push_back(Obstacle({offset_x, (float)GetScreenHeight() - 200}));
    }
    return obstacles;
}

std::vector<Alien> Game::CreateAliens()
{
    std::vector<Alien> aliens;
    for (int row=0; row<5; row++) {
        for (int col = 0; col < 13; col++) {
            int alienType;
            if (row == 0) {
                alienType = 3;
            } else if (row == 1 || row == 2) {
                alienType = 2;
            } else {
                alienType = 1;
            }
            float cellSize = 60;
            float x = 80 + col * cellSize;
            float y = 120 + row * cellSize;
            aliens.push_back(Alien(alienType, {x, y}));
        }
    }
    return aliens;
}

void Game::MoveAliens() {
    for (auto& alien : aliens) {
        if (alien.position.x + alien.alienImages[alien.type-1].width > GetScreenWidth() - 25) {
            aliensDirection = -1;
            MoveDownAliens(4);
        }
        if (alien.position.x < 25) {
            aliensDirection = 1;
            MoveDownAliens(4);
        }
        alien.Update(aliensDirection, alienSpeedMul);
    }
}

void Game::MoveDownAliens(int distance) {
    for (auto& alien: aliens) {
        alien.position.y += distance;
    }
}

void Game::AliensShootLaser() {
    double currentTime = GetTime();
    if (currentTime - timeLastAlienFired >= alienLaserInterval && !aliens.empty()) {
        int randomIdx = GetRandomValue(0, aliens.size() - 1);
        Alien& alien = aliens[randomIdx];
        alienLasers.push_back(Laser({alien.position.x + alien.alienImages[alien.type-1].width/2,
                                alien.position.y + alien.alienImages[alien.type-1].height}, 6));
        timeLastAlienFired = GetTime();
    }   
}

void Game::CheckForCollisions() {
    for (auto& laser : spaceship.lasers) {
        for (auto it = aliens.begin(); it != aliens.end();) {
            if (CheckCollisionRecs(it->getRect(), laser.getRect())) {
                if (it -> type == 1) {
                    score += 100;
                } else if (it -> type == 2) {
                    score += 200;
                } else if (it -> type == 3) {
                    score += 300;
                }
                CheckHighScore();
                it = aliens.erase(it);
                laser.active = false;
            } else ++it;
        }

        for (auto& obstacle : obstacles) {
            for (auto blockIt = obstacle.blocks.begin(); blockIt != obstacle.blocks.end();) {
                if (CheckCollisionRecs(blockIt->getRect(), laser.getRect())) {
                    blockIt = obstacle.blocks.erase(blockIt);
                    laser.active = false;
                } else ++blockIt;
            }
        }

        if (CheckCollisionRecs(mysteryship.getRect(), laser.getRect())) {
            mysteryship.alive = false;
            laser.active = false;
            score += 500;
            CheckHighScore();
        }
    }


    for (auto& alienShot : alienLasers) {
        if (CheckCollisionRecs(alienShot.getRect(), spaceship.getRect())) {
            alienShot.active = false;
            lives--;
            if (lives == 0) {
                GameOver();
            }
        }
        for (auto& obstacle : obstacles) {
            for (auto blockIt = obstacle.blocks.begin(); blockIt != obstacle.blocks.end();) {
                if (CheckCollisionRecs(blockIt->getRect(), alienShot.getRect())) {
                    blockIt = obstacle.blocks.erase(blockIt);
                    alienShot.active = false;
                } else ++blockIt;
            }
        }
    }

    for (auto& alien : aliens) {
        for (auto& obstacle : obstacles) {
            auto it = obstacle.blocks.begin();
            while (it != obstacle.blocks.end()) {
                if (CheckCollisionRecs(it -> getRect(), alien.getRect())) {
                    it = obstacle.blocks.erase(it);
                } else {
                    it++;
                }
            }
        }

        if (CheckCollisionRecs(alien.getRect(), spaceship.getRect())) {
            GameOver();
        }
    }
}

void Game::GameOver() {
    run = false;
}

void Game::InitGame() {
    alienSpeedMul = 1.0;
    obstacles = CreateObstacles();
    aliens = CreateAliens();
    aliensDirection = 1;
    timeLastAlienFired = 0;
    timeLastSpawn = 0.0;
    mysteryShipSpawnInterval = GetRandomValue(10, 20);
    lives = 3;
    score = 0;
    highscore = LoadHighScore();
    run = true;
}

void Game::CheckHighScore() {
    if (score > highscore) {
        highscore = score;
        SaveHighscoreToFile(highscore);
    }
}

void Game::SaveHighscoreToFile(int highscore) {
    std::ofstream highscoreFile("src/spaceInvadersGame/highscore.txt");
    if (highscoreFile.is_open()) {
        highscoreFile << highscore;
        highscoreFile.close();
    } else {
        std::cerr << "Failed to save highscore to file" << std::endl;
    }
}

int Game::LoadHighScore() {
    int loadedHighscore = 0;
    std::ifstream highscoreFile("src/spaceInvadersGame/highscore.txt");
    if (highscoreFile.is_open()) {
        highscoreFile >> loadedHighscore;
        highscoreFile.close();
    } else {
        std::cerr << "Failed to load highscore from file" << std::endl;
    }
    return loadedHighscore;
}

void Game::Reset() {
    spaceship.Reset();
    aliens.clear();
    alienLasers.clear();
    obstacles.clear();
}