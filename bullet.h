#ifndef BULLET_H
#define BULLET_H

class Bullet {
public:
    float x, y;

    Bullet(float startX, float startY);
    void update();
    void draw();
};

#endif