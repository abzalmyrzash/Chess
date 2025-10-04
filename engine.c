#include "engine.h"
#include "limits.h"
#include <stdio.h>

static const int pieceValues[5] = {
	100, // PAWN
	300, // KNIGHT
	320, // BISHOP
	500, // ROOK
	900, // QUEEN
};

int eval(Game* game)
{
	int total = 0;
	static const int signs[2] = {+1, -1};
	for (PieceColor color = WHITE; color <= BLACK; color++) {
		int val = 0;
		// start from 1 to skip the king (who is index 0)
		for (int i = 1; i < game->cntPieces[color]; i++) {
			if (game->pieces[color][i].pos != NONE) {
				val += pieceValues[getPieceType(game->pieces[color][i].info)];
			}
		}
		total += signs[color] * val;
	}
	return total;
}

Move getBestMove(Game* game)
{
	PieceColor color = game->colorToMove;
	int sign = (color == WHITE ? 1 : -1);
	int max = INT_MIN, val;
	Move bestMove, move;
	for (int i = 0; i < game->cntPieces[color]; i++) {
		Position from = game->pieces[color][i].pos;
		Bitboard bb = game->legalMovesBB[color][i];
		if (bb) do {
			Position to = bitScanForward(bb);
			if (isMovePromotion(from, to, game)) {
				for (PieceType prom = KNIGHT; prom <= QUEEN; prom++) {
					move = makeMove(from, to, prom, game);
					val = eval(game) * sign;
					if (val > max) {
						max = val;
						bestMove = move;
					}
					undoMove(game);
				}
			}
			else {
				move = makeMove(from, to, NONE, game);
				val = eval(game) * sign;
				if (val > max) {
					max = val;
					bestMove = move;
				}
				undoMove(game);
			}
		} while (bb &= bb - 1);
	}
	return bestMove;
}
