#include "enemy.h"
#include <GL/glut.h>
#include <cmath>

Enemy::Enemy(float startX) {
    x = startX;
    y = 1;
}

void Enemy::update(float playerX) {
    y -= 0.02;

    // Wave motion
    x += sin(y * 5) * 0.005;

    // Follow player slightly
    if (x < playerX) x += 0.002;
    else x -= 0.002;
}

void Enemy::draw() {
    glColor3f(1, 0, 1);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++) {
        float a = i * 2 * 3.1416 / 20;
        glVertex2f(x + cos(a)*0.05, y + sin(a)*0.03);
    }
    glEnd();
}