#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    float x;

    Player();
    void moveLeft();
    void moveRight();
    void draw();
};

#endif