#ifndef PARTICLE_H
#define PARTICLE_H

class Particle {
public:
    float x, y, vx, vy, life;

    Particle(float x, float y, float vx, float vy);
    void update();
    void draw();
};

#endif