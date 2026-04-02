// SpaceShooter Renderer - OpenGL 2.1 fixed pipeline
// Matches the HTML canvas version exactly (same geometry, colors, effects)
// No external font/image libraries needed.

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

static void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x,   y);   glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
}

static void drawFilledCircle(float cx, float cy, float r, int segs = 28) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; i++) {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(cx + std::cos(a)*r, cy + std::sin(a)*r);
    }
    glEnd();
}

static void drawEllipseFill(float cx, float cy, float rx, float ry,
                             float rotRad = 0.0f, int segs = 36) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotRad * 180.0f / PI, 0, 0, 1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= segs; i++) {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(std::cos(a)*rx, std::sin(a)*ry);
    }
    glEnd();
    glPopMatrix();
}

static void drawEllipseOutline(float cx, float cy, float rx, float ry,
                                float rotRad = 0.0f, int segs = 36) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotRad * 180.0f / PI, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segs; i++) {
        float a = (float)i / segs * 2.0f * PI;
        glVertex2f(std::cos(a)*rx, std::sin(a)*ry);
    }
    glEnd();
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel font  (5-wide, 7-tall pixel grid, scale = pixel size in world units)
// ─────────────────────────────────────────────────────────────────────────────

static void drawGlyph(char c, float ox, float oy, float s) {
    // H(col,row,len) = horizontal run, V = vertical, D = single dot
    auto H = [&](int col,int row,int len){ fillRect(ox+col*s, oy+row*s, len*s, s); };
    auto V = [&](int col,int row,int len){ fillRect(ox+col*s, oy+row*s, s,     len*s); };
    auto D = [&](int col,int row)        { fillRect(ox+col*s, oy+row*s, s,     s); };
    switch(c) {
    case '0': H(1,0,3);H(1,6,3);V(0,0,7);V(4,0,7);D(3,1);D(1,5); break;
    case '1': V(2,0,7); break;
    case '2': H(0,0,5);H(0,3,5);H(0,6,5);V(4,0,3);V(0,3,4); break;
    case '3': H(0,0,5);H(0,3,5);H(0,6,5);V(4,0,7); break;
    case '4': V(0,0,4);V(4,0,7);H(0,3,5); break;
    case '5': H(0,0,5);H(0,3,5);H(0,6,5);V(0,0,4);V(4,3,4); break;
    case '6': H(0,0,5);H(0,3,5);H(0,6,5);V(0,0,7);V(4,3,4); break;
    case '7': H(0,0,5);V(4,0,7); break;
    case '8': H(0,0,5);H(0,3,5);H(0,6,5);V(0,0,7);V(4,0,7); break;
    case '9': H(0,0,5);H(0,3,5);H(0,6,5);V(0,0,4);V(4,0,7); break;
    case 'A': H(0,0,5);H(0,3,5);V(0,0,7);V(4,0,7); break;
    case 'B': H(0,0,4);H(0,3,4);H(0,6,4);V(0,0,7);V(4,1,2);V(4,4,2); break;
    case 'C': H(1,0,4);H(1,6,4);V(0,1,5); break;
    case 'D': H(0,0,3);H(0,6,3);V(0,0,7);V(4,1,5);D(3,0);D(3,6); break;
    case 'E': H(0,0,5);H(0,3,4);H(0,6,5);V(0,0,7); break;
    case 'F': H(0,0,5);H(0,3,4);V(0,0,7); break;
    case 'G': H(1,0,4);H(1,6,4);V(0,1,5);H(2,3,3);V(4,3,4); break;
    case 'H': V(0,0,7);V(4,0,7);H(0,3,5); break;
    case 'I': H(0,0,5);H(0,6,5);V(2,0,7); break;
    case 'J': H(0,6,4);V(3,0,6);D(0,5);D(1,6); break;
    case 'K': V(0,0,7);H(0,3,3);V(3,0,3);V(3,4,3); break;
    case 'L': V(0,0,7);H(0,6,5); break;
    case 'M': V(0,0,7);V(4,0,7);D(1,1);D(2,2);D(3,1); break;
    case 'N': V(0,0,7);V(4,0,7);D(1,1);D(2,2);D(3,3); break;
    case 'O': H(1,0,3);H(1,6,3);V(0,1,5);V(4,1,5); break;
    case 'P': H(0,0,4);H(0,3,4);V(0,0,7);V(4,1,2); break;
    case 'Q': H(1,0,3);H(1,6,3);V(0,1,5);V(4,1,5);D(3,5);D(4,6); break;
    case 'R': H(0,0,4);H(0,3,4);V(0,0,7);V(4,1,2);V(3,3,4); break;
    case 'S': H(1,0,4);H(1,3,3);H(0,6,4);V(0,0,3);V(4,3,4); break;
    case 'T': H(0,0,5);V(2,0,7); break;
    case 'U': V(0,0,6);V(4,0,6);H(1,6,3); break;
    case 'V': V(0,0,5);V(4,0,5);D(1,5);D(3,5);D(2,6); break;
    case 'W': V(0,0,7);V(4,0,7);V(2,3,4);D(1,6);D(3,6); break;
    case 'X': D(0,0);D(4,0);D(1,1);D(3,1);D(2,2);D(2,4);D(1,5);D(3,5);D(0,6);D(4,6); break;
    case 'Y': V(0,0,3);V(4,0,3);V(2,3,4);D(1,3);D(3,3); break;
    case 'Z': H(0,0,5);H(0,6,5);D(3,1);D(2,2);D(1,3);D(0,4);D(1,5); break;
    case ':': D(2,2);D(2,4); break;
    case '-': H(1,3,3); break;
    case ' ': break;
    default:  H(0,3,5); break;
    }
}

static float textW(const std::string& s, float scale) {
    return (float)s.size() * 6.0f * scale;
}
static void drawText(const std::string& text, float x, float y, float scale,
                     float r, float g, float b, float a=1.0f) {
    glColor4f(r,g,b,a);
    float cx = x;
    for (char c : text) { drawGlyph(c,cx,y,scale); cx += 6.0f*scale; }
}
static void drawCentered(const std::string& text, float y, float scale,
                          float r, float g, float b, float a=1.0f) {
    drawText(text, W/2.0f - textW(text,scale)/2.0f, y, scale, r,g,b,a);
}

// ─────────────────────────────────────────────────────────────────────────────
// Background
// ─────────────────────────────────────────────────────────────────────────────

static void drawBackground() {
    glBegin(GL_QUADS);
    glColor4f(0.0f,0.0f,0.06f,1.0f);  glVertex2f(0,0); glVertex2f(W,0);
    glColor4f(0.016f,0.0f,0.082f,1.0f); glVertex2f(W,H); glVertex2f(0,H);
    glEnd();
    // Nebula
    glColor4f(0.27f,0.0f,0.8f,0.06f); drawEllipseFill(100,160,120,80,0.4f);
    glColor4f(0.0f,0.13f,0.6f,0.06f); drawEllipseFill(320,300,100,140,0.8f);
    glColor4f(0.2f,0.0f,0.4f,0.06f);  drawEllipseFill(200,420,90,60,0.2f);
}

static void drawStars(const std::vector<Star>& stars) {
    for (const auto& s : stars) {
        if (s.layer==2) {
            glColor4f(0.8f,0.87f,1.0f, s.opacity*0.3f);
            drawFilledCircle(s.x,s.y,s.radius*2.8f);
        }
        glColor4f(0.9f,0.93f,1.0f,s.opacity);
        drawFilledCircle(s.x,s.y,s.radius);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Player ship
// ─────────────────────────────────────────────────────────────────────────────

static void drawPlayer(const Player& p) {
    float x=p.x, y=p.y;
    float tF = 0.7f + 0.3f*std::sin(p.thrustAnim);

    // Flames (additive)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int side=-1; side<=1; side+=2) {
        float fx = x + side*9;
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.0f,0.92f,1.0f,0.9f); glVertex2f(fx, y+16);
        glColor4f(0.0f,0.39f,1.0f,0.0f);
        for(int i=0;i<=16;i++){
            float a=(float)i/16*2*PI;
            glVertex2f(fx+std::cos(a)*4, y+16+std::sin(a)*10*tF);
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Wings
    for (int s=-1; s<=1; s+=2) {
        float sx=(float)s;
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.06f,0.16f,0.25f,1.0f); glVertex2f(x+sx*12,y+16);
        glColor4f(0.12f,0.35f,0.54f,1.0f); glVertex2f(x+sx*14,y+2);
        glColor4f(0.07f,0.22f,0.38f,1.0f); glVertex2f(x+sx*22,y+6);
        glColor4f(0.06f,0.16f,0.25f,1.0f); glVertex2f(x+sx*26,y+18);
        glEnd();
    }

    // Hull
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.75f,0.91f,1.0f,1.0f);  glVertex2f(x,   y-18);
    glColor4f(0.23f,0.56f,0.75f,1.0f); glVertex2f(x-6, y-8);
    glColor4f(0.23f,0.56f,0.75f,1.0f); glVertex2f(x-14,y+2);
    glColor4f(0.04f,0.16f,0.27f,1.0f); glVertex2f(x-12,y+16);
    glColor4f(0.04f,0.16f,0.27f,1.0f); glVertex2f(x-6, y+14);
    glColor4f(0.04f,0.16f,0.27f,1.0f); glVertex2f(x,   y+10);
    glColor4f(0.04f,0.16f,0.27f,1.0f); glVertex2f(x+6, y+14);
    glColor4f(0.04f,0.16f,0.27f,1.0f); glVertex2f(x+12,y+16);
    glColor4f(0.23f,0.56f,0.75f,1.0f); glVertex2f(x+14,y+2);
    glColor4f(0.23f,0.56f,0.75f,1.0f); glVertex2f(x+6, y-8);
    glColor4f(0.75f,0.91f,1.0f,1.0f);  glVertex2f(x,   y-18);
    glEnd();

    // Outline
    glColor4f(0.0f,0.92f,1.0f,0.67f); glLineWidth(0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,y-18);glVertex2f(x-6,y-8);glVertex2f(x-14,y+2);
    glVertex2f(x-12,y+16);glVertex2f(x-6,y+14);glVertex2f(x,y+10);
    glVertex2f(x+6,y+14);glVertex2f(x+12,y+16);glVertex2f(x+14,y+2);glVertex2f(x+6,y-8);
    glEnd();

    // Cockpit
    glColor4f(0.67f,0.94f,1.0f,0.85f); drawEllipseFill(x,y-4,5,7);
    glColor4f(0.0f,0.47f,0.8f,0.5f);   drawEllipseFill(x,y-4,4,6);
    glColor4f(0.0f,0.13f,0.27f,0.65f); drawEllipseFill(x+1,y-3,3,5);
    glColor4f(0.0f,0.92f,1.0f,0.8f);   drawEllipseOutline(x,y-4,5,7);

    // Guns
    glColor4f(0.0f,0.92f,1.0f,0.8f);
    fillRect(x-13,y,3,10); fillRect(x+10,y,3,10);

    // Wing accents
    glColor4f(0.0f,0.92f,1.0f,0.5f); glLineWidth(0.5f);
    glBegin(GL_LINES);
    glVertex2f(x-22,y+12);glVertex2f(x-16,y+6);
    glVertex2f(x+22,y+12);glVertex2f(x+16,y+6);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
// Enemy saucer
// ─────────────────────────────────────────────────────────────────────────────

static void drawEnemy(const Enemy& e) {
    float pulse = 0.9f + 0.1f*std::sin((float)g_frame*0.12f + e.phase);
    glPushMatrix();
    glTranslatef(e.x,e.y,0); glScalef(pulse,pulse,1.0f);

    // Saucer body
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.4f,0.0f,0.6f,1.0f); glVertex2f(0,0);
    glColor4f(0.1f,0.0f,0.2f,1.0f);
    for(int i=0;i<=40;i++){float a=(float)i/40*2*PI;glVertex2f(std::cos(a)*18,std::sin(a)*9);}
    glEnd();
    glColor4f(0.93f,0.53f,1.0f,0.67f); glLineWidth(0.8f);
    drawEllipseOutline(0,0,18,9);

    // Dome
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f,0.8f,1.0f,0.9f); glVertex2f(0,-5);
    for(int i=0;i<=20;i++){
        float a=PI+(float)i/20*PI, t=(float)i/20;
        glColor4f(0.67f+0.33f*(1-t),0.27f*(1-t),1.0f,0.53f);
        glVertex2f(std::cos(a)*9, std::sin(a)*7-4);
    }
    glEnd();

    // Rotating lights
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(int i=0;i<5;i++){
        float angle=(float)i/5*2*PI + g_frame*0.07f;
        float lx=std::cos(angle)*11, ly=std::sin(angle)*5.5f;
        float lg = (i%2==0)?0.27f:0.67f;
        glColor4f(1.0f,lg,1.0f,0.9f); drawFilledCircle(lx,ly,2.2f);
        glColor4f(1.0f,lg,1.0f,0.25f); drawFilledCircle(lx,ly,5.5f);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Tractor beam
    if(e.y>H*0.55f){
        float alpha=std::min(1.0f,(e.y-H*0.55f)/(H*0.2f))*0.5f;
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        glBegin(GL_QUADS);
        glColor4f(0.78f,0.0f,1.0f,alpha); glVertex2f(-6,6);  glVertex2f(6,6);
        glColor4f(0.78f,0.0f,1.0f,0.0f);  glVertex2f(10,30); glVertex2f(-10,30);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
// Bullet
// ─────────────────────────────────────────────────────────────────────────────

static void drawBullet(const Bullet& b){
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBegin(GL_QUADS);
    glColor4f(0.0f,0.92f,1.0f,1.0f); glVertex2f(b.x-1.5f,b.y);   glVertex2f(b.x+1.5f,b.y);
    glColor4f(0.0f,0.78f,1.0f,0.0f); glVertex2f(b.x+1.5f,b.y-14); glVertex2f(b.x-1.5f,b.y-14);
    glEnd();
    glBegin(GL_QUADS);
    glColor4f(0.0f,0.92f,1.0f,0.3f); glVertex2f(b.x-3.5f,b.y);   glVertex2f(b.x+3.5f,b.y);
    glColor4f(0.0f,0.78f,1.0f,0.0f); glVertex2f(b.x+3.5f,b.y-14); glVertex2f(b.x-3.5f,b.y-14);
    glEnd();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

// ─────────────────────────────────────────────────────────────────────────────
// Particles
// ─────────────────────────────────────────────────────────────────────────────

static void drawParticles(const std::vector<Particle>& particles){
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(const auto& p:particles){
        glColor4f(p.color.r,p.color.g,p.color.b,p.life);
        drawFilledCircle(p.x,p.y,p.radius);
        glColor4f(p.color.r,p.color.g,p.color.b,p.life*0.3f);
        drawFilledCircle(p.x,p.y,p.radius*2.5f);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

// ─────────────────────────────────────────────────────────────────────────────
// HUD
// ─────────────────────────────────────────────────────────────────────────────

static void drawHUD(int score, int level, int lives){
    float s=1.4f, cy=8.0f;
    drawText("SCORE:"+std::to_string(score), 8,cy, s, 0.0f,0.92f,1.0f);
    std::string lv="LV"+std::to_string(level);
    drawText(lv, W/2-textW(lv,s)/2, cy, s, 0.0f,0.92f,1.0f);
    // Hearts
    float hx = W-8-(float)lives*12.0f;
    for(int i=0;i<lives;i++){
        float hcx=hx+i*12.0f+5, hcy=cy+5;
        glColor4f(1.0f,0.3f,0.5f,1.0f);
        drawFilledCircle(hcx-2.5f,hcy,2.8f);
        drawFilledCircle(hcx+2.5f,hcy,2.8f);
        glBegin(GL_TRIANGLES);
        glVertex2f(hcx-5,hcy);glVertex2f(hcx+5,hcy);glVertex2f(hcx,hcy+5.5f);
        glEnd();
    }
    // Separator
    glColor4f(0.1f,0.23f,0.42f,0.5f); glLineWidth(1.0f);
    glBegin(GL_LINES); glVertex2f(0,32); glVertex2f(W,32); glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
// Overlay screens
// ─────────────────────────────────────────────────────────────────────────────

static void drawOverlayBg(){
    glColor4f(0.0f,0.0f,0.03f,0.82f);
    glBegin(GL_QUADS);
    glVertex2f(0,0);glVertex2f(W,0);glVertex2f(W,H);glVertex2f(0,H);
    glEnd();
}

static void drawMenuScreen(){
    drawOverlayBg();
    float cy=H/2-90;
    // Title with glow
    drawCentered("SPACE SHOOTER",cy+1,1.9f,0.0f,0.4f,0.7f);
    drawCentered("SPACE SHOOTER",cy,  1.9f,0.0f,0.92f,1.0f);
    cy+=55;
    drawCentered("ARROWS OR WASD TO MOVE",cy,1.0f,0.67f,0.8f,1.0f);
    cy+=20;
    drawCentered("SPACE TO FIRE",cy,1.0f,0.67f,0.8f,1.0f);
    cy+=50;
    float pulse=0.75f+0.25f*std::sin((float)g_frame*0.05f);
    // Button
    float bx=W/2-65,bw=130,bh=26;
    glColor4f(0.0f,0.57f,0.63f,0.18f*pulse);
    fillRect(bx,cy-4,bw,bh);
    glColor4f(0.0f,0.92f,1.0f,0.35f*pulse); glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx,cy-4);glVertex2f(bx+bw,cy-4);
    glVertex2f(bx+bw,cy+bh-4);glVertex2f(bx,cy+bh-4);
    glEnd();
    drawCentered("PRESS ENTER",cy+2,1.2f,0.0f,0.92f*pulse,1.0f*pulse);
    cy+=55;
    drawCentered("PARALLAX STARS  CUSTOM SHIPS  ALIEN SWARMS",cy,0.85f,0.2f,0.33f,0.47f);
}

static void drawGameOverScreen(int score){
    drawOverlayBg();
    float cy=H/2-60;
    drawCentered("GAME OVER",cy+1,2.1f,0.6f,0.1f,0.15f);
    drawCentered("GAME OVER",cy,  2.1f,1.0f,0.2f,0.3f);
    cy+=60;
    drawCentered("SCORE:"+std::to_string(score),cy,1.4f,0.67f,0.8f,1.0f);
    cy+=50;
    float pulse=0.75f+0.25f*std::sin((float)g_frame*0.05f);
    drawCentered("PRESS ENTER TO RESTART",cy,1.0f,0.33f*pulse,0.47f*pulse,0.6f*pulse);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void initRenderer(){
    glClearColor(0.0f,0.0f,0.05f,1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glDisable(GL_DEPTH_TEST);
}

void renderFrame(GameData& g, int winW, int winH){
    g_frame = g.frameCount;

    // Letterbox viewport
    float scaleX=(float)winW/W, scaleY=(float)winH/H;
    float scale=(scaleX<scaleY)?scaleX:scaleY;
    int vpW=(int)(W*scale), vpH=(int)(H*scale);
    int vpX=(winW-vpW)/2,   vpY=(winH-vpH)/2;

    glViewport(0,0,winW,winH);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(vpX,vpY,vpW,vpH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,W,H,0,-1,1); // y-down
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    drawBackground();
    drawStars(g.stars);

    if(g.state==GameState::PLAYING||g.state==GameState::GAMEOVER){
        for(const auto& b:g.bullets) drawBullet(b);
        for(const auto& e:g.enemies) drawEnemy(e);
        drawParticles(g.particles);
        if(g.state==GameState::PLAYING) drawPlayer(g.player);
        drawHUD(g.score,g.level,g.lives);
    }
    if(g.state==GameState::MENU)     drawMenuScreen();
    if(g.state==GameState::GAMEOVER) drawGameOverScreen(g.score);
}
