#include "control.h"
#include "window.h"
#include <stdio.h>
#include <math.h>

void onClick(float x, float y, Game* game)
{
	if (isPromoting) {
		int8_t row, column;
		row = floor((y - promPopup.y) / squareH);
		column = floor((x - promPopup.x) / squareW);
		printf("%d %d\n", row, column);
		if (row < 0 || row > 1 || column < 0 || column > 1) {
			promotion = NONE;
			return;
		}
		promotion = promOptions[row][column];
		printf("%d\n", promotion);
		return;
	}
	uint8_t rank = 7 - floor((y - boardY) / squareH);
	uint8_t file = floor((x - boardX) / squareW);
	selFrom = getPos(rank, file);
	if (isPosValid(selFrom) == false) goto reset;
	selPiece = game->refBoard[selFrom];
	if (selPiece == NONE) goto reset;
	PieceColor color = getPieceColor(getPieceByRef(selPiece, game).info);
	if (game->colorToMove != color) goto reset;
	Bitboard legalMoves = *((Bitboard*)game->legalMovesBB + selPiece);
	if (legalMoves == 0) goto reset;
	mouseX = x;
	mouseY = y;
	return;

	reset:
	selFrom = NONE;
	selPiece = NONE;
}

void onMove(float x, float y, Game* game)
{
	if (selPiece == NONE) return;
	mouseX = x;
	mouseY = y;
}

void onRelease(float x, float y, Game* game)
{
	if (selPiece == NONE) return;
	if (isPromoting) {
		int8_t row, column;
		row = floor((y - promPopup.y) / squareH);
		column = floor((x - promPopup.x) / squareW);
		printf("%d %d\n", row, column);
		if (row < 0 || row > 1 || column < 0 || column > 1 ||
			promotion == NONE || promotion != promOptions[row][column])
		{
			isPromoting = false;
			promotion = NONE;
			goto reset;
		}
		isPromoting = false;
		goto makeMove;
	}

	uint8_t rank = 7 - floor((y - boardY) / squareH);
	uint8_t file = floor((x - boardX) / squareW);
	selTo = getPos(rank, file);
	if (isMovePromotion(selFrom, selTo, game)
		&& isMoveLegal(selFrom, selTo, game))
	{
		isPromoting = true;
		promPopup.x = boardX + (file - 0.5) * squareW;
		return;
	}
	makeMove:
	Move move = checkAndMove(selFrom, selTo, promotion, game);
	if (move.type != NONE) {
		prevFrom = selFrom;
		prevTo = selTo;
	}
	reset:
	selFrom = NONE;
	selTo = NONE;
	selPiece = NONE;
}

static bool ctrl = false;
static bool shift = false;
void onKeyDown(SDL_KeyboardEvent event, Game* game)
{
	switch(event.key)
	{
	case SDLK_LCTRL:
		ctrl = true;
		break;
	case SDLK_LSHIFT:
		shift = true;
		break;
	case SDLK_Z:
		if (ctrl) {
			if (shift) redoMove(game);
			else undoMove(game);
		}
		break;
	case SDLK_Y:
		if (!ctrl) break;
		redoMove(game);
		break;
	default:
		break;
	}
}

void onKeyUp(SDL_KeyboardEvent event, Game* game)
{
	switch(event.key)
	{
	case SDLK_LCTRL:
		ctrl = false;
		break;
	case SDLK_LSHIFT:
		shift = false;
		break;
	default:
		break;
	}
}
