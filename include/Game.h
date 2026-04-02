#pragma once
#include <vector>
#include <string>
#include <random>

struct Vec2 { float x, y; };
struct Color { float r, g, b, a; };

struct Star {
    float x, y, radius, speed, opacity;
    int layer; // 0=slow, 1=mid, 2=fast
};

struct Bullet {
    float x, y;
    float vy = -9.0f;
    bool active = true;
};

struct Enemy {
    float x, y;
    float vx, vy;
    float phase;
    int hp = 1;
    bool active = true;
};

struct Particle {
    float x, y;
    float vx, vy;
    float life; // 0..1
    Color color;
    float radius;
};

enum class GameState { MENU, PLAYING, GAMEOVER };

struct Player {
    float x, y;
    float speed = 4.0f;
    float thrustAnim = 0.0f;
    int width = 30, height = 36;
};

struct GameData {
    GameState state = GameState::MENU;
    int score = 0;
    int level = 1;
    int lives = 3;
    int frameCount = 0;
    int shootCooldown = 0;

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;
    std::vector<Star> stars;

    bool keyLeft = false, keyRight = false, keySpace = false;
    bool keyA = false, keyD = false;

    std::mt19937 rng;

    GameData() : rng(std::random_device{}()) {}

    float randF(float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
    }
};
