#pragma once
#include "chess.h"
#include <SDL3/SDL.h>

void onClick(float x, float y, GameState* game);

void onMove(float x, float y, GameState* game);

void onRelease(float x, float y, GameState* game);

void onKeyDown(SDL_KeyboardEvent event, GameState* game);

void onKeyUp(SDL_KeyboardEvent event, GameState* game);

