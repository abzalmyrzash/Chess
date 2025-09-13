#include "window.h"
#include <stdio.h>
#include "mouse.h"

SDL_Texture* textures[2][6];
SDL_Texture* legalCircleTexture;

float legalCircleRadius;

SDL_FRect boardSquares[64];
float boardX, boardY, squareW, squareH;

const SDL_Color SQUARE_COLOR_DARK = { 100, 60, 60, 255 };
const SDL_Color SQUARE_COLOR_LIGHT = { 230, 220, 220, 255 };
const SDL_Color BACKGROUND_COLOR = { 190, 160, 160, 255 };
const SDL_Color BACKGROUND_COLOR2 = { 140, 110, 110, 255 };
const SDL_Color PROM_POPUP_COLOR = { 255, 255, 255, 255 };
const SDL_Color PROM_HIGHLIGHT_COLOR = { 200, 200, 200, 255};
const SDL_Color LEGAL_CIRCLE_COLOR = { 128, 128, 128, 255 };
const SDL_Color MOVE_COLOR = { 255, 255, 0, 80 };

float mouseX, mouseY;
Position selFrom, selTo;
PieceRef selPiece;
Position prevFrom, prevTo;

bool isPromoting;
PieceType promotion;
SDL_FRect promPopup;
const PieceType promOptions[2][2] = { { QUEEN, ROOK}, { BISHOP, KNIGHT } };

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

	legalCircleTexture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		squareW, squareH);
	SDL_SetRenderTarget(renderer, legalCircleTexture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	renderCircle(renderer, squareW / 2, squareH / 2,
		legalCircleRadius, 0, &LEGAL_CIRCLE_COLOR, NULL);
	SDL_SetRenderTarget(renderer, NULL);
}

void initWindow(SDL_Renderer* renderer)
{
	mouseX = 0;
	mouseY = 0;
	selFrom = NONE;
	selTo = NONE;
	selPiece = NONE;
	prevFrom = NONE;
	prevTo = NONE;
	promotion = NONE;
	isPromoting = false;
	calcWindowVars();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void calcWindowVars()
{
	boardX = 100;
	boardY = 100;
	squareW = 100;
	squareH = 100;
	legalCircleRadius = 25;

	SDL_FRect rect = {.w = squareW, .h = squareH };

	for (int i = 0; i < 8; i++) {
		rect.y = boardY + (7 - i) * squareH;
		for (int j = 0; j < 8; j++) {
			rect.x = boardX + j * squareH;
			boardSquares[getPos(i, j)] = rect;
		}
	}
	promPopup = (SDL_FRect) { .y = 200, .w = 200, .h = 200 };
}

void renderPiece(SDL_Renderer* renderer, PieceInfo piece, SDL_FRect* rect)
{
	SDL_Texture* tex = textures[getPieceColor(piece)][getPieceType(piece)];
	SDL_RenderTexture(renderer, tex, NULL, rect);
}

void render(SDL_Renderer* renderer, Game* game)
{
	if (game->colorToMove == WHITE)
		setColor(renderer, &BACKGROUND_COLOR);
	else setColor(renderer, &BACKGROUND_COLOR2);

	static PieceColor prevColor = WHITE;
	if (prevColor != game->colorToMove) {
		prevColor = game->colorToMove;
		// printBoard(game);
	}

	SDL_RenderClear(renderer);
	const SDL_Color* squareColor;
	Position pos;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if ((i + j) % 2 == 0) squareColor = &SQUARE_COLOR_DARK;
			else squareColor = &SQUARE_COLOR_LIGHT;
			pos = getPos(i, j);
			renderRect(renderer, &boardSquares[pos], 0, squareColor, NULL);
			if (pos == prevFrom || pos == prevTo) {
				renderRect(renderer, &boardSquares[pos], 0, &MOVE_COLOR, NULL);
			}
		}
	}

	SDL_FRect rect = {.w = squareW, .h = squareH };
	Piece piece;
	PieceType type;
	SDL_Texture* tex;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 16; j++) {
			if (selPiece == i * 16 + j) continue;
			piece = game->pieces[i][j];
			type = getPieceType(piece.info);
			tex = textures[i][type];
			pos = piece.pos;
			if (isPosValid(pos) == false) continue;
			rect = boardSquares[pos];
			SDL_RenderTexture(renderer, tex, NULL, &rect);
		}
	}

	PieceColor color;
	if (selPiece != NONE) {
		piece = getPieceByRef(selPiece, game);
		color = getPieceColor(piece.info);
		type = getPieceType(piece.info);

		Bitboard bb = game->legalMovesBB[color][selPiece & 0b1111];
		if (bb) do {
			Position pos = bitScanForward(bb);
			rect = boardSquares[pos];
			SDL_RenderTexture(renderer, legalCircleTexture, NULL, &rect);
		} while(bb &= bb - 1);

		tex = textures[color][type];
		rect.x = mouseX - squareW / 2;
		rect.y = mouseY - squareH / 2;
		if (isPromoting) {
			rect = boardSquares[selTo];
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
