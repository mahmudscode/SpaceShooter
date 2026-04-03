

#include "Game.h"
#include <cmath>
#include <algorithm>

static const float W = 420.0f;
static const float H = 520.0f;

// ── Star field ────────────────────────────────────────────────────────────────
void resetStars(GameData &g)
{
    g.stars.clear();
    auto mkLayer = [&](int n, float r, float spd, int layer)
    {
        for (int i = 0; i < n; i++)
        {
            Star s;
            s.x = g.randF(0, W);
            s.y = g.randF(0, H);
            s.radius = r;
            s.speed = spd;
            s.opacity = g.randF(0.2f, 0.8f);
            s.layer = layer;
            g.stars.push_back(s);
        }
    };
    mkLayer(80, 0.6f, 0.3f, 0);
    mkLayer(40, 1.1f, 0.6f, 1);
    mkLayer(16, 1.8f, 1.1f, 2);
}

// ── Helpers ───────────────────────────────────────────────────────────────────
static DiffConfig dc(const GameData &g) { return GameData::getDiffConfig(g.difficulty); }
static LevelConfig lc(const GameData &g) { return GameData::getLevelConfig(g.level, g.difficulty); }

// ── Start / restart ───────────────────────────────────────────────────────────
void startGame(GameData &g)
{
    g.score = 0;
    g.level = 1;
    g.lives = GameData::getDiffConfig(g.difficulty).startLives;
    g.killCount = 0;
    g.killsNeeded = GameData::killsForLevel(1, g.difficulty);
    g.frameCount = 0;
    g.shootCooldown = 0;
    g.levelFlashTimer = 0;
    g.bullets.clear();
    g.enemies.clear();
    g.particles.clear();
    g.player.x = W / 2.0f;
    g.player.y = H - 60.0f;
    g.player.invincible = 0;
    resetStars(g);
    g.state = GameState::PLAYING;
}

// Advance to the next level
static void advanceLevel(GameData &g)
{
    g.level++;
    if (g.level > 10)
    {
        g.state = GameState::VICTORY;
        return;
    }
    g.killCount = 0;
    g.killsNeeded = GameData::killsForLevel(g.level, g.difficulty);
    g.enemies.clear();
    g.bullets.clear();
    g.frameCount = 0;
    g.state = GameState::LEVEL_COMPLETE;
    g.levelFlashTimer = 120; // 2 seconds at 60fps
}

// ── Explosion ─────────────────────────────────────────────────────────────────
void spawnExplosion(GameData &g, float x, float y, bool big = false)
{
    static const Color colors[] = {
        {1.0f, 0.4f, 0.0f, 1.0f}, {1.0f, 0.8f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.27f, 1.0f}, {0.8f, 0.27f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};
    int count = big ? 36 : 18;
    for (int i = 0; i < count; i++)
    {
        float angle = g.randF(0, 6.283f);
        float spd = g.randF(1.0f, big ? 6.0f : 4.0f);
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = std::cos(angle) * spd;
        p.vy = std::sin(angle) * spd;
        p.life = 1.0f;
        p.color = colors[i % 5];
        p.radius = g.randF(1.5f, big ? 6.0f : 4.0f);
        g.particles.push_back(p);
    }
}

// ── Enemy spawn ───────────────────────────────────────────────────────────────
void spawnEnemy(GameData &g, bool boss = false)
{
    auto d = dc(g);
    auto l = lc(g);

    Enemy e;
    e.x = g.randF(30, W - 30);
    e.y = -20.0f;
    e.vx = (g.randF(0, 1) - 0.5f) * d.enemySideMul * (1.0f + (g.level - 1) * 0.08f);
    e.vy = (d.enemySpeedMul + l.enemySpeedBonus) * (0.8f + g.randF(0, 0.3f));
    e.phase = g.randF(0, 6.283f);
    e.hp = d.enemyHPBase + l.enemyHPBonus + (boss ? 8 + g.level * 2 : 0);
    e.maxHp = e.hp;
    e.active = true;
    e.isBoss = boss;
    e.zigzagTimer = 0;
    e.diveTimer = l.enemyDive ? g.randF(60, 180) : 99999;
    g.enemies.push_back(e);
}

// ── Shoot (player) ────────────────────────────────────────────────────────────
void shoot(GameData &g)
{
    if (g.shootCooldown > 0)
        return;
    auto d = dc(g);
    float by = d.bulletSpeed;

    // Extra barrels on higher levels
    int barrels = 2;
    if (g.level >= 6 && g.difficulty == Difficulty::HARD)
        barrels = 3;
    if (g.level >= 8)
        barrels = (g.difficulty == Difficulty::EASY) ? 2 : 3;

    float offsets[] = {-11.0f, 11.0f, 0.0f};
    for (int i = 0; i < barrels; i++)
    {
        Bullet b;
        b.x = g.player.x + offsets[i];
        b.y = g.player.y - 14.0f;
        b.vy = -by;
        b.isEnemy = false;
        b.active = true;
        g.bullets.push_back(b);
    }
    g.shootCooldown = d.shootCooldown;
}

// ── Main update ───────────────────────────────────────────────────────────────
void updateGame(GameData &g)
{
    if (g.state == GameState::LEVEL_COMPLETE)
    {
        g.levelFlashTimer--;
        // Still scroll stars during level flash
        for (auto &s : g.stars)
        {
            s.y += s.speed;
            if (s.y > H)
            {
                s.y = 0;
                s.x = g.randF(0, W);
            }
        }
        if (g.levelFlashTimer <= 0)
            g.state = GameState::PLAYING;
        return;
    }

    if (g.state != GameState::PLAYING)
        return;

    g.frameCount++;
    if (g.shootCooldown > 0)
        g.shootCooldown--;
    if (g.player.invincible > 0)
        g.player.invincible--;

    auto d = dc(g);
    auto l = lc(g);

    // ── Player movement ───────────────────────────────────────────────────
    if ((g.keyLeft || g.keyA) && g.player.x > 22)
        g.player.x -= g.player.speed;
    if ((g.keyRight || g.keyD) && g.player.x < W - 22)
        g.player.x += g.player.speed;
    if (g.keySpace)
        shoot(g);

    g.player.thrustAnim += 0.18f;
    if (g.player.thrustAnim > 6.2832f)
        g.player.thrustAnim -= 6.2832f;

    // ── Bullets (player) ──────────────────────────────────────────────────
    for (auto &b : g.bullets)
    {
        if (b.isEnemy)
        {
            b.y -= b.vy;
        } // vy negative for player, positive for enemy
        else
        {
            b.y += b.vy;
        } // b.vy is already negative for player
        if (b.y < -10 || b.y > H + 10)
            b.active = false;
    }

    // ── Enemy spawn ───────────────────────────────────────────────────────
    int spawnRate = std::max(15, (int)(d.spawnRateBase * l.spawnRateMul));
    int onScreen = (int)g.enemies.size();
    if (g.frameCount % spawnRate == 0 && onScreen < l.maxEnemies)
    {
        // Boss check
        bool boss = (l.bossEveryN > 0 && g.killCount > 0 && g.killCount % l.bossEveryN == 0 && g.enemies.empty()); // spawn boss when screen is clear
        spawnEnemy(g, boss);
    }

    // ── Enemy update ──────────────────────────────────────────────────────
    for (auto &e : g.enemies)
    {
        // Zigzag
        if (l.enemyZigzag)
        {
            e.zigzagTimer += 0.05f;
            e.vx = std::sin(e.zigzagTimer + e.phase) * d.enemySideMul * (1.0f + (g.level - 1) * 0.08f) * (e.isBoss ? 1.5f : 1.0f);
        }
        else
        {
            if (e.x < 18 || e.x > W - 18)
                e.vx *= -1.0f;
        }

        // Dive at player
        if (l.enemyDive && !e.isBoss)
        {
            e.diveTimer--;
            if (e.diveTimer <= 0)
            {
                float dx = g.player.x - e.x;
                e.vx = (dx > 0 ? 1.0f : -1.0f) * d.enemySideMul * 2.5f;
                e.vy += 0.3f;
                e.diveTimer = g.randF(90, 200);
            }
        }

        e.x += e.vx;
        e.y += e.vy;

        // Clamp x
        if (e.x < 18)
        {
            e.x = 18;
            e.vx = std::abs(e.vx);
        }
        if (e.x > W - 18)
        {
            e.x = W - 18;
            e.vx = -std::abs(e.vx);
        }

        // Escaped past bottom
        if (e.y > H + 30)
        {
            e.active = false;
            if (!e.isBoss)
                g.lives--;
        }

        // Bosses fire bullets on hard/medium level 7+
        if (e.isBoss && g.frameCount % 60 == 0 && (g.difficulty != Difficulty::EASY) && g.level >= 7)
        {
            Bullet eb;
            eb.x = e.x;
            eb.y = e.y + 10;
            eb.vy = -2.5f; // positive = downward (rendered via isEnemy flag)
            eb.isEnemy = true;
            eb.active = true;
            g.bullets.push_back(eb);
        }
    }

    // ── Bullet ↔ Enemy collision ──────────────────────────────────────────
    for (auto &b : g.bullets)
    {
        if (!b.active || b.isEnemy)
            continue;
        for (auto &e : g.enemies)
        {
            if (!e.active)
                continue;
            float hitW = e.isBoss ? 28.0f : 20.0f;
            float hitH = e.isBoss ? 20.0f : 14.0f;
            if (std::abs(b.x - e.x) < hitW && std::abs(b.y - e.y) < hitH)
            {
                b.active = false;
                e.hp--;
                if (e.hp <= 0)
                {
                    spawnExplosion(g, e.x, e.y, e.isBoss);
                    g.score += d.scorePerKill * (e.isBoss ? 5 : 1);
                    e.active = false;
                    g.killCount++;
                }
                break;
            }
        }
    }

    // ── Enemy bullet ↔ Player collision ──────────────────────────────────
    if (g.player.invincible <= 0)
    {
        for (auto &b : g.bullets)
        {
            if (!b.active || !b.isEnemy)
                continue;
            if (std::abs(b.x - g.player.x) < 16 && std::abs(b.y - g.player.y) < 18)
            {
                b.active = false;
                g.lives--;
                g.player.invincible = 90; // 1.5s invincible
                spawnExplosion(g, g.player.x, g.player.y);
            }
        }
    }

    // ── Enemy ↔ Player collision ──────────────────────────────────────────
    if (g.player.invincible <= 0)
    {
        for (auto &e : g.enemies)
        {
            if (!e.active)
                continue;
            if (std::abs(e.x - g.player.x) < 24 && std::abs(e.y - g.player.y) < 22)
            {
                spawnExplosion(g, e.x, e.y, e.isBoss);
                g.lives--;
                e.active = false;
                g.player.invincible = 90;
            }
        }
    }

    // ── Cleanup dead objects ──────────────────────────────────────────────
    g.enemies.erase(std::remove_if(g.enemies.begin(), g.enemies.end(),
                                   [](const Enemy &e)
                                   { return !e.active; }),
                    g.enemies.end());
    g.bullets.erase(std::remove_if(g.bullets.begin(), g.bullets.end(),
                                   [](const Bullet &b)
                                   { return !b.active; }),
                    g.bullets.end());

    // ── Particles ─────────────────────────────────────────────────────────
    for (auto &p : g.particles)
    {
        p.x += p.vx;
        p.y += p.vy;
        p.life -= 0.035f;
    }
    g.particles.erase(std::remove_if(g.particles.begin(), g.particles.end(),
                                     [](const Particle &p)
                                     { return p.life <= 0; }),
                      g.particles.end());

    // ── Stars (scroll faster at higher levels) ────────────────────────────
    float starSpeed = 1.0f + (g.level - 1) * 0.12f;
    for (auto &s : g.stars)
    {
        s.y += s.speed * starSpeed;
        if (s.y > H)
        {
            s.y = 0;
            s.x = g.randF(0, W);
        }
    }

    // ── Check level complete ──────────────────────────────────────────────
    if (g.killCount >= g.killsNeeded)
        advanceLevel(g);

    // ── Check game over ───────────────────────────────────────────────────
    if (g.lives <= 0)
        g.state = GameState::GAMEOVER;
}

void updateMenuStars(GameData &g)
{
    for (auto &s : g.stars)
    {
        s.y += s.speed * 0.3f;
        if (s.y > H)
        {
            s.y = 0;
            s.x = g.randF(0, W);
        }
    }
}
