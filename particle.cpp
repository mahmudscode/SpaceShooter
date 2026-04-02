#include "particle.h"
#include <GL/glut.h>

Particle::Particle(float x, float y, float vx, float vy) {
    this->x = x;
    this->y = y;
    this->vx = vx;
    this->vy = vy;
    life = 1.0f;
}

void Particle::update() {
    x += vx;
    y += vy;
    life -= 0.03;
}

void Particle::draw() {
    if (life <= 0) return;

    glColor3f(1, life, 0);
    glPointSize(3);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}