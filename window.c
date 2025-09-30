#include "window.h"
#include <stdio.h>
#include "mouse.h"
#include "label.h"

TTF_Font *font = NULL;

SDL_Texture* pieceTextures[2][6];
//SDL_Texture* borderTexture;
SDL_Texture* legalCircleTexture;

float legalCircleRadius;

SDL_FRect boardSquares[64];
SDL_FRect borderRect;
float boardX, boardY, squareW, squareH;
float borderWidth;

const SDL_Color FONT_COLOR = { 40, 23, 20, 255 };
const SDL_Color SQUARE_COLOR_DARK = { 100, 70, 60, 255 };
const SDL_Color SQUARE_COLOR_LIGHT = { 230, 223, 220, 255 };
const SDL_Color BORDER_COLOR = { 40, 23, 20, 255 };
const SDL_Color BACKGROUND_COLOR = { 150, 130, 100, 255 };
const SDL_Color PROM_POPUP_COLOR = { 255, 255, 255, 255 };
const SDL_Color PROM_HIGHLIGHT_COLOR = { 200, 200, 200, 255};
const SDL_Color LEGAL_CIRCLE_COLOR = { 128, 128, 128, 255 };
const SDL_Color MOVE_COLOR = { 255, 255, 0, 80 };

float mouseX, mouseY;
Position selFrom, selTo;
PieceRef selPiece;

bool isPromoting;
PieceType promotion;
SDL_FRect promPopup;
const PieceType promOptions[2][2] = { { QUEEN, ROOK}, { BISHOP, KNIGHT } };

char moveNotation[6] = {0};
int iMoveNotation = 0;
Label moveNotationLabel;
SDL_FRect moveNotationRect;

Label gameStateLabel;
SDL_FRect gameStateRect;

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
	pieceTextures[WHITE][PAWN] = loadTexture(renderer, "assets/pieces/wP.svg");
	pieceTextures[WHITE][KNIGHT] = loadTexture(renderer, "assets/pieces/wN.svg");
	pieceTextures[WHITE][BISHOP] = loadTexture(renderer, "assets/pieces/wB.svg");
	pieceTextures[WHITE][ROOK] = loadTexture(renderer, "assets/pieces/wR.svg");
	pieceTextures[WHITE][QUEEN] = loadTexture(renderer, "assets/pieces/wQ.svg");
	pieceTextures[WHITE][KING] = loadTexture(renderer, "assets/pieces/wK.svg");

	pieceTextures[BLACK][PAWN] = loadTexture(renderer, "assets/pieces/bP.svg");
	pieceTextures[BLACK][KNIGHT] = loadTexture(renderer, "assets/pieces/bN.svg");
	pieceTextures[BLACK][BISHOP] = loadTexture(renderer, "assets/pieces/bB.svg");
	pieceTextures[BLACK][ROOK] = loadTexture(renderer, "assets/pieces/bR.svg");
	pieceTextures[BLACK][QUEEN] = loadTexture(renderer, "assets/pieces/bQ.svg");
	pieceTextures[BLACK][KING] = loadTexture(renderer, "assets/pieces/bK.svg");
}

void generateTextures(SDL_Renderer* renderer)
{
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

void loadFont()
{
	font = TTF_OpenFont("assets/fonts/arial.ttf", 20.0f);
	if (font == NULL) {
		SDL_Log("SDL_GetError: %s\n", SDL_GetError());
	}
}

void initWindow(SDL_Renderer* renderer, PieceColor playerColor)
{
	mouseX = 0;
	mouseY = 0;
	selFrom = NONE;
	selTo = NONE;
	selPiece = NONE;
	promotion = NONE;
	isPromoting = false;

	calcWindowVars(playerColor);

	initLabel(&moveNotationLabel, moveNotation, font, moveNotationRect, FONT_COLOR);
	initLabel(&gameStateLabel, "start", font, gameStateRect, FONT_COLOR);
	createLabelTexture(renderer, &moveNotationLabel);
	createLabelTexture(renderer, &gameStateLabel);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void calcWindowVars(PieceColor playerColor)
{
	boardX = 180;
	boardY = 180;
	squareW = 80;
	squareH = 80;
	borderWidth = 20;
	legalCircleRadius = 20;

	SDL_FRect rect = {.w = squareW, .h = squareH };

	if (playerColor == WHITE || playerColor == NONE)
	{
		for (int i = 0; i < 8; i++) {
			rect.y = boardY + (7 - i) * squareH;
			for (int j = 0; j < 8; j++) {
				rect.x = boardX + j * squareW;
				boardSquares[getPos(i, j)] = rect;
			}
		}
	}

	else {
		for (int i = 0; i < 8; i++) {
			rect.y = boardY + i * squareH;
			for (int j = 0; j < 8; j++) {
				rect.x = boardX + (7 - j) * squareW;
				boardSquares[getPos(i, j)] = rect;
			}
		}
	}

	promPopup = (SDL_FRect) { .w = 2 * squareW, .h = 2 * squareH };
	borderRect = (SDL_FRect) { .x = boardX - borderWidth, .y = boardY - borderWidth,
		.w = squareW * 8 + borderWidth * 2, .h = squareH * 8 + borderWidth * 2 };

	moveNotationRect = (SDL_FRect) { .x = boardX + squareW * 7, .y = boardY + squareH * 8 + 40,
									.w = 100, .h = 20 };

	gameStateRect = (SDL_FRect) { .x = boardX, .y = boardY + squareH * 8 + 40,
									.w = 200, .h = 20 };
}

void renderPiece(SDL_Renderer* renderer, PieceInfo piece, SDL_FRect* rect)
{
	SDL_Texture* tex = pieceTextures[getPieceColor(piece)][getPieceType(piece)];
	SDL_RenderTexture(renderer, tex, NULL, rect);
}

void updateGameStateLabel(SDL_Renderer* renderer, Game* game)
{
	char* newText;
	switch(game->state)
	{
	case WHITE_WON:
		newText = "WHITE WON!";
		break;
	case DRAW:
		newText = "DRAW!";
		break;
	case BLACK_WON:
		newText = "BLACK WON!";
		break;
	default:
		if (game->colorToMove == WHITE) {
			if (isInCheck(WHITE, game)) {
				newText = "WHITE IN CHECK!";
			} else {
				newText = "WHITE TO MOVE";
			}
		}
		else {
			if (isInCheck(BLACK, game)) {
				newText = "BLACK IN CHECK!";
			} else {
				newText = "BLACK TO MOVE";
			}
		}
		break;
	}
	if (gameStateLabel.text != newText) {
		gameStateLabel.text = newText;
		updateLabelTexture(renderer, &gameStateLabel);
	}
}

void render(SDL_Renderer* renderer, Game* game)
{
	setColor(renderer, &BACKGROUND_COLOR);
	SDL_RenderClear(renderer);

	renderRect(renderer, &borderRect, borderWidth, NULL, &BORDER_COLOR);
	const SDL_Color* squareColor;
	Position pos;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if ((i + j) % 2 == 0) squareColor = &SQUARE_COLOR_DARK;
			else squareColor = &SQUARE_COLOR_LIGHT;
			pos = getPos(i, j);
			renderRect(renderer, &boardSquares[pos], 0, squareColor, NULL);
			if (game->moveCnt > game->minMove &&
					(pos == game->history[game->moveCnt - 1].from
					|| pos == game->history[game->moveCnt - 1].to))
			{
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
			tex = pieceTextures[i][type];
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

		tex = pieceTextures[color][type];
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
				tex = pieceTextures[color][type];
				if (isMouseInRect(mouseX, mouseY, &rect))
					renderRect(renderer, &rect, 0, &PROM_HIGHLIGHT_COLOR, NULL);
				SDL_RenderTexture(renderer, tex, NULL, &rect);
			}
		}
	}

	updateLabelTexture(renderer, &moveNotationLabel);
	renderLabel(renderer, &moveNotationLabel);
	updateGameStateLabel(renderer, game);
	renderLabel(renderer, &gameStateLabel);

	SDL_RenderPresent(renderer);
}
