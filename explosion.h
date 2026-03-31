#ifndef EXPLOSION_H
#define EXPLOSION_H

class Explosion {
public:
    float x, y;
    float size;
    bool active;

    Explosion(float x, float y);
    void update();
    void draw();
};

#endif