#include "Game.h"
#include <cmath>
#include <algorithm>

static const float W = 420.0f;
static const float H = 520.0f;

void resetStars(GameData& g) {
    g.stars.clear();
    // Layer 0: 80 tiny slow stars
    for (int i = 0; i < 80; i++) {
        Star s;
        s.x = g.randF(0, W);
        s.y = g.randF(0, H);
        s.radius = 0.6f;
        s.speed = 0.3f;
        s.opacity = g.randF(0.2f, 0.8f);
        s.layer = 0;
        g.stars.push_back(s);
    }
    // Layer 1: 40 medium stars
    for (int i = 0; i < 40; i++) {
        Star s;
        s.x = g.randF(0, W);
        s.y = g.randF(0, H);
        s.radius = 1.1f;
        s.speed = 0.6f;
        s.opacity = g.randF(0.2f, 0.8f);
        s.layer = 1;
        g.stars.push_back(s);
    }
    // Layer 2: 16 large bright stars
    for (int i = 0; i < 16; i++) {
        Star s;
        s.x = g.randF(0, W);
        s.y = g.randF(0, H);
        s.radius = 1.8f;
        s.speed = 1.1f;
        s.opacity = g.randF(0.2f, 0.8f);
        s.layer = 2;
        g.stars.push_back(s);
    }
}

void startGame(GameData& g) {
    g.score = 0;
    g.level = 1;
    g.lives = 3;
    g.frameCount = 0;
    g.shootCooldown = 0;
    g.bullets.clear();
    g.enemies.clear();
    g.particles.clear();
    g.player.x = W / 2.0f;
    g.player.y = H - 60.0f;
    resetStars(g);
    g.state = GameState::PLAYING;
}

void spawnExplosion(GameData& g, float x, float y) {
    std::vector<Color> colors = {
        {1.0f, 0.4f, 0.0f, 1.0f},
        {1.0f, 0.8f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.27f, 1.0f},
        {0.8f, 0.27f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    for (int i = 0; i < 18; i++) {
        float angle = g.randF(0, 6.283f);
        float spd = g.randF(1.0f, 4.0f);
        Particle p;
        p.x = x; p.y = y;
        p.vx = std::cos(angle) * spd;
        p.vy = std::sin(angle) * spd;
        p.life = 1.0f;
        p.color = colors[i % colors.size()];
        p.radius = g.randF(1.5f, 4.0f);
        g.particles.push_back(p);
    }
}

void spawnEnemy(GameData& g) {
    Enemy e;
    e.x = g.randF(30, W - 30);
    e.y = -20.0f;
    e.vx = (g.randF(0, 1) - 0.5f) * 0.8f;
    e.vy = 0.8f + g.level * 0.15f;
    e.phase = g.randF(0, 6.283f);
    e.hp = 1;
    e.active = true;
    g.enemies.push_back(e);
}

void shoot(GameData& g) {
    if (g.shootCooldown > 0) return;
    Bullet b1, b2;
    b1.x = g.player.x - 11.0f; b1.y = g.player.y - 14.0f;
    b2.x = g.player.x + 11.0f; b2.y = g.player.y - 14.0f;
    g.bullets.push_back(b1);
    g.bullets.push_back(b2);
    g.shootCooldown = 12;
}

void updateGame(GameData& g) {
    if (g.state != GameState::PLAYING) return;

    g.frameCount++;
    if (g.shootCooldown > 0) g.shootCooldown--;

    // Player movement
    float spd = g.player.speed;
    if ((g.keyLeft || g.keyA) && g.player.x > 22) g.player.x -= spd;
    if ((g.keyRight || g.keyD) && g.player.x < W - 22) g.player.x += spd;
    if (g.keySpace) shoot(g);

    g.player.thrustAnim += 0.18f;
    if (g.player.thrustAnim > 6.2832f) g.player.thrustAnim -= 6.2832f;

    // Bullets
    for (auto& b : g.bullets) {
        b.y -= 9.0f;
        if (b.y < -10.0f) b.active = false;
    }
    g.bullets.erase(std::remove_if(g.bullets.begin(), g.bullets.end(),
        [](const Bullet& b){ return !b.active; }), g.bullets.end());

    // Spawn enemies
    int spawnRate = std::max(40, 80 - g.level * 8);
    if (g.frameCount % spawnRate == 0) spawnEnemy(g);

    // Update enemies
    for (auto& e : g.enemies) {
        e.x += e.vx;
        e.y += e.vy;
        if (e.x < 18 || e.x > W - 18) e.vx *= -1.0f;
        if (e.y > H + 30) {
            e.active = false;
            g.lives--;
        }
    }

    // Bullet-enemy collision
    for (auto& b : g.bullets) {
        if (!b.active) continue;
        for (auto& e : g.enemies) {
            if (!e.active) continue;
            if (std::abs(b.x - e.x) < 20 && std::abs(b.y - e.y) < 14) {
                spawnExplosion(g, e.x, e.y);
                g.score += 10;
                b.active = false;
                e.active = false;
                if (g.score > 0 && g.score % 100 == 0)
                    g.level = std::min(8, g.level + 1);
                break;
            }
        }
    }

    // Enemy-player collision
    for (auto& e : g.enemies) {
        if (!e.active) continue;
        if (std::abs(e.x - g.player.x) < 24 && std::abs(e.y - g.player.y) < 22) {
            spawnExplosion(g, e.x, e.y);
            g.lives--;
            e.active = false;
        }
    }

    // Remove dead enemies and bullets
    g.enemies.erase(std::remove_if(g.enemies.begin(), g.enemies.end(),
        [](const Enemy& e){ return !e.active; }), g.enemies.end());
    g.bullets.erase(std::remove_if(g.bullets.begin(), g.bullets.end(),
        [](const Bullet& b){ return !b.active; }), g.bullets.end());

    // Particles
    for (auto& p : g.particles) {
        p.x += p.vx; p.y += p.vy;
        p.life -= 0.035f;
    }
    g.particles.erase(std::remove_if(g.particles.begin(), g.particles.end(),
        [](const Particle& p){ return p.life <= 0; }), g.particles.end());

    // Stars
    float scroll = 1.0f;
    for (auto& s : g.stars) {
        s.y += s.speed * scroll;
        if (s.y > H) { s.y = 0; s.x = g.randF(0, W); }
    }

    if (g.lives <= 0) g.state = GameState::GAMEOVER;
}

void updateMenuStars(GameData& g) {
    for (auto& s : g.stars) {
        s.y += s.speed * 0.3f;
        if (s.y > H) { s.y = 0; s.x = g.randF(0, W); }
    }
}
