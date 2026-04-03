// =============================================================================
//  SpaceShooter — Core Data Architecture
// =============================================================================
//
//  AUTHOR         : Mitu
//                   [ Data Structures & Engine Architecture ]
//
//  Prime Author   : Mahmudur Rahman  (Renderer.cpp)
//  Contributors   : Era (GameLogic.cpp) · Tripty (main.cpp) · PK (Build)
// =============================================================================
#pragma once
#include <vector>
#include <string>
#include <random>

// ── Difficulty mode ───────────────────────────────────────────────────────────
enum class Difficulty
{
    EASY,
    MEDIUM,
    HARD
};

// Per-difficulty base parameters (applied on top of per-level scaling)
struct DiffConfig
{
    float enemySpeedMul; // base enemy vy multiplier
    float enemySideMul;  // base enemy vx multiplier
    float spawnRateBase; // base frames between spawns (lower = faster)
    float bulletSpeed;   // player bullet speed (pixels/frame)
    int shootCooldown;   // frames between shots
    int startLives;      // lives at game start
    int enemyHPBase;     // base HP for enemies (scales with level)
    int scorePerKill;    // points per kill
};

// ── Level progression (10 levels per difficulty) ────────────────────────────
// Each level multiplies the difficulty config values further.
// Level 1 = easiest, Level 10 = hardest within the chosen difficulty.
struct LevelConfig
{
    float enemySpeedBonus; // added to enemy vy each level
    float spawnRateMul;    // multiply spawnRate (< 1 = faster spawning)
    int maxEnemies;        // cap on simultaneous enemies on screen
    int enemyHPBonus;      // extra HP added each level
    bool enemyZigzag;      // enemies zigzag horizontally
    bool enemyDive;        // enemies occasionally dive at player
    int bossEveryN;        // spawn a boss every N kills (0 = no boss)
};

// ── Primitive types ───────────────────────────────────────────────────────────
struct Vec2
{
    float x, y;
};
struct Color
{
    float r, g, b, a;
};

struct Star
{
    float x, y, radius, speed, opacity;
    int layer;
};

struct Bullet
{
    float x, y;
    float vx = 0.0f;  // horizontal velocity (used by aimed enemy bullets)
    float vy = -9.0f; // vertical velocity (negative = upward for player)
    bool active = true;
    bool isEnemy = false;
};

struct Enemy
{
    float x, y;
    float vx, vy;
    float phase;
    float zigzagTimer = 0;
    float diveTimer = 0;
    float shootTimer = 0; // frames until this enemy fires next
    int hp = 1;
    int maxHp = 1;
    bool active = true;
    bool isBoss = false;
};

struct Particle
{
    float x, y, vx, vy;
    float life;
    Color color;
    float radius;
};

// ── Game state machine ────────────────────────────────────────────────────────
enum class GameState
{
    MENU,        // difficulty + start screen
    DIFF_SELECT, // choosing Easy / Medium / Hard
    PLAYING,
    LEVEL_COMPLETE, // brief flash between levels
    GAMEOVER,
    VICTORY // beat all 10 levels
};

struct Player
{
    float x, y;
    float speed = 4.0f;
    float thrustAnim = 0.0f;
    int width = 30, height = 36;
    float invincible = 0.0f; // frames of invincibility after being hit
};

// ── Master game data ──────────────────────────────────────────────────────────
struct GameData
{
    GameState state = GameState::MENU;
    Difficulty difficulty = Difficulty::EASY;

    int score = 0;
    int level = 1; // 1–10
    int lives = 3;
    int killCount = 0;    // kills this level
    int killsNeeded = 10; // kills to complete this level (scales)
    int frameCount = 0;
    int shootCooldown = 0;
    int levelFlashTimer = 0; // countdown for LEVEL_COMPLETE display

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;
    std::vector<Star> stars;

    // Input
    bool keyLeft = false, keyRight = false, keySpace = false;
    bool keyUp = false, keyDown = false;
    bool keyA = false, keyD = false;
    bool keyW = false, keyS = false;

    std::mt19937 rng;
    GameData() : rng(std::random_device{}()) {}

    float randF(float lo, float hi)
    {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
    }

    // ── Static difficulty table ───────────────────────────────────────────
    static DiffConfig getDiffConfig(Difficulty d)
    {
        switch (d)
        {
        case Difficulty::EASY:
            return {0.7f, 0.5f, 90, 10.0f, 14, 5, 1, 10};
        case Difficulty::MEDIUM:
            return {1.0f, 0.8f, 65, 10.0f, 12, 3, 1, 15};
        case Difficulty::HARD:
            return {1.4f, 1.2f, 45, 11.0f, 10, 2, 2, 20};
        }
        return {1.0f, 0.8f, 65, 10.0f, 12, 3, 1, 15};
    }

    // ── Per-level scaling (1-indexed) ─────────────────────────────────────
    // Returns config for the given level (1–10)
    static LevelConfig getLevelConfig(int lv, Difficulty d)
    {
        // lv clamped 1..10
        if (lv < 1)
            lv = 1;
        if (lv > 10)
            lv = 10;

        // Base progression table (difficulty-agnostic scaling)
        // Each entry: {speedBonus, spawnMul, maxEnemies, hpBonus, zigzag, dive, bossEveryN}
        static const LevelConfig table[10] = {
            {0.00f, 1.00f, 4, 0, false, false, 0}, // Lv 1
            {0.05f, 0.95f, 5, 0, false, false, 0}, // Lv 2
            {0.10f, 0.88f, 6, 0, false, false, 0}, // Lv 3
            {0.15f, 0.80f, 7, 0, true, false, 0},  // Lv 4
            {0.20f, 0.72f, 8, 1, true, false, 20}, // Lv 5  (boss)
            {0.28f, 0.64f, 9, 1, true, true, 0},   // Lv 6
            {0.36f, 0.56f, 10, 2, true, true, 0},  // Lv 7
            {0.45f, 0.48f, 12, 2, true, true, 15}, // Lv 8  (boss)
            {0.55f, 0.40f, 14, 3, true, true, 0},  // Lv 9
            {0.70f, 0.30f, 16, 4, true, true, 10}, // Lv 10 (boss)
        };

        LevelConfig lc = table[lv - 1];

        // Hard difficulty: tighten further
        if (d == Difficulty::HARD)
        {
            lc.maxEnemies += 3;
            lc.enemyHPBonus += 1;
            lc.spawnRateMul *= 0.85f;
            if (lc.bossEveryN > 0)
                lc.bossEveryN -= 3;
        }
        else if (d == Difficulty::EASY)
        {
            lc.maxEnemies = std::max(3, lc.maxEnemies - 2);
            lc.enemyHPBonus = std::max(0, lc.enemyHPBonus - 1);
            lc.spawnRateMul *= 1.15f;
            lc.bossEveryN = 0; // no bosses on easy
        }
        return lc;
    }

    // Kills needed to complete a level (scales with level & difficulty)
    static int killsForLevel(int lv, Difficulty d)
    {
        int base = 8 + lv * 3; // 11 at lv1 … 38 at lv10
        if (d == Difficulty::MEDIUM)
            base = int(base * 1.3f);
        if (d == Difficulty::HARD)
            base = int(base * 1.7f);
        return base;
    }
};
