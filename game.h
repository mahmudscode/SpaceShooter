#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "explosion.h"
#include <vector>

enum GameState { MENU, PLAYING, GAMEOVER };

class Game {
public:
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Explosion> explosions;

    int score;
    GameState state;

    Game();

    void update();
    void draw();
    void shoot();
};

#endif