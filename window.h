#pragma once
#include "chess.h"
#include "graphics.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

extern float boardX, boardY, squareW, squareH;

extern float mouseX, mouseY;
extern Position selFrom, selTo;
extern PieceRef selPiece;

extern bool isPromoting;
extern PieceType promotion;
extern SDL_FRect promPopup;
extern const PieceType promOptions[2][2];

void loadTextures(SDL_Renderer* renderer);
void initWindow();
void calcWindowVars();
void render(SDL_Renderer* renderer, GameState* game);
void renderPiece(SDL_Renderer* renderer, PieceInfo piece, SDL_FRect* rect);
