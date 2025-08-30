#include "window.h"
#include <stdio.h>
#include "mouse.h"

SDL_Texture* textures[2][6];

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* p_filePath)
{
	SDL_Texture* texture = NULL;
	texture = IMG_LoadTexture(renderer, p_filePath);

	if (texture == NULL)
		printf("Failed to load texture. Error: %s\n", SDL_GetError());

	return texture;
}
						
void loadTextures(SDL_Renderer* renderer)
{
	textures[WHITE][PAWN] = loadTexture(renderer, "assets/wP.svg");
	textures[WHITE][KNIGHT] = loadTexture(renderer, "assets/wN.svg");
	textures[WHITE][BISHOP] = loadTexture(renderer, "assets/wB.svg");
	textures[WHITE][ROOK] = loadTexture(renderer, "assets/wR.svg");
	textures[WHITE][QUEEN] = loadTexture(renderer, "assets/wQ.svg");
	textures[WHITE][KING] = loadTexture(renderer, "assets/wK.svg");

	textures[BLACK][PAWN] = loadTexture(renderer, "assets/bP.svg");
	textures[BLACK][KNIGHT] = loadTexture(renderer, "assets/bN.svg");
	textures[BLACK][BISHOP] = loadTexture(renderer, "assets/bB.svg");
	textures[BLACK][ROOK] = loadTexture(renderer, "assets/bR.svg");
	textures[BLACK][QUEEN] = loadTexture(renderer, "assets/bQ.svg");
	textures[BLACK][KING] = loadTexture(renderer, "assets/bK.svg");
}

SDL_FRect boardSquares[8][8];
float boardX, boardY, squareW, squareH;

const SDL_Color SQUARE_COLOR_DARK = { 100, 60, 60, 255 };
const SDL_Color SQUARE_COLOR_LIGHT = { 230, 220, 220, 255 };
const SDL_Color BACKGROUND_COLOR = { 190, 160, 160, 255 };
const SDL_Color BACKGROUND_COLOR2 = { 140, 110, 110, 255 };
const SDL_Color PROM_POPUP_COLOR = { 255, 255, 255, 255 };
const SDL_Color PROM_HIGHLIGHT_COLOR = { 200, 200, 200, 255};

float mouseX, mouseY;
Position selFrom, selTo;
PieceRef selPiece;

bool isPromoting;
PieceType promotion;
SDL_FRect promPopup;
const PieceType promOptions[2][2] = { { QUEEN, ROOK}, { BISHOP, KNIGHT } };

void initWindow()
{
	mouseX = 0;
	mouseY = 0;
	selFrom = (Position) {NONE, NONE};
	selTo = (Position) {NONE, NONE};
	selPiece = NONE;
	promotion = NONE;
	isPromoting = false;
	calcWindowVars();
}

void calcWindowVars()
{
	boardX = 100;
	boardY = 100;
	squareW = 100;
	squareH = 100;
	SDL_FRect rect = {.w = squareW, .h = squareH };
	for (int i = 0; i < 8; i++) {
		rect.y = boardY + (7 - i) * squareH;
		for (int j = 0; j < 8; j++) {
			rect.x = boardX + j * squareH;
			boardSquares[i][j] = rect;
		}
	}
	promPopup = (SDL_FRect) { .y = 200, .w = 200, .h = 200 };
}

void renderPiece(SDL_Renderer* renderer, PieceInfo piece, SDL_FRect* rect)
{
	SDL_Texture* tex = textures[getPieceColor(piece)][getPieceType(piece)];
	SDL_RenderTexture(renderer, tex, NULL, rect);
}

void render(SDL_Renderer* renderer, GameState* game)
{
	if (game->colorToMove == WHITE)
		setColor(renderer, &BACKGROUND_COLOR);
	else setColor(renderer, &BACKGROUND_COLOR2);

	static PieceColor prevColor = WHITE;
	if (prevColor != game->colorToMove) {
		prevColor = game->colorToMove;
		printBoard(game);
	}

	SDL_RenderClear(renderer);
	const SDL_Color* squareColor;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if ((i + j) % 2 == 0) squareColor = &SQUARE_COLOR_DARK;
			else squareColor = &SQUARE_COLOR_LIGHT;
			renderRect(renderer, &boardSquares[i][j], 0, squareColor, NULL);
		}
	}

	SDL_FRect rect = {.w = squareW, .h = squareH };
	Piece piece;
	Position pos;
	PieceType type;
	SDL_Texture* tex;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 16; j++) {
			if (selPiece == i * 16 + j) continue;
			piece = game->pieces[i][j];
			type = getPieceType(piece.info);
			tex = textures[i][type];
			pos = getPos(piece.pos);
			if (isPosValid(pos) == false) continue;
			rect = boardSquares[pos.rank][pos.file];
			SDL_RenderTexture(renderer, tex, NULL, &rect);
		}
	}

	PieceColor color;
	if (selPiece != NONE) {
		piece = getPieceByRef(selPiece, game);
		color = getPieceColor(piece.info);
		type = getPieceType(piece.info);
		tex = textures[color][type];
		rect.x = mouseX - squareW / 2;
		rect.y = mouseY - squareH / 2;
		if (isPromoting) {
			rect = boardSquares[selTo.rank][selTo.file];
		}
		SDL_RenderTexture(renderer, tex, NULL, &rect);
	}

	if (selPiece != NONE && isPromoting) {
		renderRect(renderer, &promPopup, 0,
			&PROM_POPUP_COLOR, NULL);

		for (int i = 0; i < 2; i++) {
			rect.y = promPopup.y + i * squareH;
			for (int j = 0; j < 2; j++) {
				rect.x = promPopup.x + j * squareW;
				type = promOptions[i][j];
				tex = textures[color][type];
				if (isMouseInRect(mouseX, mouseY, &rect))
					renderRect(renderer, &rect, 0, &PROM_HIGHLIGHT_COLOR, NULL);
				SDL_RenderTexture(renderer, tex, NULL, &rect);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

void renderPromotion()
{
}
