#include "label.h"
#include <string.h>

void initLabel(Label* label, char* text, TTF_Font* font,
				SDL_FRect rect, SDL_Color color)
{
	label->text = text;
	label->font = font;
	label->rect = rect;
	label->color = color;
	label->dstRect.x = rect.x;
	label->dstRect.y = rect.y;
}

bool updateLabelText(Label* label, char* text)
{
	char* prevText = label->text;
	label->text = text;
	return (strcmp(prevText, text) != 0);
}

void createLabelTexture(SDL_Renderer* renderer, Label* label)
{
	if (strlen(label->text) == 0) {
		label->texture = NULL;
		return;
	}
	SDL_Surface* surface = TTF_RenderText_Blended(label->font, label->text, 0, label->color);
	if (surface == NULL) {
		SDL_Log("SDL_GetError: %s\n", SDL_GetError());
	}
	label->texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (label->texture == NULL) {
		SDL_Log("SDL_GetError: %s\n", SDL_GetError());
	}
    SDL_GetTextureSize(label->texture, &label->dstRect.w, &label->dstRect.h);
	
	SDL_DestroySurface(surface);
}

void updateLabelTexture(SDL_Renderer* renderer, Label* label)
{
	SDL_DestroyTexture(label->texture);
	createLabelTexture(renderer, label);
}

void renderLabel(SDL_Renderer* renderer, Label* label)
{
	SDL_RenderTexture(renderer, label->texture, NULL, &label->dstRect);
}
