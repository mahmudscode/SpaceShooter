// =============================================================================
//  SpaceShooter — Application Entry Point
// =============================================================================
//
//  AUTHOR         : Tripty
//                   [ SDL2 Window Management & Main Game Loop ]
//
//  Prime Author   : Mahmudur Rahman  (Renderer.cpp)
//  Contributors   : Era (GameLogic.cpp) · Mitu (Game.h) · PK (Build)
// =============================================================================

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "Game.h"
#include "Renderer.h"

void resetStars(GameData &g);
void startGame(GameData &g);
void updateGame(GameData &g);
void updateMenuStars(GameData &g);

static const int WIN_W = 840;
static const int WIN_H = 1040;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window *window = SDL_CreateWindow(
        "SPACE SHOOTER",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Log("Window error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx)
    {
        SDL_Log("GL error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1);

    GameData game;
    initRenderer();
    resetStars(game);

    bool running = true;
    SDL_Event event;
    const float FRAME_TIME = 1000.0f / 60.0f;

    while (running)
    {
        Uint32 now = SDL_GetTicks();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            else if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode k = event.key.keysym.sym;

                // ── Movement (always) ──────────────────────────────────
                if (k == SDLK_LEFT || k == SDLK_a)
                {
                    game.keyLeft = true;
                    game.keyA = true;
                }
                if (k == SDLK_RIGHT || k == SDLK_d)
                {
                    game.keyRight = true;
                    game.keyD = true;
                }
                if (k == SDLK_UP || k == SDLK_w)
                {
                    game.keyUp = true;
                    game.keyW = true;
                }
                if (k == SDLK_DOWN || k == SDLK_s)
                {
                    game.keyDown = true;
                    game.keyS = true;
                }
                if (k == SDLK_SPACE)
                    game.keySpace = true;
                if (k == SDLK_ESCAPE)
                    running = false;

                // ── Menu: go to difficulty select ──────────────────────
                if (game.state == GameState::MENU)
                {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER)
                        game.state = GameState::DIFF_SELECT;
                }

                // ── Difficulty select ──────────────────────────────────
                else if (game.state == GameState::DIFF_SELECT)
                {
                    if (k == SDLK_1 || k == SDLK_KP_1)
                    {
                        game.difficulty = Difficulty::EASY;
                        startGame(game);
                    }
                    else if (k == SDLK_2 || k == SDLK_KP_2)
                    {
                        game.difficulty = Difficulty::MEDIUM;
                        startGame(game);
                    }
                    else if (k == SDLK_3 || k == SDLK_KP_3)
                    {
                        game.difficulty = Difficulty::HARD;
                        startGame(game);
                    }
                    else if (k == SDLK_BACKSPACE)
                    {
                        game.state = GameState::MENU;
                    }
                }

                // ── Game over / victory: restart ───────────────────────
                else if (game.state == GameState::GAMEOVER ||
                         game.state == GameState::VICTORY)
                {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER)
                        game.state = GameState::DIFF_SELECT;
                }
            }

            else if (event.type == SDL_KEYUP)
            {
                SDL_Keycode k = event.key.keysym.sym;
                if (k == SDLK_LEFT || k == SDLK_a)
                {
                    game.keyLeft = false;
                    game.keyA = false;
                }
                if (k == SDLK_RIGHT || k == SDLK_d)
                {
                    game.keyRight = false;
                    game.keyD = false;
                }
                if (k == SDLK_UP || k == SDLK_w)
                {
                    game.keyUp = false;
                    game.keyW = false;
                }
                if (k == SDLK_DOWN || k == SDLK_s)
                {
                    game.keyDown = false;
                    game.keyS = false;
                }
                if (k == SDLK_SPACE)
                    game.keySpace = false;
            }
        }

        // ── Update ────────────────────────────────────────────────────────
        if (game.state == GameState::MENU ||
            game.state == GameState::DIFF_SELECT ||
            game.state == GameState::GAMEOVER ||
            game.state == GameState::VICTORY)
        {
            updateMenuStars(game);
        }
        else
        {
            updateGame(game); // handles PLAYING + LEVEL_COMPLETE internally
        }

        // ── Render ────────────────────────────────────────────────────────
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        renderFrame(game, w, h);
        SDL_GL_SwapWindow(window);

        Uint32 elapsed = SDL_GetTicks() - now;
        if (elapsed < (Uint32)FRAME_TIME)
            SDL_Delay((Uint32)FRAME_TIME - elapsed);
    }

    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
