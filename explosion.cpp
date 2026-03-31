#include "explosion.h"
#include <GL/glut.h>
#include <cmath>

Explosion::Explosion(float x, float y) {
    this->x = x;
    this->y = y;
    size = 0.02;
    active = true;
}

void Explosion::update() {
    size += 0.02;
    if (size > 0.15) active = false;
}

void Explosion::draw() {
    if (!active) return;

    glColor3f(1, 0.5, 0);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++) {
        float a = i * 2 * 3.1416 / 20;
        glVertex2f(x + cos(a) * size, y + sin(a) * size);
    }
    glEnd();
}
