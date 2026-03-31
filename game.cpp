#include "game.h"
#include "utils.h"
#include "audio.h"
#include <cstdlib>
#include <cmath>

Game::Game() {
    score = 0;
    state = MENU;
}

void Game::shoot() {
    bullets.push_back(Bullet(player.x, -0.8));
}

void Game::update() {
    if (state != PLAYING) return;

    for (auto &b : bullets) b.update();
    for (auto &e : enemies) e.update(player.x);
    for (auto &ex : explosions) ex.update();

    // Collision
    for (int i = 0; i < bullets.size(); i++) {
        for (int j = 0; j < enemies.size(); j++) {
            if (fabs(bullets[i].x - enemies[j].x) < 0.05 &&
                fabs(bullets[i].y - enemies[j].y) < 0.05) {

                explosions.push_back(Explosion(enemies[j].x, enemies[j].y));
                explosionSound();

                bullets.erase(bullets.begin() + i);
                enemies.erase(enemies.begin() + j);
                score++;
                break;
            }
        }
    }

    // Game Over
    for (auto &e : enemies) {
        if (e.y < -0.8 && fabs(e.x - player.x) < 0.1) {
            state = GAMEOVER;
        }
    }

    // Spawn enemies
    if (rand() % 50 == 0) {
        float x = (rand() % 200 - 100) / 100.0f;
        enemies.push_back(Enemy(x));
    }
}

void Game::draw() {

    if (state == MENU) {
        drawText(-0.3, 0.2, "SPACE SHOOTER");
        drawText(-0.3, 0.0, "Press ENTER");
        return;
    }

    if (state == GAMEOVER) {
        drawText(-0.2, 0.1, "GAME OVER");
        drawText(-0.3, -0.1, "Press R to Restart");
        return;
    }

    player.draw();

    for (auto &b : bullets) b.draw();
    for (auto &e : enemies) e.draw();
    for (auto &ex : explosions) ex.draw();

    drawText(-0.9, 0.9, "Score: " + std::to_string(score));
}