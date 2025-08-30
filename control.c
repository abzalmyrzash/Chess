#include "control.h"
#include "window.h"
#include <stdio.h>
#include <math.h>

void onClick(float x, float y, GameState* game)
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
	selFrom.rank = 7 - floor((y - boardY) / squareH);
	selFrom.file = floor((x - boardX) / squareW);
	if (isPosValid(selFrom) == false) goto reset;
	selPiece = game->board[selFrom.rank][selFrom.file];
	if (selPiece == NONE) goto reset;
	PieceColor color = getPieceColor(getPieceByRef(selPiece, game).info);
	if (game->colorToMove != color) goto reset;
	mouseX = x;
	mouseY = y;
	return;

	reset:
	selFrom.rank = NONE;
	selFrom.file = NONE;
	selPiece = NONE;
}

void onMove(float x, float y, GameState* game)
{
	if (selPiece == NONE) return;
	mouseX = x;
	mouseY = y;
}

void onRelease(float x, float y, GameState* game)
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
		checkAndMove(selFrom, selTo, promotion, game);
		goto reset;
	}

	selTo.rank = 7 - floor((y - boardY) / squareH);
	selTo.file = floor((x - boardX) / squareW);
	if (isMovePromotion(selFrom, selTo, game)
		&& isMoveLegal(selFrom, selTo, game))
	{
		isPromoting = true;
		promPopup.x = boardX + (selTo.file - 0.5) * squareW;
		return;
	}
	checkAndMove(selFrom, selTo, NONE, game);
	reset:
	selFrom.rank = NONE;
	selFrom.file = NONE;
	selTo.rank = NONE;
	selTo.file = NONE;
	selPiece = NONE;
}

static bool ctrl = false;
static bool shift = false;
void onKeyDown(SDL_KeyboardEvent event, GameState* game)
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

void onKeyUp(SDL_KeyboardEvent event, GameState* game)
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
