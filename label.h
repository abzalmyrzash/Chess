#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>

typedef struct {
	char* text;
	TTF_Font* font;
	SDL_FRect rect;
	SDL_FRect dstRect;
	SDL_Color color;
	SDL_Texture* texture;
	bool needsUpdate;
} Label;

void initLabel(Label* label, char* text, TTF_Font* font,
				SDL_FRect rect, SDL_Color color);

bool updateLabelText(Label* label, char* text);

void createLabelTexture(SDL_Renderer* renderer, Label* label);

void updateLabelTexture(SDL_Renderer* renderer, Label* label);

void renderLabel(SDL_Renderer* renderer, Label* label);
