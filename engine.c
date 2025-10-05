#include "engine.h"
#include <limits.h>
#include <stdio.h>

static const int pieceValues[5] = {
	100, // PAWN
	300, // KNIGHT
	320, // BISHOP
	500, // ROOK
	900, // QUEEN
};

static int pos;

static const int positionalModifiers[6][64] = {
	//PAWN
	{
		0,	0,	0,	0,	0,	0,	0,	0,	
		-10,-10,-10,-10,-10,-10,-10,-10,
		-10,-10,0,	0,	0,	0,	-10,-10,
		-10,-10,10,	10,	10,	10,	-10,-10,
		10,	10,	10,	10,	10,	10,	10,	10,
		10,	10,	10,	10,	10,	10,	10,	10,
		10,	10,	10,	10,	10,	10,	10,	10,
		0,	0,	0,	0,	0,	0,	0,	0,	
	},

	// KNIGHT
	{
		-30,-20,-10,-10,-10,-10,-20,-30,
		-20,-10,0,	0,	0,	0,	-10,-20,
		-10,0,	10,	10,	10,	10,	-20,-10,
		-10,0,	10,	10,	10,	10,	-20,-10,
		-10,0,	10,	10,	10,	10,	-20,-10,
		-10,0,	10,	10,	10,	10,	-20,-10,
		-20,-10,0,	0,	0,	0,	-10,-20,
		-30,-20,-10,-10,-10,-10,-20,-30,
	},

	// BISHOP
	{
		
	},

	{
	},
};

int eval(Game* game)
{
	int total = 0;
	static const int signs[2] = {+1, -1};
	for (PieceColor color = WHITE; color <= BLACK; color++) {
		int val = 0;
		// start from 1 to skip the king (who is index 0)
		for (int i = 1; i < game->cntPieces[color]; i++) {
			Position pos = game->pieces[color][i].pos;  
			if (pos != NONE) {
				PieceType type = getPieceType(game->pieces[color][i].info);
				val += pieceValues[type];
				if (type != PAWN || color == WHITE) {
					val += positionalModifiers[type][pos];
				} else {
					val -= positionalModifiers[type][pos];
				}
			}
		}
		val += positionalModifiers[KING][game->pieces[color][0].pos];
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
		Bitboard bb = game->legalMovesBB[i];
		if (bb) do {
			Position to = bitScanForward(bb);
			bool isProm = isMovePromotion(from, to, game);
			PieceType prom = NONE;
			if (isProm) {
				for (prom = KNIGHT; prom <= QUEEN; prom++) {
					goto evalMove;
					promLoop:
				}
				continue;
			}
		evalMove:
			move = makeMove(from, to, NONE, game);
			val = eval(game) * sign;
			if (val > max) {
				max = val;
				bestMove = move;
			}
			undoMove(game);
			if (isProm) goto promLoop;
		} while (bb &= bb - 1);
	}
	return bestMove;
}
