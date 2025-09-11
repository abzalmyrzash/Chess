#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "bitboard.h"

#define NONE -1

#define MAX_GAME_LENGTH 10000
#define NEVER MAX_GAME_LENGTH

#define MAX_LEGAL_MOVES 27
#define MAX_LONG_ATTACKERS 8

typedef enum : int8_t {
	WHITE,
	BLACK,
} PieceColor;

typedef enum : int8_t {
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	NUM_TYPES
} PieceType;

typedef enum : uint8_t {
	QUEENSIDE,
	KINGSIDE
} CastleSide;

#define PIECE(COLOR, TYPE) (COLOR << 3 | TYPE)

typedef int8_t PieceInfo;

typedef int8_t Position;

typedef struct {
	uint8_t rank;
	uint8_t file;
} RankFile;

typedef struct {
	PieceInfo info;
	Position pos;
} Piece;

typedef int8_t PieceRef;

typedef enum : int8_t {
	ILLEGAL = NONE,
	REGMOVE = 0,
	PROMOTION_KNIGHT = KNIGHT,
	PROMOTION_BISHOP = BISHOP,
	PROMOTION_ROOK = ROOK,
	PROMOTION_QUEEN = QUEEN,
	DOUBLESTEP,
	ENPASSANT,
	CASTLE,
} MoveType;

typedef struct {
	Position from;
	Position to;
	PieceRef captured;
	MoveType type;
} Move;

typedef enum {
	ONGOING,
	WHITE_WON,
	BLACK_WON,
	DRAW
} GameState;

typedef struct {
	GameState state;
	uint8_t cntPieces[2];
	Piece pieces[2][16];
	PieceInfo board[64];
	PieceRef refBoard[64];
	Bitboard piecesBB[2]; // bitboard of pieces by color
	Bitboard pieceTypeBB[2][6]; // bitboard of pieces by color and type
	Bitboard attacksBB[2][16]; // squares attacked by each piece
	Bitboard checkersBB[2]; // where the king is being checked from
	uint16_t moveCnt;
	uint16_t totalMoves;
	uint16_t lastPawnOrCapture;
	PieceColor colorToMove;
	int8_t enPassantFile;
	uint16_t whenLostCR[2][2]; // when each king lost castling rights to each side
	
	Move history[MAX_GAME_LENGTH];
} Game;

typedef enum : int8_t {
	SUBTRACT_CONTROL = -1,
	ADD_CONTROL = 1
} UpdateControlOption;

static const uint8_t knightMoves[8] = {
		15,		17,
	6,				10,
			//N
	-10,			-6,
		-17,	-15
};

static const uint8_t kingMoves[8] = {
	7,	8,	9,
	-1,		1,
	-9,	-8,	-7
};

static const uint8_t diagonals[4] = {
	7,		9,
		//B
	-9,		-7
};

static const uint8_t orthogonals[4] = {
		8,
	-1,		1,
		-8
};

void initGame(Game* game);

void printBoard(const Game* game);

void printBoardFlipped(const Game* game);

bool isPosValid(Position pos);

RankFile getRankFile(Position pos);

uint8_t getRank(Position pos);

uint8_t getFile(Position pos);

Position getPos(uint8_t rank, uint8_t file);

Position notationToPos(char* notation);

void posToNotation(Position pos, char* notation);

void createPiece(PieceColor color, PieceType type,
	uint8_t rank, uint8_t file, Game* game);

bool isPosEmpty(Position pos, const Game* game);

bool isPosEmptyOrInvalid(Position pos, const Game* game);

Piece getPiece(Position pos, const Game* game);

static Piece* getPiecePtr(Position pos, Game* game);

Piece getPieceByRef(PieceRef ref, const Game* game);

static Piece* getPiecePtrByRef(PieceRef ref, Game* game);

PieceColor getPieceColor(PieceInfo p);

PieceColor getEnemyColor(PieceColor color);

PieceType getPieceType(PieceInfo p);

PieceInfo getPieceInfo(PieceColor color, PieceType type);

char getPieceSymbol(PieceInfo p);

PieceInfo symbolToPiece(char c);

void movePiece(Position from, Position to, Game* game);

PieceRef capturePiece(Position pos, Game* game);

void undoCapture(Position pos, PieceRef piece, Game* game);

PieceRef enPassant(Position from, Position to, Game* game);

void undoEnPassant(Position from, Position to, PieceRef captured, Game* game);

void castle(Position from, Position to, Game* game);

void undoCastle(Position from, Position to, Game* game);

void changeType(Position pos, PieceType type, Game* game);

// function assumes move is legal, so check move legality before calling
Move makeMove(Position from, Position to, PieceType promotion, Game* game);

MoveType getMoveType(Position from, Position to, Game* game);

void takeCastlingRights(PieceColor c, CastleSide side, Game* game);

void updateCastlingRights(Position pos, Game* game);

bool isMovePromotion(Position from, Position to, const Game* game);

bool isPromotionValid(Position from, Position to, PieceType prom,
	const Game* game);

bool isMoveLegal(Position from, Position to, const Game* game);

bool isMoveGenerallyValid(Position from, Position to, const Game* game);

bool isWhitePawnMoveLegal(Position from, Position to, const Game* game);
bool isBlackPawnMoveLegal(Position from, Position to, const Game* game);
bool isPawnMoveLegal(Position from, Position to, const Game* game);
bool isKnightMoveLegal(Position from, Position to);
bool isBishopMoveLegal(Position from, Position to, const Game* game);
bool isRookMoveLegal(Position from, Position to, const Game* game);
bool isQueenMoveLegal(Position from, Position to, const Game* game);
bool isKingMoveLegal(PieceColor color, Position from, Position to,
	const Game* game);

void undoMove(Game* game);

void redoMove(Game* game);

Move moveByNotation(char *notation, Game* game);

Move checkAndMove(Position from, Position to, PieceType prom,
	Game* game);

Bitboard getAttacks(Position from, const Game* game);

void calculateAllAttacks(Game* game);

Bitboard getAttackers(Position target, PieceColor color, const Game* game);

Bitboard getMoves(Position from, const Game* game);

Bitboard getLegalMoves(Position from, const Game* game);

bool isUnderCheck(PieceColor color, const Game* game);

bool isPosUnderAttack(Position target, PieceColor color, const Game* game);

bool isMated(PieceColor color, const Game* game);

bool hasLegalMoves(PieceColor color, const Game* game);

bool isPieceAttacking(Position from, Position to, const Game* game);

/*
void calculateControlBoard(Game* game);

// get pieces that control pos from a far (bishops, rooks, queens)
int getLongAttackers(Position pos, Game* game, PieceRef attackers[]);

// get long attackers affected by move
int getAffectedAttackers(MoveInfo* move, Game* game,
	PieceRef attackers[]);

void updatePieces(MoveInfo* move, Game* game);

void updateControl(PieceInfo pieces, int n, Game* game,
	UpdateControlOption opt);
*/
