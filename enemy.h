#ifndef ENEMY_H
#define ENEMY_H

class Enemy {
public:
    float x, y;

    Enemy(float startX);
    void update(float playerX);
    void draw();
};

#endif