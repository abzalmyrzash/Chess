#pragma once
#include "chess.h"
#include <SDL3/SDL.h>

void onClick(float x, float y, Game* game);

void onMove(float x, float y, Game* game);

void onRelease(float x, float y, Game* game);

void onKeyDown(SDL_KeyboardEvent event, Game* game);

void onKeyUp(SDL_KeyboardEvent event, Game* game);

