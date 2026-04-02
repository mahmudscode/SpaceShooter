#include "game.h"
#include "utils.h"
#include <cstdlib>
#include <cmath>

Game::Game() {
    score = 0;
    level = 1;
    lives = 3;
    state = MENU;

    // create stars
    for (int i = 0; i < 100; i++) {
        stars.push_back(Star(
            (rand()%200-100)/100.0f,
            (rand()%200-100)/100.0f,
            0.002 + (rand()%100)/50000.0f,
            1 + rand()%3
        ));
    }
}

void Game::shoot() {
    bullets.push_back(Bullet(player.x, -0.8));
}

void Game::update() {
    if (state != PLAYING) return;

    for (auto &s : stars) s.update();
    for (auto &b : bullets) b.update();
    for (auto &e : enemies) e.update(player.x);
    for (auto &p : particles) p.update();

    // spawn enemy
    if (rand()%50 == 0) {
        enemies.push_back(Enemy((rand()%200-100)/100.0f));
    }

    // collision
    for (int i=0;i<bullets.size();i++){
        for (int j=0;j<enemies.size();j++){
            if (fabs(bullets[i].x-enemies[j].x)<0.05 &&
                fabs(bullets[i].y-enemies[j].y)<0.05){

                // explosion
                for(int k=0;k<20;k++){
                    float a = (rand()%360)*3.14/180;
                    particles.push_back(Particle(
                        enemies[j].x,enemies[j].y,
                        cos(a)*0.02,sin(a)*0.02
                    ));
                }

                bullets.erase(bullets.begin()+i);
                enemies.erase(enemies.begin()+j);
                score+=10;

                if(score%100==0) level++;

                break;
            }
        }
    }

    // enemy hit player
    for(auto &e:enemies){
        if (fabs(e.x-player.x)<0.1 && e.y<-0.8){
            lives--;
            if(lives<=0) state=GAMEOVER;
        }
    }
}

void Game::draw() {

    // stars
    for (auto &s : stars) s.draw();

    if (state == MENU) {
        drawText(-0.3, 0.2, "SPACE SHOOTER");
        drawText(-0.4, 0.0, "Press ENTER");
        return;
    }

    if (state == GAMEOVER) {
        drawText(-0.2, 0.1, "GAME OVER");
        drawText(-0.3, -0.1, "Press R");
        return;
    }

    player.draw();

    for (auto &b : bullets) b.draw();
    for (auto &e : enemies) e.draw();
    for (auto &p : particles) p.draw();

    drawText(-0.9, 0.9, "Score: "+std::to_string(score));
    drawText(0.4, 0.9, "Lvl: "+std::to_string(level));
    drawText(0.0, 0.9, "Life: "+std::to_string(lives));
}