#include <stdio.h>
#include "chess.h"

unsigned long long generateAndCountMoves(int depth, Game* game, bool verbose) {
	if (depth < 0) return 0;
	if (depth == 0) return 1;
	if (depth == 1 && !verbose) return game->cntLegalMoves;
	unsigned long long cnt = 0;
	unsigned long long res;
	char notation[6] = {0};
	for (int i = 0; i < 16; i++) {
		Piece piece = game->pieces[game->colorToMove][i];
		Position from = piece.pos;
		if (from == NONE) continue;
		bool isProm = isPieceGoingToPromote(piece);
		Bitboard bb = game->legalMovesBB[i];
		if (bb) do {
			Position to = bitScanForward(bb);
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
