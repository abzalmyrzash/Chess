#pragma once
#include "chess.h"
#include "net/network.h"
#include <SDL3/SDL.h>

void onClick(float x, float y, Game* game, PieceColor playerColor);

void onMove(float x, float y, Game* game);

void onRelease(float x, float y, Game* game, PieceColor playerColor,
	SOCKET socket, bool* quit);

int onReceive(ReceiveThreadData* data, Game* game);

void onKeyDown(SDL_KeyboardEvent event, Game* game, PieceColor playerColor,
	SOCKET socket, bool* quit);

void onKeyUp(SDL_KeyboardEvent event, Game* game);

