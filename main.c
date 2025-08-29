#include "SDL3/SDL.h"
#include "chess.h"
#include "stdio.h"

#define SCREEN_HEIGHT 1000
#define SCREEN_WIDTH 1000

void clear_stdin() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

int main(int argc, char* argv[]) {
	FILE* file;
	if (argc == 2) {
		file = fopen(argv[1], "r");
	} else {
		file = stdin;
	}
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window;
	SDL_Renderer* renderer;
//	window = SDL_CreateWindow("Chess", SCREEN_HEIGHT, SCREEN_WIDTH, 0);
//	renderer = SDL_GetRenderer(window);

	GameState game;
	initGame(&game);
	printBoard(&game);
	char notation[10];
	while (true) {
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
			printControlBoard(&game);
			continue;
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
