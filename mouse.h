#pragma once
#include "SDL3/SDL.h"

bool isMouseInCircle(float mouseX, float mouseY,
		float centerX, float centerY, float radius);

bool isMouseInRect(float mouseX, float mouseY, SDL_FRect* rect);
