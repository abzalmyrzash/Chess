#include "control.h"
#include "window.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

char sendbuf[1024];

void resetSelection() {
	selFrom = NONE;
	selTo = NONE;
	selPiece = NONE;
	promotion = NONE;
	isPromoting = false;
}

void onClick(float x, float y, Game* game, PieceColor playerColor)
{
	if (isPromoting) {
		int8_t row, column;
		row = floor((y - promPopup.y) / squareH);
		column = floor((x - promPopup.x) / squareW);
		if (row < 0 || row > 1 || column < 0 || column > 1) {
			promotion = NONE;
			return;
		}
		promotion = promOptions[row][column];
		return;
	}

	uint8_t rank;
	uint8_t file;
	if (playerColor == WHITE || playerColor == NONE) {
		rank = 7 - floor((y - boardY) / squareH);
		file = floor((x - boardX) / squareW);
	} else {
		rank = floor((y - boardY) / squareH);
		file = 7 - floor((x - boardX) / squareW);
	}
	if (isRankFileValid(rank, file) == false) goto reset;

	selFrom = getPos(rank, file);
	selPiece = game->refBoard[selFrom];
	if (selPiece == NONE) goto reset;
	PieceColor color = getPieceColor(getPieceByRef(selPiece, game).info);
	if (game->colorToMove != color) goto reset;
	if (game->colorToMove != playerColor && playerColor != NONE) goto reset;
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

void onRelease(float x, float y, Game* game, PieceColor playerColor,
	SOCKET socket, bool* quit)
{
	if (selPiece == NONE) return;
	if (isPromoting) {
		int8_t row, column;
		row = floor((y - promPopup.y) / squareH);
		column = floor((x - promPopup.x) / squareW);
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

	uint8_t rank;
	uint8_t file;
	if (playerColor == WHITE || playerColor == NONE) {
		rank = 7 - floor((y - boardY) / squareH);
		file = floor((x - boardX) / squareW);
	} else {
		rank = floor((y - boardY) / squareH);
		file = 7 - floor((x - boardX) / squareW);
	}
	if (isRankFileValid(rank, file) == false) goto reset;

	selTo = getPos(rank, file);
	if (isMovePromotion(selFrom, selTo, game)
		&& isMoveLegal(selFrom, selTo, game))
	{
		isPromoting = true;
		if (playerColor == WHITE || playerColor == NONE) {
			promPopup.x = boardX + (file - 0.5) * squareW;
		} else {
			promPopup.x = boardX + ((7 - file) - 0.5) * squareW;
		}
		if (playerColor == WHITE || (playerColor == NONE && game->colorToMove == WHITE)) {
			promPopup.y = boardY + squareH;
		} else {
			promPopup.y = boardY + 5 * squareH;
		}
		return;
	}

	makeMove:
	Move move = checkAndMove(selFrom, selTo, promotion, game);
	if (move.type != NONE) {
		if (socket != INVALID_SOCKET) {
			int sendlen = 3;
			sendbuf[0] = selFrom;
			sendbuf[1] = selTo;
			sendbuf[2] = promotion;
			int iResult;
			iResult = sendData(socket, sendbuf, sendlen);
			if (iResult == -1) {
				printf("send failed!\n");
				fflush(stdout);
				*quit = true;
			}
		}
	}

	reset:
	selFrom = NONE;
	selTo = NONE;
	selPiece = NONE;
	promotion = NONE;
}

int onReceive(ReceiveThreadData* data, Game* game)
{
	assert(data->status == THREAD_FINISHED);
	if (data->result <= 0) return data->result;
	data->status = NO_THREAD;
	Position from = data->recvbuf[0];
	Position to = data->recvbuf[1];
	PieceType prom = data->recvbuf[2];
	Move move = checkAndMove(from, to, prom, game);
	if (move.type == NONE) {
		printf("Received illegal move! (%d %d %d)\n", from, to, prom);
		return -1;
	} else {
		return data->result;
	}
}

static bool ctrl = false;
static bool shift = false;

void onKeyDown(SDL_KeyboardEvent event, Game* game, SOCKET socket, bool* quit)
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
		if (ctrl && socket == INVALID_SOCKET) {
			if (shift) redoMove(game);
			else {
				undoMove(game);
				resetSelection();
			}
		}
		break;
	case SDLK_Y:
		if (ctrl && socket == INVALID_SOCKET) {
			redoMove(game);
			resetSelection();
		}
		break;
	case SDLK_C:
		if (ctrl) {
			resetSelection();
		}
		break;
	case SDLK_Q:
		if (ctrl) {
			*quit = true;
		}
		break;
	case SDLK_RETURN:
		moveNotation[iMoveNotation] = '\0';
		moveByNotation(moveNotation, game);
		while(iMoveNotation > 0) {
			moveNotation[--iMoveNotation] = '\0';
		}
		break;
	case SDLK_BACKSPACE:
		if (iMoveNotation == 0) break;
		moveNotation[--iMoveNotation] = '\0';
		break;
	case SDLK_SPACE:
	case SDLK_ESCAPE:
		while(iMoveNotation > 0) {
			moveNotation[--iMoveNotation] = '\0';
		}
		break;
	default:
		break;
	}

	if (!ctrl && (event.key >= 'a' && event.key <= 'z') 
		|| (event.key >= '0' && event.key <= '9'))
	{
		if (iMoveNotation < 5) {
			moveNotation[iMoveNotation++] = event.key;
		}
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
