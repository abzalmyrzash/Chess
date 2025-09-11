#pragma once
#include <stdint.h>

typedef unsigned long long U64;
#define C64(constantU64) constantU64##ULL
typedef U64 Bitboard;

void printBitboard(Bitboard bb);

U64 genShift(Bitboard bb, int s);

#define setBit(x, p) (x |= C64(1) << (p))
#define resetBit(x, p) (x &= ~(C64(1) << (p)))
#define toggleBit(x, p) (x ^= C64(1) << (p))
#define testBit(x, p) (x & C64(1) << (p))

uint8_t bitScanForward(Bitboard bb);
uint8_t popCount(Bitboard bb);

extern Bitboard PAWN_MOVES_BB[64];
extern Bitboard PAWN_ATTACKS_BB[64];
extern Bitboard KNIGHT_MOVES_BB[64];
extern Bitboard BISHOP_MOVES_BB[64];
extern Bitboard ROOK_MOVES_BB[64];
extern Bitboard QUEEN_MOVES_BB[64];
extern Bitboard KING_MOVES_BB[64];

void initBitboards();
