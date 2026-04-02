#pragma once
#include "Game.h"

// Forward declare to avoid including GL headers in header
void renderFrame(GameData& g, int winW, int winH);
void renderMenu(GameData& g, int winW, int winH);
void renderGameOver(GameData& g, int winW, int winH);
void initRenderer();
