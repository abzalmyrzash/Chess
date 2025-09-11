#include "bitboard.h"
#include <stdio.h>
#include <assert.h>

uint8_t popCount (Bitboard bb) {
	uint8_t count = 0;
	while (bb) {
		count++;
		bb &= bb - 1; // reset LS1B
	}
	return count;
}

const uint8_t debruijnIndex64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

/**
 * bitScanForward
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
uint8_t bitScanForward(Bitboard bb) {
	const U64 debruijn64 = C64(0x03f79d71b4cb0a89);
	assert (bb != 0);
	return debruijnIndex64[((bb & -bb) * debruijn64) >> 58];
}

Bitboard PAWN_MOVES_BB[64];
Bitboard PAWN_ATTACKS_BB[64];
Bitboard KNIGHT_MOVES_BB[64];
Bitboard BISHOP_MOVES_BB[64];
Bitboard ROOK_MOVES_BB[64];
Bitboard QUEEN_MOVES_BB[64];
Bitboard KING_MOVES_BB[64];

void initBitboards(Bitboard bb)
{
	uint8_t pos;
	for (int i = 0; i < 64; i++) {
	}
}

void printBitboard(Bitboard bb)
{
	printf(" +");
	for (int j = 0; j < 8; j++) {
		printf("-+");
	}
	printf("\n");
	for (int i = 7; i >= 0; i--) {
		printf("%d|", i + 1);
		for (int j = 0; j < 8; j++) {
			printf("%d|", (bb & (1ULL << (i << 3 | j))) > 0);
		}
		printf("\n");
		printf(" +");
		for (int j = 0; j < 8; j++) {
			printf("-+");
		}
		printf("\n");
	}
	printf(" ");
	for (int j = 0; j < 8; j++) {
		printf("%c ", 'a' + j);
	}
	printf("\n");
}

U64 genShift(Bitboard bb, int s)
{
	return (s < 0) ? (bb << s) : (bb >> -s);
}
