#include "stdio.h"
#include "chess.h"
#include "window.h"
#include "control.h"

#define SCREEN_HEIGHT 1000
#define SCREEN_WIDTH 1000

void clear_stdin() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

int main(int argc, char** argv) {
	FILE* file;
	if (argc == 2) {
		file = fopen(argv[1], "r");
	} else {
		file = stdin;
	}

	Game game;
	initGame(&game);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window;
	SDL_Renderer* renderer;
	window = SDL_CreateWindow("Chess", SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
	initWindow(renderer);
	loadTextures(renderer);

	render(renderer, &game);
	printBoard(&game);
	SDL_Event e;
	bool quit = false;

	while (!quit) {
		while(SDL_PollEvent(&e)) {
			switch(e.type)
			{
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				onClick(e.button.x, e.button.y, &game);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				onMove(e.button.x, e.button.y, &game);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				onRelease(e.button.x, e.button.y, &game);
				break;
			case SDL_EVENT_KEY_DOWN:
				onKeyDown(e.key, &game);
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
	return 0;

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
		/*
		if (strcmp(notation, "ctrl\n") == 0) {
			printControlBoard(&game);
			continue;
		}
		*/
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
}
