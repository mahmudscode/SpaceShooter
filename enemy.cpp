#include "enemy.h"
#include <GL/glut.h>

Enemy::Enemy(float startX) {
    x = startX;
    y = 1;
}

void Enemy::update(float playerX) {
    y -= 0.02;
    if (x < playerX) x += 0.005;
    else x -= 0.005;
}

void Enemy::draw() {
    glColor3f(1, 0, 0);
    glRectf(x - 0.05, y, x + 0.05, y + 0.05);
}