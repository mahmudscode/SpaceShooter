#include "player.h"
#include <GL/glut.h>

Player::Player() { x = 0; }

void Player::moveLeft() { if (x > -0.9) x -= 0.08; }
void Player::moveRight() { if (x < 0.9) x += 0.08; }

void Player::draw() {
    glColor3f(0, 1, 1);

    // Main ship
    glBegin(GL_TRIANGLES);
    glVertex2f(x, -0.75);
    glVertex2f(x - 0.05, -0.9);
    glVertex2f(x + 0.05, -0.9);
    glEnd();

    // Wings
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.05, -0.85);
    glVertex2f(x - 0.1, -0.9);
    glVertex2f(x - 0.02, -0.9);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(x + 0.05, -0.85);
    glVertex2f(x + 0.1, -0.9);
    glVertex2f(x + 0.02, -0.9);
    glEnd();
}