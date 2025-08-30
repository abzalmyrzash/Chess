#include "mouse.h"

bool isMouseInCircle(float mouseX, float mouseY,
		float centerX, float centerY, float radius)
{
	float dx = mouseX - centerX;
	float dy = mouseY - centerY;
	return (dx * dx + dy * dy) < radius * radius;
}

bool isMouseInRect(float mouseX, float mouseY, SDL_FRect* rect) {
	return (mouseX > rect->x && mouseX < rect->x + rect->w) &&
		(mouseY > rect->y && mouseY < rect->y + rect->h);
}
