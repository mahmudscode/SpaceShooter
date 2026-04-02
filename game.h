#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "particle.h"
#include "star.h"
#include <vector>

enum GameState { MENU, PLAYING, GAMEOVER };

class Game {
public:
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;
    std::vector<Star> stars;

    int score, level, lives;
    GameState state;

    Game();
    void update();
    void draw();
    void shoot();
};

#endif