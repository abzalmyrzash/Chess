#pragma once
#include "chess.h"
#include "graphics.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

extern TTF_Font *font;

extern float boardX, boardY, squareW, squareH;

extern float mouseX, mouseY;
extern Position selFrom, selTo;
extern PieceRef selPiece;

extern bool isPromoting;
extern PieceType promotion;
extern SDL_FRect promPopup;
extern const PieceType promOptions[2][2];

extern char moveNotation[6];
extern int iMoveNotation;

void loadTextures(SDL_Renderer* renderer);
void generateTextures(SDL_Renderer* renderer);
void loadFont();
void initWindow(SDL_Renderer* renderer, PieceColor playerColor);
void calcWindowVars(PieceColor playerColor);
void render(SDL_Renderer* renderer, Game* game);
void renderPiece(SDL_Renderer* renderer, PieceInfo piece, SDL_FRect* rect);
