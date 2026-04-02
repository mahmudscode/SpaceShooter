// SpaceShooter - C++ OpenGL version
// Requires: SDL2, OpenGL
// Build: see CMakeLists.txt or Makefile

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "Game.h"
#include "Renderer.h"

// Forward declarations from GameLogic.cpp
void resetStars(GameData& g);
void startGame(GameData& g);
void updateGame(GameData& g);
void updateMenuStars(GameData& g);

static const int WIN_W = 840;  // 2x game width for crisp rendering
static const int WIN_H = 1040; // 2x game height

int main(int argc, char* argv[]) {
    // ── SDL2 init ─────────────────────────────────────────────────────────
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // MSAA 4x

    SDL_Window* window = SDL_CreateWindow(
        "SPACE SHOOTER",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) {
        SDL_Log("SDL_GL_CreateContext error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // VSync

    // ── Game init ─────────────────────────────────────────────────────────
    GameData game;
    initRenderer();
    resetStars(game);

    // ── Main loop ─────────────────────────────────────────────────────────
    bool running = true;
    SDL_Event event;
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1000.0f / TARGET_FPS;

    while (running) {
        Uint32 now = SDL_GetTicks();

        // ── Events ────────────────────────────────────────────────────────
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode k = event.key.keysym.sym;
                if (k == SDLK_LEFT  || k == SDLK_a) game.keyLeft  = true;
                if (k == SDLK_RIGHT || k == SDLK_d) game.keyRight = true;
                if (k == SDLK_a) game.keyA = true;
                if (k == SDLK_d) game.keyD = true;
                if (k == SDLK_SPACE) game.keySpace = true;

                // Start / restart
                if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                    if (game.state == GameState::MENU ||
                        game.state == GameState::GAMEOVER) {
                        startGame(game);
                    }
                }
                if (k == SDLK_ESCAPE) running = false;
            }
            else if (event.type == SDL_KEYUP) {
                SDL_Keycode k = event.key.keysym.sym;
                if (k == SDLK_LEFT  || k == SDLK_a) game.keyLeft  = false;
                if (k == SDLK_RIGHT || k == SDLK_d) game.keyRight = false;
                if (k == SDLK_a) game.keyA = false;
                if (k == SDLK_d) game.keyD = false;
                if (k == SDLK_SPACE) game.keySpace = false;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                // Handle resize — just re-render
            }
        }

        // ── Update ────────────────────────────────────────────────────────
        if (game.state == GameState::MENU || game.state == GameState::GAMEOVER) {
            updateMenuStars(game);
        } else {
            updateGame(game);
        }

        // ── Render ────────────────────────────────────────────────────────
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        renderFrame(game, w, h);
        SDL_GL_SwapWindow(window);

        // ── Frame cap (if VSync off) ──────────────────────────────────────
        Uint32 elapsed = SDL_GetTicks() - now;
        if (elapsed < (Uint32)FRAME_TIME) {
            SDL_Delay((Uint32)FRAME_TIME - elapsed);
        }
    }

    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
