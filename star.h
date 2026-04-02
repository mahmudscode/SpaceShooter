#ifndef STAR_H
#define STAR_H

class Star {
public:
    float x, y, speed, size;

    Star(float x, float y, float speed, float size);
    void update();
    void draw();
};

#endif