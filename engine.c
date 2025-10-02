#include "engine.h"

const int pieceValues[5] = {
	100, // PAWN
	300, // KNIGHT
	320, // BISHOP
	500, // ROOK
	900, // QUEEN
};

int eval(Game* game)
{
	int total = 0;
	static int signs[2] = {+1, -1};
	for (PieceColor color = WHITE; color <= BLACK; color++) {
		int val = 0;
		// start from 1 to skip the king (index 0)
		for (int i = 1; i < game->cntPieces[color]; i++) {
			if (game->pieces[color][i].pos != NONE) {
				val += pieceValues[getPieceType(game->pieces[color][i].info)];
			}
		}
		total += signs[color] * val;
	}
	return total;
}
