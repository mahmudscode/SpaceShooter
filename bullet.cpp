#include "bullet.h"
#include <GL/glut.h>

Bullet::Bullet(float x, float y) {
    this->x = x;
    this->y = y;
}

void Bullet::update() { y += 0.05; }

void Bullet::draw() {
    glColor3f(1, 1, 0);
    glRectf(x - 0.01, y, x + 0.01, y + 0.05);
}