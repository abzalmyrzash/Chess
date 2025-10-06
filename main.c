#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chess.h"
#include "window.h"
#include "control.h"
#include "net/network.h"
#include "net/server.h"
#include "net/client.h"
#include "test.h"
#include "engine.h"

#define SCREEN_HEIGHT 1000
#define SCREEN_WIDTH 1000

#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void clear_stdin() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

void inputFEN(char* fenBuffer, int fenLen, char** FEN) {
	printf("Enter FEN (leave empty for default board position): ");
	fgets(fenBuffer, fenLen, stdin);
	fenBuffer[strcspn(fenBuffer, "\n")] = '\0';
	if (strlen(fenBuffer) == 0) *FEN = DEFAULT_FEN;
	else *FEN = fenBuffer;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		int depth;
		sscanf(argv[1], "%d", &depth);
		Game game;
		initGameFEN(&game, argv[2]);
		calculateGame(&game);
		int i = 0;
		if (argc > 3) {
			char* str = argv[3];
			while (*str != '\0') {
				moveByNotation(str, &game);
				while (*str != ' ') str++;
				if (*str == '\0') break;
				while (*str == ' ') str++;
			}
		}
		printf("\n%llu", generateAndCountMoves(depth, &game, true));
		
		return 0;
	}

	srand(time(NULL));

	SOCKET serverSocket, socket;
	int iResult;
	bool offline = false;
	bool isServer = false;
	bool vsComp = false;
	char* windowName;
	PieceColor playerColor;
	char* FEN;
	const int fenLen = 100;
	char fenBuffer[fenLen];
	Game game;

menu:
	printf("1. Play hotseat\n");
	printf("2. Play vs computer\n");
	printf("3. Start server\n");
	printf("4. Connect\n");
	printf("5. Test move generation\n");
	printf("6. Quit\n");
	printf("Type option number: ");
	int opt;
	if (scanf("%d", &opt) != 1) {
		clear_stdin();
		goto menu;
	}
	clear_stdin();

	switch(opt)
	{
	case 1:
		offline = true;
		vsComp = false;
		socket = INVALID_SOCKET;
		windowName = "Chess (hotseat)";
		playerColor = NONE;
		break;

	case 2:
		offline = true;
		vsComp = true;
		socket = INVALID_SOCKET;
		printf("Choose color\n");
		printf("(W for White, B for Black, any other symbol for random): ");
		char colorChar;
		scanf("%c", &colorChar);
		clear_stdin();

		switch(colorChar)
		{
		case 'W':
		case 'w':
			playerColor = WHITE;
			break;
		case 'B':
		case 'b':
			playerColor = BLACK;
			break;
		default:
			playerColor = rand() % 2;
			break;
		}

		if (playerColor == WHITE) {
			printf("Selected White\n");
		} else {
			printf("Selected Black\n");
		}
		windowName = "Chess (vs computer)";
		break;

	case 3:
		printf("Choose color\n");
		printf("(W for White, B for Black, any other symbol for random): ");
		scanf("%c", &colorChar);
		clear_stdin();

		switch(colorChar)
		{
		case 'W':
		case 'w':
			playerColor = WHITE;
			break;
		case 'B':
		case 'b':
			playerColor = BLACK;
			break;
		default:
			playerColor = rand() % 2;
			break;
		}

		if (playerColor == WHITE) {
			printf("Selected White\n");
		} else {
			printf("Selected Black\n");
		}

		iResult = startServerAndWaitForClient(&serverSocket, &socket);
		if (iResult == -1) return 1;
		isServer = true;
		windowName = "Chess (server)";

		send(socket, (char*)&playerColor, 1, 0);
		break;

	case 4:
		char servername[256];
		printf("Server name/IP: ");
		scanf("%255s", servername);
		iResult = startClientAndConnect(servername, &socket);
		if (iResult == -1) return 1;
		windowName = "Chess (client)";

		iResult = recv(socket, (char*)&playerColor, 1, 0);
		if (iResult > 0) {
			playerColor = !playerColor;
			if (playerColor == WHITE) {
				printf("Selected White\n");
			} else {
				printf("Selected Black\n");
			}
		}
		else if (iResult == 0) {
			printf("Server disconnected\n");
			return 1;
		} else {
			printf("Receive playerColor error\n");
			return 1;
		}

		break;

	case 5:
		int depth;
		inputFEN(fenBuffer, fenLen, &FEN);
		initGameFEN(&game, FEN);
		calculateGame(&game);
		printBoard(&game);
		for (PieceColor color = WHITE; color <= BLACK; color++) {
			printf("%d\n", game.cntPieces[color]);
			for (int i = 0; i < 16; i++) {
				printf("%d - %d - %d\n", i, game.pieces[color][i].info, game.pieces[color][i].pos);
			}
		}
		printf("Castling rights: ");
		if (game.whenLostCR[WHITE][KINGSIDE] == NEVER) printf("K");
		if (game.whenLostCR[WHITE][QUEENSIDE] == NEVER) printf("Q");
		if (game.whenLostCR[BLACK][KINGSIDE] == NEVER) printf("k");
		if (game.whenLostCR[BLACK][QUEENSIDE] == NEVER) printf("q");
		printf("\n");
		printf("Move cnt: %d\n", game.moveCnt);
		printf("Last pawn move or last capture: %d\n", peekPawnOrCap(&game));
		printf("En passant file: %d\n", game.enPassantFile);
		printf("FEN: %s\n", FEN);
		char outFEN[100];
		gameToFEN(&game, outFEN);
		printf("FEN: %s\n", outFEN);

		while (1) {
			printf("Depth: ");
			scanf("%d", &depth);
			if (depth < 0) return 0;
			clock_t start = clock();
			uint64_t result = generateAndCountMoves(depth, &game, false);
			float time = (float)(clock() - start) / CLOCKS_PER_SEC;
			printf("Generated %llu moves in %f seconds\n", result, time);
		}

		break;

	case 6:
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

	if (offline || isServer) {
		inputFEN(fenBuffer, fenLen, &FEN);
	}

	initGameFEN(&game, FEN);
	calculateGame(&game);

	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	SDL_Window* window;
	SDL_Renderer* renderer;
	window = SDL_CreateWindow(windowName, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	SDL_RaiseWindow(window);
	renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
	loadFont();
	loadTextures(renderer);
	initWindow(renderer, playerColor);
	generateTextures(renderer);

	render(renderer, &game);
	SDL_Event e;
	bool quit = false;

	while (!quit) {
		if (!offline) {
			if (recvThrData.status == NO_THREAD && 
				game.colorToMove != playerColor)
			{
				startReceiveThread(&recvThrData, 3);
			}
			else if (recvThrData.status == THREAD_FINISHED) {
				if (onReceive(&recvThrData, &game) <= 0) quit = true;
			}
		}
		
		if (vsComp && game.state == ONGOING &&
			game.colorToMove != playerColor)
		{
			Move compMove = getBestMove(&game);
			makeMove(compMove.from, compMove.to, compMove.type, &game);
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
				onKeyDown(e.key, &game, playerColor, socket, &quit);
				break;
			case SDL_EVENT_KEY_UP:
				onKeyUp(e.key, &game);
				break;
			case SDL_EVENT_QUIT:
				quit = true;
				break;
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
}
