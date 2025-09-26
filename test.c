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
			if (isMovePromotion(from, to, game)) {
				PieceType prom;
				for (prom = KNIGHT; prom <= QUEEN; prom++) {
					moveToNotation(from, to, prom, notation);
					if (depth == 1) res = 1;
					else {
						makeMove(from, to, prom, game);
						res = generateAndCountMoves(depth - 1, game, false);
						undoMove(game);
					}
					cnt += res;
					if (verbose) printf("%s %llu\n", notation, res);
				}
				continue;
			}
			moveToNotation(from, to, NONE, notation);
			if (depth == 1) res = 1;
			else {
				makeMove(from, to, NONE, game);
				res = generateAndCountMoves(depth - 1, game, false);
				undoMove(game);
			}
			cnt += res;
			if (verbose) printf("%s %llu\n", notation, res);
		} while (bb &= bb - 1);
	}
	return cnt;
}
