#include <stdio.h>
#include "chess.h"

unsigned long long generateAndCountMoves(int depth, Game* game, bool verbose) {
	if (depth < 0) return 0;
	if (depth == 0) return 1;
	unsigned long long cnt = 0;
	unsigned long long res;
	char notation[6] = {0};
	for (int i = 0; i < 16; i++) {
		Position from = game->pieces[game->colorToMove][i].pos;
		if (from == NONE) continue;
		Bitboard bb = game->legalMovesBB[game->colorToMove][i];
		if (bb) do {
			Position to = bitScanForward(bb);
			bool isProm = isMovePromotion(from, to, game);
			PieceType prom = NONE;
			if (isProm) {
				for (prom = KNIGHT; prom <= QUEEN; prom++) {
					goto generate;
					promLoop:
				}
				continue;
			}
		generate:
			if (depth == 1) res = 1;
			else {
				makeMove(from, to, prom, game);
				res = generateAndCountMoves(depth - 1, game, false);
				undoMove(game);
			}
			cnt += res;
			if (verbose) {
				moveToNotation(from, to, prom, notation);
				printf("%s %llu\n", notation, res);
			}
			if (isProm) goto promLoop;
		} while (bb &= bb - 1);
	}
	return cnt;
}
