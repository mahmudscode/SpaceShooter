/**
 * main.cpp - Entry point for the Space Shooter game
 *
 * This file initializes SDL2 and OpenGL, creates the game window,
 * and runs the main game loop (input handling, updates, and rendering).
 */

#include <SDL2/SDL.h>      // SDL2 library for window/input/audio
#include <SDL2/SDL_opengl.h> // SDL2 OpenGL support
#include <GL/gl.h>           // OpenGL for graphics rendering

#include "Game.h"            // Game logic (GameData, game states, entities)
#include "Renderer.h"        // Graphics rendering functions

// Forward declarations - these functions are defined in Game.cpp
void resetStars(GameData &g);      // Initialize/reset starfield background
void startGame(GameData &g);       // Start a new game session
void updateGame(GameData &g);      // Update game during play
void updateMenuStars(GameData &g); // Update stars in menu/game over screens

// Window dimensions (portrait orientation like a phone screen)
static const int WIN_W = 840;   // Window width in pixels
static const int WIN_H = 1040;  // Window height in pixels

/**
 * Main function - Program entry point
 * argc/argv: Command line arguments (unused)
 * Returns: 0 on success, 1 on error
 */
int main(int argc, char *argv[])
{
    // --- Initialize SDL2 ----------------------------------------------------
    // SDL_INIT_VIDEO: Initialize video subsystem (window, rendering)
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return 1;  // Exit with error code
    }

    // --- Configure OpenGL context settings ----------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);   // OpenGL 2.x
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);   // OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);            // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);              // No depth buffer (2D game)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);      // Enable antialiasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);      // 4x MSAA antialiasing

    // --- Create the game window --------------------------------------------
    SDL_Window *window = SDL_CreateWindow(
        "SPACE SHOOTER",                         // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // Center on screen
        WIN_W, WIN_H,                            // Window size
        SDL_WINDOW_OPENGL |                      // Use OpenGL for rendering
        SDL_WINDOW_RESIZABLE |                   // Window can be resized
        SDL_WINDOW_SHOWN);                       // Show immediately
    if (!window)
    {
        SDL_Log("Window error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // --- Create OpenGL context ------------------------------------------------
    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx)
    {
        SDL_Log("GL error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Enable vsync (1 = wait for vertical refresh, prevents screen tearing)
    SDL_GL_SetSwapInterval(1);

    // --- Initialize game systems --------------------------------------------
    GameData game;         // Create game state object
    initRenderer();        // Set up OpenGL rendering (from Renderer.h)
    resetStars(game);      // Initialize starfield background

    // --- Main game loop variables -------------------------------------------
    bool running = true;                // Loop control flag
    SDL_Event event;                   // Event structure for input handling
    const float FRAME_TIME = 1000.0f / 60.0f;  // Target: 60 FPS (16.67ms per frame)

    // ========================================================================
    // MAIN GAME LOOP
    // ========================================================================
    while (running)
    {
        Uint32 now = SDL_GetTicks();   // Get current time in milliseconds

        // --- Process all pending events (input) ------------------------------
        while (SDL_PollEvent(&event))
        {
            // Event handling - process user input
            if (event.type == SDL_QUIT)
            {
                running = false;    // User clicked window's X button
            }

            // --- KEY PRESSED DOWN ------------------------------------------------
            else if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode k = event.key.keysym.sym;  // Get key code

                // -- MOVEMENT KEYS (work in all game states) --
                // Support both arrow keys and WASD
                if (k == SDLK_LEFT || k == SDLK_a)
                {
                    game.keyLeft = true;  // Move left
                    game.keyA = true;
                }
                if (k == SDLK_RIGHT || k == SDLK_d)
                {
                    game.keyRight = true; // Move right
                    game.keyD = true;
                }
                if (k == SDLK_UP || k == SDLK_w)
                {
                    game.keyUp = true;    // Move up
                    game.keyW = true;
                }
                if (k == SDLK_DOWN || k == SDLK_s)
                {
                    game.keyDown = true;  // Move down
                    game.keyS = true;
                }
                if (k == SDLK_SPACE)
                    game.keySpace = true;   // Fire weapon
                if (k == SDLK_ESCAPE)
                    running = false;        // Quit game

                // -- STATE-SPECIFIC KEYS --

                // MENU state: Press Enter to go to difficulty selection
                if (game.state == GameState::MENU)
                {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER)
                        game.state = GameState::DIFF_SELECT;
                }

                // DIFF_SELECT state: Choose difficulty (1, 2, 3) or go back
                else if (game.state == GameState::DIFF_SELECT)
                {
                    if (k == SDLK_1 || k == SDLK_KP_1)
                    {
                        game.difficulty = Difficulty::EASY;   // Easy mode
                        startGame(game);
                    }
                    else if (k == SDLK_2 || k == SDLK_KP_2)
                    {
                        game.difficulty = Difficulty::MEDIUM; // Medium mode
                        startGame(game);
                    }
                    else if (k == SDLK_3 || k == SDLK_KP_3)
                    {
                        game.difficulty = Difficulty::HARD;   // Hard mode
                        startGame(game);
                    }
                    else if (k == SDLK_BACKSPACE)
                    {
                        game.state = GameState::MENU;  // Return to menu
                    }
                }

                // GAMEOVER or VICTORY: Press Enter to restart
                else if (game.state == GameState::GAMEOVER ||
                         game.state == GameState::VICTORY)
                {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER)
                        game.state = GameState::DIFF_SELECT;  // Back to difficulty select
                }
            }

            // --- KEY RELEASED ----------------------------------------------------
            else if (event.type == SDL_KEYUP)
            {
                SDL_Keycode k = event.key.keysym.sym;
                // Reset movement flags when keys are released
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
                    game.keySpace = false;  // Stop firing
            }
        }

        // --- GAME UPDATE ------------------------------------------------------
        // Update game logic based on current state
        if (game.state == GameState::MENU ||
            game.state == GameState::DIFF_SELECT ||
            game.state == GameState::GAMEOVER ||
            game.state == GameState::VICTORY)
        {
            // In menu/game over: just animate stars in background
            updateMenuStars(game);
        }
        else
        {
            // PLAYING or LEVEL_COMPLETE: full game update (movement, collision, etc.)
            updateGame(game);
        }

        // --- RENDER -----------------------------------------------------------
        int w, h;                           // Get current window dimensions
        SDL_GetWindowSize(window, &w, &h);
        renderFrame(game, w, h);            // Draw everything
        SDL_GL_SwapWindow(window);          // Swap front/back buffers (show frame)

        // --- FRAME RATE CONTROL -----------------------------------------------
        // Calculate how long this frame took
        Uint32 elapsed = SDL_GetTicks() - now;
        // If we finished early, delay to maintain 60 FPS
        if (elapsed < (Uint32)FRAME_TIME)
            SDL_Delay((Uint32)FRAME_TIME - elapsed);
    }
    // --- END OF GAME LOOP -------------------------------------------------

    // --- CLEANUP ------------------------------------------------------------
    SDL_GL_DeleteContext(glCtx);  // Destroy OpenGL context
    SDL_DestroyWindow(window);    // Destroy game window
    SDL_Quit();                   // Shutdown SDL
    return 0;                     // Exit program successfully
}
