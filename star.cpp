#include "star.h"
#include <GL/glut.h>
#include <cstdlib>

Star::Star(float x, float y, float speed, float size) {
    this->x = x;
    this->y = y;
    this->speed = speed;
    this->size = size;
}

void Star::update() {
    y -= speed;
    if (y < -1) {
        y = 1;
        x = (rand() % 200 - 100) / 100.0f;
    }
}

void Star::draw() {
    glPointSize(size);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}