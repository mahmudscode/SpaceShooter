// =============================================================================
//  SpaceShooter — OpenGL Renderer
// =============================================================================
//
//  PRIMARY AUTHOR : Mahmudur Rahman
//                   [ Core Graphics Engine, OpenGL Pipeline ]
//                   Responsible for all visual rendering:
//                     - Background, nebula & parallax star field
//                     - Player ship geometry (hull, wings, cockpit, thrust)
//                     - Alien saucer (dome, rotating lights, tractor beam)
//                     - Bullet glow trails (additive blending)
//                     - Explosion particle system
//                     - HUD (score, level, lives / hearts)
//                     - Menu & Game Over overlay screens
//                     - Pixel-art geometric font (no external library)
//                     - Letterbox viewport scaling
//
//  Project        : Space Shooter C++/OpenGL
//  File           : src/Renderer.cpp
//  Description    : Full OpenGL 2.1 fixed-pipeline renderer.
//                   No shaders, no textures — pure geometry & blending.
//
//  Contributors   : Era       (GameLogic.cpp — physics & collision)
//                   Mitu      (Game.h / Renderer.h — data architecture)
//                   Tripty    (main.cpp — SDL2 window & game loop)
//                   PK        (CMakeLists.txt / Makefile — build system)
// =============================================================================

#include "Renderer.h"
#include "Game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include <cmath>
#include <string>
#include <vector>

static const float W = 420.0f;
static const float H = 520.0f;
static const float PI = 3.14159265f;

static int g_frame = 0; // updated each render call

// ─────────────────────────────────────────────────────────────────────────────
// Primitive helpers
// ─────────────────────────────────────────────────────────────────────────────

static void fillRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void drawFilledCircle(float cx, float cy, float r, int segs = 28)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; i++)
    {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r);
    }
    glEnd();
}

static void drawEllipseFill(float cx, float cy, float rx, float ry,
                            float rotRad = 0.0f, int segs = 36)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotRad * 180.0f / PI, 0, 0, 1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= segs; i++)
    {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(std::cos(a) * rx, std::sin(a) * ry);
    }
    glEnd();
    glPopMatrix();
}

static void drawEllipseOutline(float cx, float cy, float rx, float ry,
                               float rotRad = 0.0f, int segs = 36)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotRad * 180.0f / PI, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segs; i++)
    {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(std::cos(a) * rx, std::sin(a) * ry);
    }
    glEnd();
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel font  (5-wide, 7-tall pixel grid, scale = pixel size in world units)
// ─────────────────────────────────────────────────────────────────────────────

static void drawGlyph(char c, float ox, float oy, float s)
{
    // H(col,row,len) = horizontal run, V = vertical, D = single dot
    auto H = [&](int col, int row, int len)
    { fillRect(ox + col * s, oy + row * s, len * s, s); };
    auto V = [&](int col, int row, int len)
    { fillRect(ox + col * s, oy + row * s, s, len * s); };
    auto D = [&](int col, int row)
    { fillRect(ox + col * s, oy + row * s, s, s); };
    switch (c)
    {
    case '0':
        H(1, 0, 3);
        H(1, 6, 3);
        V(0, 0, 7);
        V(4, 0, 7);
        D(3, 1);
        D(1, 5);
        break;
    case '1':
        V(2, 0, 7);
        break;
    case '2':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(4, 0, 3);
        V(0, 3, 4);
        break;
    case '3':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(4, 0, 7);
        break;
    case '4':
        V(0, 0, 4);
        V(4, 0, 7);
        H(0, 3, 5);
        break;
    case '5':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(0, 0, 4);
        V(4, 3, 4);
        break;
    case '6':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(0, 0, 7);
        V(4, 3, 4);
        break;
    case '7':
        H(0, 0, 5);
        V(4, 0, 7);
        break;
    case '8':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(0, 0, 7);
        V(4, 0, 7);
        break;
    case '9':
        H(0, 0, 5);
        H(0, 3, 5);
        H(0, 6, 5);
        V(0, 0, 4);
        V(4, 0, 7);
        break;
    case 'A':
        H(0, 0, 5);
        H(0, 3, 5);
        V(0, 0, 7);
        V(4, 0, 7);
        break;
    case 'B':
        H(0, 0, 4);
        H(0, 3, 4);
        H(0, 6, 4);
        V(0, 0, 7);
        V(4, 1, 2);
        V(4, 4, 2);
        break;
    case 'C':
        H(1, 0, 4);
        H(1, 6, 4);
        V(0, 1, 5);
        break;
    case 'D':
        H(0, 0, 3);
        H(0, 6, 3);
        V(0, 0, 7);
        V(4, 1, 5);
        D(3, 0);
        D(3, 6);
        break;
    case 'E':
        H(0, 0, 5);
        H(0, 3, 4);
        H(0, 6, 5);
        V(0, 0, 7);
        break;
    case 'F':
        H(0, 0, 5);
        H(0, 3, 4);
        V(0, 0, 7);
        break;
    case 'G':
        H(1, 0, 4);
        H(1, 6, 4);
        V(0, 1, 5);
        H(2, 3, 3);
        V(4, 3, 4);
        break;
    case 'H':
        V(0, 0, 7);
        V(4, 0, 7);
        H(0, 3, 5);
        break;
    case 'I':
        H(0, 0, 5);
        H(0, 6, 5);
        V(2, 0, 7);
        break;
    case 'J':
        H(0, 6, 4);
        V(3, 0, 6);
        D(0, 5);
        D(1, 6);
        break;
    case 'K':
        V(0, 0, 7);
        H(0, 3, 3);
        V(3, 0, 3);
        V(3, 4, 3);
        break;
    case 'L':
        V(0, 0, 7);
        H(0, 6, 5);
        break;
    case 'M':
        V(0, 0, 7);
        V(4, 0, 7);
        D(1, 1);
        D(2, 2);
        D(3, 1);
        break;
    case 'N':
        V(0, 0, 7);
        V(4, 0, 7);
        D(1, 1);
        D(2, 2);
        D(3, 3);
        break;
    case 'O':
        H(1, 0, 3);
        H(1, 6, 3);
        V(0, 1, 5);
        V(4, 1, 5);
        break;
    case 'P':
        H(0, 0, 4);
        H(0, 3, 4);
        V(0, 0, 7);
        V(4, 1, 2);
        break;
    case 'Q':
        H(1, 0, 3);
        H(1, 6, 3);
        V(0, 1, 5);
        V(4, 1, 5);
        D(3, 5);
        D(4, 6);
        break;
    case 'R':
        H(0, 0, 4);
        H(0, 3, 4);
        V(0, 0, 7);
        V(4, 1, 2);
        V(3, 3, 4);
        break;
    case 'S':
        H(1, 0, 4);
        H(1, 3, 3);
        H(0, 6, 4);
        V(0, 0, 3);
        V(4, 3, 4);
        break;
    case 'T':
        H(0, 0, 5);
        V(2, 0, 7);
        break;
    case 'U':
        V(0, 0, 6);
        V(4, 0, 6);
        H(1, 6, 3);
        break;
    case 'V':
        V(0, 0, 5);
        V(4, 0, 5);
        D(1, 5);
        D(3, 5);
        D(2, 6);
        break;
    case 'W':
        V(0, 0, 7);
        V(4, 0, 7);
        V(2, 3, 4);
        D(1, 6);
        D(3, 6);
        break;
    case 'X':
        D(0, 0);
        D(4, 0);
        D(1, 1);
        D(3, 1);
        D(2, 2);
        D(2, 4);
        D(1, 5);
        D(3, 5);
        D(0, 6);
        D(4, 6);
        break;
    case 'Y':
        V(0, 0, 3);
        V(4, 0, 3);
        V(2, 3, 4);
        D(1, 3);
        D(3, 3);
        break;
    case 'Z':
        H(0, 0, 5);
        H(0, 6, 5);
        D(3, 1);
        D(2, 2);
        D(1, 3);
        D(0, 4);
        D(1, 5);
        break;
    case ':':
        D(2, 2);
        D(2, 4);
        break;
    case '-':
        H(1, 3, 3);
        break;
    case ' ':
        break;
    default:
        H(0, 3, 5);
        break;
    }
}

static float textW(const std::string &s, float scale)
{
    return (float)s.size() * 6.0f * scale;
}
static void drawText(const std::string &text, float x, float y, float scale,
                     float r, float g, float b, float a = 1.0f)
{
    glColor4f(r, g, b, a);
    float cx = x;
    for (char c : text)
    {
        drawGlyph(c, cx, y, scale);
        cx += 6.0f * scale;
    }
}
static void drawCentered(const std::string &text, float y, float scale,
                         float r, float g, float b, float a = 1.0f)
{
    drawText(text, W / 2.0f - textW(text, scale) / 2.0f, y, scale, r, g, b, a);
}

// ─────────────────────────────────────────────────────────────────────────────
// Background
// ─────────────────────────────────────────────────────────────────────────────

static void drawBackground()
{
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.0f, 0.06f, 1.0f);
    glVertex2f(0, 0);
    glVertex2f(W, 0);
    glColor4f(0.016f, 0.0f, 0.082f, 1.0f);
    glVertex2f(W, H);
    glVertex2f(0, H);
    glEnd();
    // Nebula
    glColor4f(0.27f, 0.0f, 0.8f, 0.06f);
    drawEllipseFill(100, 160, 120, 80, 0.4f);
    glColor4f(0.0f, 0.13f, 0.6f, 0.06f);
    drawEllipseFill(320, 300, 100, 140, 0.8f);
    glColor4f(0.2f, 0.0f, 0.4f, 0.06f);
    drawEllipseFill(200, 420, 90, 60, 0.2f);
}

static void drawStars(const std::vector<Star> &stars)
{
    for (const auto &s : stars)
    {
        if (s.layer == 2)
        {
            glColor4f(0.8f, 0.87f, 1.0f, s.opacity * 0.3f);
            drawFilledCircle(s.x, s.y, s.radius * 2.8f);
        }
        glColor4f(0.9f, 0.93f, 1.0f, s.opacity);
        drawFilledCircle(s.x, s.y, s.radius);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Player ship
// ─────────────────────────────────────────────────────────────────────────────

static void drawPlayer(const Player &p)
{
    float x = p.x, y = p.y;
    float tF = 0.7f + 0.3f * std::sin(p.thrustAnim);

    // Flames (additive)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int side = -1; side <= 1; side += 2)
    {
        float fx = x + side * 9;
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.0f, 0.92f, 1.0f, 0.9f);
        glVertex2f(fx, y + 16);
        glColor4f(0.0f, 0.39f, 1.0f, 0.0f);
        for (int i = 0; i <= 16; i++)
        {
            float a = (float)i / 16 * 2 * PI;
            glVertex2f(fx + std::cos(a) * 4, y + 16 + std::sin(a) * 10 * tF);
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Wings
    for (int s = -1; s <= 1; s += 2)
    {
        float sx = (float)s;
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.06f, 0.16f, 0.25f, 1.0f);
        glVertex2f(x + sx * 12, y + 16);
        glColor4f(0.12f, 0.35f, 0.54f, 1.0f);
        glVertex2f(x + sx * 14, y + 2);
        glColor4f(0.07f, 0.22f, 0.38f, 1.0f);
        glVertex2f(x + sx * 22, y + 6);
        glColor4f(0.06f, 0.16f, 0.25f, 1.0f);
        glVertex2f(x + sx * 26, y + 18);
        glEnd();
    }

    // Hull
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.75f, 0.91f, 1.0f, 1.0f);
    glVertex2f(x, y - 18);
    glColor4f(0.23f, 0.56f, 0.75f, 1.0f);
    glVertex2f(x - 6, y - 8);
    glColor4f(0.23f, 0.56f, 0.75f, 1.0f);
    glVertex2f(x - 14, y + 2);
    glColor4f(0.04f, 0.16f, 0.27f, 1.0f);
    glVertex2f(x - 12, y + 16);
    glColor4f(0.04f, 0.16f, 0.27f, 1.0f);
    glVertex2f(x - 6, y + 14);
    glColor4f(0.04f, 0.16f, 0.27f, 1.0f);
    glVertex2f(x, y + 10);
    glColor4f(0.04f, 0.16f, 0.27f, 1.0f);
    glVertex2f(x + 6, y + 14);
    glColor4f(0.04f, 0.16f, 0.27f, 1.0f);
    glVertex2f(x + 12, y + 16);
    glColor4f(0.23f, 0.56f, 0.75f, 1.0f);
    glVertex2f(x + 14, y + 2);
    glColor4f(0.23f, 0.56f, 0.75f, 1.0f);
    glVertex2f(x + 6, y - 8);
    glColor4f(0.75f, 0.91f, 1.0f, 1.0f);
    glVertex2f(x, y - 18);
    glEnd();

    // Outline
    glColor4f(0.0f, 0.92f, 1.0f, 0.67f);
    glLineWidth(0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y - 18);
    glVertex2f(x - 6, y - 8);
    glVertex2f(x - 14, y + 2);
    glVertex2f(x - 12, y + 16);
    glVertex2f(x - 6, y + 14);
    glVertex2f(x, y + 10);
    glVertex2f(x + 6, y + 14);
    glVertex2f(x + 12, y + 16);
    glVertex2f(x + 14, y + 2);
    glVertex2f(x + 6, y - 8);
    glEnd();

    // Cockpit
    glColor4f(0.67f, 0.94f, 1.0f, 0.85f);
    drawEllipseFill(x, y - 4, 5, 7);
    glColor4f(0.0f, 0.47f, 0.8f, 0.5f);
    drawEllipseFill(x, y - 4, 4, 6);
    glColor4f(0.0f, 0.13f, 0.27f, 0.65f);
    drawEllipseFill(x + 1, y - 3, 3, 5);
    glColor4f(0.0f, 0.92f, 1.0f, 0.8f);
    drawEllipseOutline(x, y - 4, 5, 7);

    // Guns
    glColor4f(0.0f, 0.92f, 1.0f, 0.8f);
    fillRect(x - 13, y, 3, 10);
    fillRect(x + 10, y, 3, 10);

    // Wing accents
    glColor4f(0.0f, 0.92f, 1.0f, 0.5f);
    glLineWidth(0.5f);
    glBegin(GL_LINES);
    glVertex2f(x - 22, y + 12);
    glVertex2f(x - 16, y + 6);
    glVertex2f(x + 22, y + 12);
    glVertex2f(x + 16, y + 6);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
// Enemy saucer
// ─────────────────────────────────────────────────────────────────────────────

static void drawEnemy(const Enemy &e)
{
    float pulse = 0.9f + 0.1f * std::sin((float)g_frame * 0.12f + e.phase);
    float bossScale = e.isBoss ? 1.8f : 1.0f;
    glPushMatrix();
    glTranslatef(e.x, e.y, 0);
    glScalef(pulse * bossScale, pulse * bossScale, 1.0f);

    float cr = e.isBoss ? 0.8f : 0.4f;
    float cg = 0.0f;
    float cb = e.isBoss ? 0.1f : 0.6f;

    // Saucer body
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr, cg, cb, 1.0f);
    glVertex2f(0, 0);
    glColor4f(cr * 0.25f, cg, cb * 0.33f, 1.0f);
    for (int i = 0; i <= 40; i++)
    {
        float a = (float)i / 40 * 2 * PI;
        glVertex2f(std::cos(a) * 18, std::sin(a) * 9);
    }
    glEnd();
    glColor4f(e.isBoss ? 1.0f : 0.93f, e.isBoss ? 0.4f : 0.53f, e.isBoss ? 0.4f : 1.0f, 0.67f);
    glLineWidth(0.8f);
    drawEllipseOutline(0, 0, 18, 9);

    // Dome
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, e.isBoss ? 0.6f : 0.8f, e.isBoss ? 0.6f : 1.0f, 0.9f);
    glVertex2f(0, -5);
    for (int i = 0; i <= 20; i++)
    {
        float a = PI + (float)i / 20 * PI, t = (float)i / 20;
        glColor4f(0.67f + 0.33f * (1 - t), e.isBoss ? 0.1f * (1 - t) : 0.27f * (1 - t),
                  e.isBoss ? 0.1f : 1.0f, 0.53f);
        glVertex2f(std::cos(a) * 9, std::sin(a) * 7 - 4);
    }
    glEnd();

    // Rotating lights (more for boss)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    int nL = e.isBoss ? 8 : 5;
    for (int i = 0; i < nL; i++)
    {
        float angle = (float)i / nL * 2 * PI + g_frame * 0.07f;
        float lx = std::cos(angle) * 11, ly = std::sin(angle) * 5.5f;
        float lr = 1.0f, lg2 = e.isBoss ? 0.1f : (i % 2 == 0 ? 0.27f : 0.67f), lb2 = e.isBoss ? 0.1f : 1.0f;
        glColor4f(lr, lg2, lb2, 0.9f);
        drawFilledCircle(lx, ly, 2.2f);
        glColor4f(lr, lg2, lb2, 0.25f);
        drawFilledCircle(lx, ly, 5.5f);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Tractor beam
    if (e.y > H * 0.55f)
    {
        float alpha = std::min(1.0f, (e.y - H * 0.55f) / (H * 0.2f)) * 0.5f;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float bR = e.isBoss ? 1.0f : 0.78f, bB = e.isBoss ? 0.2f : 1.0f;
        float bW = e.isBoss ? 14.0f : 10.0f;
        glBegin(GL_QUADS);
        glColor4f(bR, 0.0f, bB, alpha);
        glVertex2f(-6, 6);
        glVertex2f(6, 6);
        glColor4f(bR, 0.0f, bB, 0.0f);
        glVertex2f(bW, 30);
        glVertex2f(-bW, 30);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Boss crown spikes
    if (e.isBoss)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 0.3f, 0.0f, 0.7f);
        for (int i = 0; i < 6; i++)
        {
            float a = (float)i / 6 * 2 * PI + g_frame * 0.02f;
            float sx = std::cos(a) * 18, sy = std::sin(a) * 9;
            glBegin(GL_TRIANGLES);
            glVertex2f(sx, sy);
            glVertex2f(sx + std::cos(a) * 8 - std::sin(a) * 3, sy + std::sin(a) * 8 + std::cos(a) * 3);
            glVertex2f(sx + std::cos(a) * 8 + std::sin(a) * 3, sy + std::sin(a) * 8 - std::cos(a) * 3);
            glEnd();
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
// Bullet
// ─────────────────────────────────────────────────────────────────────────────

static void drawBullet(const Bullet &b)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float r, g, bl;
    float tx, ty; // trail tip (opposite direction of travel)
    if (b.isEnemy)
    {
        r = 1.0f;
        g = 0.25f;
        bl = 0.1f;
        // Normalise velocity for trail
        float len = std::sqrt(b.vx * b.vx + b.vy * b.vy);
        if (len < 0.01f)
            len = 0.01f;
        float nx = b.vx / len, ny = b.vy / len;
        tx = b.x - nx * 14;
        ty = b.y - ny * 14;
    }
    else
    {
        r = 0.0f;
        g = 0.92f;
        bl = 1.0f;
        tx = b.x;
        ty = b.y - 14;
    }
    float x0 = b.x, y0 = b.y;
    // Core trail
    glBegin(GL_QUADS);
    glColor4f(r, g, bl, 1.0f);
    glVertex2f(x0 - 1.5f, y0);
    glVertex2f(x0 + 1.5f, y0);
    glColor4f(r, g, bl, 0.0f);
    glVertex2f(tx + 1.5f, ty);
    glVertex2f(tx - 1.5f, ty);
    glEnd();
    // Glow
    glBegin(GL_QUADS);
    glColor4f(r, g, bl, 0.3f);
    glVertex2f(x0 - 3.5f, y0);
    glVertex2f(x0 + 3.5f, y0);
    glColor4f(r, g, bl, 0.0f);
    glVertex2f(tx + 3.5f, ty);
    glVertex2f(tx - 3.5f, ty);
    glEnd();
    // Bright tip dot
    glColor4f(r, g, bl, 0.9f);
    drawFilledCircle(x0, y0, b.isEnemy ? 2.8f : 2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ─────────────────────────────────────────────────────────────────────────────
// Particles
// ─────────────────────────────────────────────────────────────────────────────

static void drawParticles(const std::vector<Particle> &particles)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (const auto &p : particles)
    {
        glColor4f(p.color.r, p.color.g, p.color.b, p.life);
        drawFilledCircle(p.x, p.y, p.radius);
        glColor4f(p.color.r, p.color.g, p.color.b, p.life * 0.3f);
        drawFilledCircle(p.x, p.y, p.radius * 2.5f);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ─────────────────────────────────────────────────────────────────────────────
// HUD
// ─────────────────────────────────────────────────────────────────────────────

static void drawHUD(const GameData &g)
{
    float s = 1.4f, cy = 8.0f;
    drawText("SCORE:" + std::to_string(g.score), 8, cy, s, 0.0f, 0.92f, 1.0f);

    // Level indicator
    std::string lv = "LV" + std::to_string(g.level);
    drawText(lv, W / 2 - textW(lv, s) / 2, cy, s, 0.0f, 0.92f, 1.0f);

    // Hearts
    float hx = W - 8 - (float)g.lives * 12.0f;
    for (int i = 0; i < g.lives; i++)
    {
        float hcx = hx + i * 12.0f + 5, hcy = cy + 5;
        glColor4f(1.0f, 0.3f, 0.5f, g.player.invincible > 0 ? 0.4f : 1.0f);
        drawFilledCircle(hcx - 2.5f, hcy, 2.8f);
        drawFilledCircle(hcx + 2.5f, hcy, 2.8f);
        glBegin(GL_TRIANGLES);
        glVertex2f(hcx - 5, hcy);
        glVertex2f(hcx + 5, hcy);
        glVertex2f(hcx, hcy + 5.5f);
        glEnd();
    }

    // Separator line
    glColor4f(0.1f, 0.23f, 0.42f, 0.5f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 32);
    glVertex2f(W, 32);
    glEnd();

    // ── Kill progress bar ─────────────────────────────────────────────────
    float barY = H - 12.0f;
    float barW = W - 20.0f;
    float progress = (g.killsNeeded > 0)
                         ? std::min(1.0f, (float)g.killCount / (float)g.killsNeeded)
                         : 0.0f;

    // Background track
    glColor4f(0.1f, 0.15f, 0.25f, 0.7f);
    fillRect(10, barY, barW, 6);

    // Filled portion (color shifts green→yellow→red as level increases)
    float t = (float)(g.level - 1) / 9.0f;
    glColor4f(0.2f + t * 0.8f, 0.9f - t * 0.6f, 0.3f - t * 0.3f, 0.9f);
    fillRect(10, barY, barW * progress, 6);

    // Border
    glColor4f(0.0f, 0.92f, 1.0f, 0.3f);
    glLineWidth(0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(10, barY);
    glVertex2f(10 + barW, barY);
    glVertex2f(10 + barW, barY + 6);
    glVertex2f(10, barY + 6);
    glEnd();

    // Kill count label
    std::string kstr = std::to_string(g.killCount) + ":" + std::to_string(g.killsNeeded);
    drawText(kstr, W / 2 - textW(kstr, 0.9f) / 2, barY - 10, 0.9f, 0.5f, 0.7f, 1.0f);

    // ── Boss HP bar (if any boss on screen) ──────────────────────────────
    for (const auto &e : g.enemies)
    {
        if (e.isBoss && e.active)
        {
            float bossBarW = W - 40.0f;
            float bossBarY = 38.0f;
            float bossHP = (e.maxHp > 0) ? (float)e.hp / (float)e.maxHp : 0.0f;
            // Label
            drawCentered("BOSS", bossBarY - 1, 0.9f, 1.0f, 0.3f, 0.3f);
            // Track
            glColor4f(0.3f, 0.05f, 0.05f, 0.8f);
            fillRect(20, bossBarY + 10, bossBarW, 7);
            // HP fill
            float pulse2 = 0.7f + 0.3f * std::sin((float)g_frame * 0.12f);
            glColor4f(1.0f, 0.2f * pulse2, 0.2f * pulse2, 0.95f);
            fillRect(20, bossBarY + 10, bossBarW * bossHP, 7);
            // Border
            glColor4f(1.0f, 0.4f, 0.4f, 0.5f);
            glLineWidth(0.8f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(20, bossBarY + 10);
            glVertex2f(20 + bossBarW, bossBarY + 10);
            glVertex2f(20 + bossBarW, bossBarY + 17);
            glVertex2f(20, bossBarY + 17);
            glEnd();
            break;
        }
    }

    // ── Difficulty badge (top-right corner) ──────────────────────────────
    std::string dstr;
    float dr = 0, dg = 0, db = 0;
    switch (g.difficulty)
    {
    case Difficulty::EASY:
        dstr = "EASY";
        dr = 0.3f;
        dg = 1.0f;
        db = 0.4f;
        break;
    case Difficulty::MEDIUM:
        dstr = "MEDIUM";
        dr = 1.0f;
        dg = 0.8f;
        db = 0.0f;
        break;
    case Difficulty::HARD:
        dstr = "HARD";
        dr = 1.0f;
        dg = 0.2f;
        db = 0.2f;
        break;
    }
    drawText(dstr, W - 8 - textW(dstr, 0.85f), H - 24, 0.85f, dr, dg, db, 0.8f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Overlay screens
// ─────────────────────────────────────────────────────────────────────────────

static void drawOverlayBg()
{
    glColor4f(0.0f, 0.0f, 0.03f, 0.82f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(W, 0);
    glVertex2f(W, H);
    glVertex2f(0, H);
    glEnd();
}

// Draws a bordered button box, returns true (just draws — state handled by main)
static void drawButton(float cx, float cy, float bw, float bh,
                       const std::string &label, float scale,
                       float r, float g, float b, float pulse = 1.0f)
{
    float bx = cx - bw / 2;
    glColor4f(r, g, b, 0.18f * pulse);
    fillRect(bx, cy - 4, bw, bh);
    glColor4f(r, g, b, 0.4f * pulse);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx, cy - 4);
    glVertex2f(bx + bw, cy - 4);
    glVertex2f(bx + bw, cy + bh - 4);
    glVertex2f(bx, cy + bh - 4);
    glEnd();
    drawCentered(label, cy + 2, scale, r, g, b, pulse);
}

static void drawMenuScreen()
{
    drawOverlayBg();
    float cy = H / 2 - 100;
    drawCentered("SPACE SHOOTER", cy + 1, 1.9f, 0.0f, 0.4f, 0.7f);
    drawCentered("SPACE SHOOTER", cy, 1.9f, 0.0f, 0.92f, 1.0f);
    cy += 52;
    drawCentered("ARROWS OR WASD TO MOVE", cy, 1.0f, 0.67f, 0.8f, 1.0f);
    cy += 18;
    drawCentered("UP DOWN TO DODGE  SPACE TO FIRE", cy, 1.0f, 0.67f, 0.8f, 1.0f);
    cy += 42;
    drawCentered("10 LEVELS  3 DIFFICULTIES", cy, 1.0f, 0.4f, 0.6f, 0.9f);
    cy += 18;
    drawCentered("BOSS ENEMIES  ZIGZAG AI  DIVE ATTACKS", cy, 0.85f, 0.25f, 0.4f, 0.6f);
    cy += 40;
    float p = 0.75f + 0.25f * std::sin((float)g_frame * 0.05f);
    drawButton(W / 2, cy, 130, 26, "PRESS ENTER", 1.2f, 0.0f, 0.92f, 1.0f, p);
}

static void drawDiffSelectScreen()
{
    drawOverlayBg();
    float cy = H / 2 - 100;
    drawCentered("SELECT DIFFICULTY", cy, 1.5f, 0.0f, 0.92f, 1.0f);
    cy += 50;

    float p = 0.8f + 0.2f * std::sin((float)g_frame * 0.06f);

    // Easy
    drawButton(W / 2, cy, 160, 28, "1  EASY", 1.2f, 0.3f, 1.0f, 0.4f, p);
    cy += 14;
    drawCentered("5 LIVES  SLOW  NO BOSSES", cy, 0.8f, 0.2f, 0.7f, 0.3f, 0.8f);

    cy += 38;
    // Medium
    drawButton(W / 2, cy, 160, 28, "2  MEDIUM", 1.2f, 1.0f, 0.75f, 0.0f, p);
    cy += 14;
    drawCentered("3 LIVES  NORMAL  BOSSES LV5+", cy, 0.8f, 0.7f, 0.55f, 0.0f, 0.8f);

    cy += 38;
    // Hard
    drawButton(W / 2, cy, 160, 28, "3  HARD", 1.2f, 1.0f, 0.2f, 0.2f, p);
    cy += 14;
    drawCentered("2 LIVES  FAST  BOSSES FIRE BACK", cy, 0.8f, 0.7f, 0.15f, 0.15f, 0.8f);

    cy += 40;
    drawCentered("BACKSPACE TO GO BACK", cy, 0.8f, 0.3f, 0.4f, 0.5f, 0.6f);
}

static void drawLevelCompleteScreen(int level, int score)
{
    // Semi-transparent flash
    glColor4f(0.0f, 0.3f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(W, 0);
    glVertex2f(W, H);
    glVertex2f(0, H);
    glEnd();

    float cy = H / 2 - 40;
    drawCentered("LEVEL CLEAR", cy + 1, 1.8f, 0.0f, 0.5f, 0.0f);
    drawCentered("LEVEL CLEAR", cy, 1.8f, 0.3f, 1.0f, 0.4f);
    cy += 50;
    if (level <= 10)
    {
        std::string next = "ENTERING LEVEL " + std::to_string(level);
        drawCentered(next, cy, 1.1f, 0.67f, 0.9f, 1.0f);
    }
    cy += 30;
    drawCentered("SCORE:" + std::to_string(score), cy, 1.1f, 0.5f, 0.8f, 1.0f);
}

static void drawVictoryScreen(int score)
{
    drawOverlayBg();
    float cy = H / 2 - 80;
    drawCentered("YOU WIN", cy + 1, 2.4f, 0.5f, 0.5f, 0.0f);
    drawCentered("YOU WIN", cy, 2.4f, 1.0f, 1.0f, 0.2f);
    cy += 65;
    drawCentered("ALL 10 LEVELS CLEARED", cy, 1.1f, 0.8f, 0.9f, 1.0f);
    cy += 25;
    drawCentered("SCORE:" + std::to_string(score), cy, 1.4f, 0.4f, 1.0f, 0.6f);
    cy += 50;
    float pulse = 0.75f + 0.25f * std::sin((float)g_frame * 0.05f);
    drawCentered("PRESS ENTER TO PLAY AGAIN", cy, 1.0f, 0.3f * pulse, 0.47f * pulse, 0.6f * pulse);
}

static void drawGameOverScreen(const GameData &g)
{
    drawOverlayBg();
    float cy = H / 2 - 70;
    drawCentered("GAME OVER", cy + 1, 2.1f, 0.6f, 0.1f, 0.15f);
    drawCentered("GAME OVER", cy, 2.1f, 1.0f, 0.2f, 0.3f);
    cy += 55;
    std::string dlabel;
    switch (g.difficulty)
    {
    case Difficulty::EASY:
        dlabel = "EASY";
        break;
    case Difficulty::MEDIUM:
        dlabel = "MEDIUM";
        break;
    case Difficulty::HARD:
        dlabel = "HARD";
        break;
    }
    drawCentered("DIFFICULTY:" + dlabel, cy, 1.0f, 0.6f, 0.7f, 0.9f);
    cy += 22;
    drawCentered("REACHED LEVEL:" + std::to_string(g.level), cy, 1.0f, 0.5f, 0.7f, 1.0f);
    cy += 22;
    drawCentered("SCORE:" + std::to_string(g.score), cy, 1.4f, 0.67f, 0.8f, 1.0f);
    cy += 45;
    float pulse = 0.75f + 0.25f * std::sin((float)g_frame * 0.05f);
    drawCentered("PRESS ENTER TO RETRY", cy, 1.0f, 0.33f * pulse, 0.47f * pulse, 0.6f * pulse);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void initRenderer()
{
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glDisable(GL_DEPTH_TEST);
}

void renderFrame(GameData &g, int winW, int winH)
{
    g_frame = g.frameCount;

    // Letterbox viewport
    float scaleX = (float)winW / W, scaleY = (float)winH / H;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    int vpW = (int)(W * scale), vpH = (int)(H * scale);
    int vpX = (winW - vpW) / 2, vpY = (winH - vpH) / 2;

    glViewport(0, 0, winW, winH);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(vpX, vpY, vpW, vpH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, W, H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawBackground();
    drawStars(g.stars);

    bool gameActive = (g.state == GameState::PLAYING ||
                       g.state == GameState::LEVEL_COMPLETE ||
                       g.state == GameState::GAMEOVER);
    if (gameActive)
    {
        for (const auto &b : g.bullets)
            drawBullet(b);
        for (const auto &e : g.enemies)
            drawEnemy(e);
        drawParticles(g.particles);
        // Player: flicker when invincible
        bool showPlayer = (g.state == GameState::PLAYING &&
                           (g.player.invincible <= 0 || (g_frame / 4) % 2 == 0));
        if (showPlayer)
            drawPlayer(g.player);
        drawHUD(g);
    }

    // Overlays
    if (g.state == GameState::MENU)
        drawMenuScreen();
    if (g.state == GameState::DIFF_SELECT)
        drawDiffSelectScreen();
    if (g.state == GameState::LEVEL_COMPLETE)
        drawLevelCompleteScreen(g.level, g.score);
    if (g.state == GameState::GAMEOVER)
        drawGameOverScreen(g);
    if (g.state == GameState::VICTORY)
        drawVictoryScreen(g.score);
}
