#include "stdio.h"
#include "chess.h"
#include "window.h"
#include "control.h"
#include "net/network.h"
#include "net/server.h"
#include "net/client.h"

#define SCREEN_HEIGHT 1000
#define SCREEN_WIDTH 1000

void clear_stdin() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

int main(int argc, char** argv) {
	SOCKET serverSocket, socket;
	int iResult;
	bool offline = false;
	bool isServer = false;
	char* windowName;
	PieceColor playerColor;

menu:
	printf("1. Play offline\n");
	printf("2. Start server\n");
	printf("3. Connect\n");
	printf("4. Quit\n");
	printf("Type option number: ");
	int opt;
	if (scanf("%d", &opt) != 1) {
		clear_stdin();
		goto menu;
	}
	switch(opt)
	{
	case 1:
		offline = true;
		socket = INVALID_SOCKET;
		windowName = "Chess (offline)";
		playerColor = NONE;
		break;
	case 2:
		iResult = startServerAndWaitForClient(&serverSocket, &socket);
		if (iResult == -1) return 1;
		isServer = true;
		windowName = "Chess (server)";
		playerColor = WHITE;
		break;
	case 3:
		char servername[256];
		printf("Server name/IP: ");
		scanf("%255s", servername);
		iResult = startClientAndConnect(servername, &socket);
		if (iResult == -1) return 1;
		windowName = "Chess (client)";
		playerColor = BLACK;
		break;
	case 4:
		return 0;
		break;
	default:
		goto menu;
	}
	char recvbuf[1024];

	ReceiveThreadData recvThrData;
	recvThrData.socket = socket;
	recvThrData.recvbuf = recvbuf;
	recvThrData.recvlen = 0;
	recvThrData.status = NO_THREAD;
	recvThrData.result = 0;

	Game game;
	initGame(&game);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window;
	SDL_Renderer* renderer;
	window = SDL_CreateWindow(windowName, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
	initWindow(renderer, playerColor);
	loadTextures(renderer);

	render(renderer, &game);
	printBoard(&game);
	SDL_Event e;
	bool quit = false;

	while (!quit) {
		if (recvThrData.status == NO_THREAD && playerColor != NONE &&
			game.colorToMove != playerColor)
		{
			startReceiveThread(&recvThrData, 3);
		}
		if (recvThrData.status == THREAD_FINISHED) {
			if (onReceive(&recvThrData, &game) <= 0) quit = true;
		}

		while(SDL_PollEvent(&e)) {
			switch(e.type)
			{
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				onClick(e.button.x, e.button.y, &game, playerColor);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				onMove(e.button.x, e.button.y, &game);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				onRelease(e.button.x, e.button.y, &game, playerColor,
					socket, &quit);
				break;
			case SDL_EVENT_KEY_DOWN:
				onKeyDown(e.key, &game, socket, &quit);
				break;
			case SDL_EVENT_KEY_UP:
				onKeyUp(e.key, &game);
				break;
			case SDL_EVENT_QUIT:
				quit = true;
				return 0;
			}
		}
		render(renderer, &game);
		SDL_Delay(10);
	}

	if (!offline) {
		if (isServer) {
			closeServer(serverSocket, socket);
		}
		else {
			closeClient(socket);
		}
	}

	return 0;

//

/*
	FILE* file = stdin;
	char notation[10];
	while (true) {
		printf("%d. ", game.moveCnt + 1);
		if (game.colorToMove == WHITE)
			printf("White to move: ");
		else
			printf("Black to move: ");
		if (fgets(notation, sizeof(notation), file) == NULL) {
			return 0;
		}
		printf(notation);
		if (strcmp(notation, "undo\n") == 0) {
			undoMove(&game);
			printBoard(&game);
			continue;
		}
		if (strcmp(notation, "redo\n") == 0) {
			redoMove(&game);
			printBoard(&game);
			continue;
		}
		if (strcmp(notation, "ctrl\n") == 0) {
		//	printControlBoard(&game);
			continue;
		}
		if (strcmp(notation, "exit\n") == 0) {
			break;
		}
		Move move = moveByNotation(notation, &game);
		if (move.type != NONE) {
			if (move.type == ENPASSANT) {
				printf("En passant!\n");
			}
			printBoard(&game);
		} else {
			printf("Illegal move!\n");
		}
	}
*/
}
