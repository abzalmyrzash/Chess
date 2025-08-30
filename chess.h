#pragma once
#include <stdbool.h>
#include <stdint.h>

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

#define PIECE(COLOR, TYPE) (COLOR << 3 | TYPE)

typedef int8_t PieceInfo;

typedef struct {
	uint8_t rank;
	uint8_t file;
} Position;

typedef uint8_t CompactPosition;

typedef struct {
	PieceInfo info;
	CompactPosition pos;
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
	CompactPosition from;
	CompactPosition to;
	PieceRef captured;
	MoveType type;
} Move;

typedef struct {
	Piece pieces[2][16];
	PieceRef board[8][8];
	uint8_t controlBoard[8][8][2];
	uint16_t moveCnt;
	uint16_t totalMoves;
	PieceColor colorToMove;
	int8_t enPassantFile;
	uint16_t whenLostCR[2][2]; // when each king lost castling rights to each side
	Move history[MAX_GAME_LENGTH];
} GameState;

typedef enum : int8_t {
	SUBTRACT_CONTROL = -1,
	ADD_CONTROL = 1
} UpdateControlOption;

static const uint8_t knightMoves[8][2] = {
	{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
	{1, 2}, {1, -2}, {-1, 2}, {-1, -2}
};

static const uint8_t kingMoves[8][2] = {
	{1, -1},  {1, 0},  {1, 1},
	{0, -1},           {0, 1},
	{-1, -1}, {-1, 0}, {-1, 1},
};

void initGame(GameState* game);

void printBoard(const GameState* game);

void printBoardFlipped(const GameState* game);

void printControlBoard(const GameState* game);

CompactPosition compPos(Position pos);

Position getPos(CompactPosition pos);

bool isPosValid(Position pos);

bool isPosEqual(Position p1, Position p2);

Position notationToPos(char* notation);

void posToNotation(Position pos, char* notation);

void createPiece(PieceColor color, PieceType type,
	uint8_t rank, uint8_t file, GameState* game);

bool isPosEmpty(Position pos, const GameState* game);

bool isPosEmptyOrInvalid(Position pos, const GameState* game);

Piece getPiece(Position pos, const GameState* game);

static Piece* getPiecePtr(Position pos, GameState* game);

Piece getPieceByRef(PieceRef ref, const GameState* game);

static Piece* getPiecePtrByRef(PieceRef ref, GameState* game);

PieceColor getPieceColor(PieceInfo p);

PieceColor getEnemyColor(PieceColor color);

PieceType getPieceType(PieceInfo p);

PieceInfo getPieceInfo(PieceColor color, PieceType type);

char getPieceSymbol(PieceInfo p);

PieceInfo symbolToPiece(char c);

void movePiece(Position from, Position to, GameState* game);

PieceRef capturePiece(Position pos, GameState* game);

void undoCapture(Position pos, PieceRef piece, GameState* game);

PieceRef enPassant(Position from, Position to, GameState* game);

void undoEnPassant(Position from, Position to, PieceRef captured, GameState* game);

void castle(Position from, Position to, GameState* game);

void undoCastle(Position from, Position to, GameState* game);

void changeType(Position pos, PieceType type, GameState* game);

// function assumes move is legal, so check move legality before calling
Move makeMove(Position from, Position to, PieceType promotion, GameState* game);

MoveType getMoveType(Position from, Position to, GameState* game);

void takeCastlingRights(PieceColor c, uint8_t side, GameState* game);

void updateCastlingRights(Position pos, GameState* game);

bool isMovePromotion(Position from, Position to, const GameState* game);

bool isPromotionValid(Position from, Position to, PieceType prom,
	const GameState* game);

bool isMoveLegal(Position from, Position to, const GameState* game);

bool isMoveGenerallyValid(Position from, Position to, const GameState* game);

bool isWhitePawnMoveLegal(Position from, Position to, const GameState* game);
bool isBlackPawnMoveLegal(Position from, Position to, const GameState* game);
bool isPawnMoveLegal(Position from, Position to, const GameState* game);
bool isKnightMoveLegal(Position from, Position to);
bool isBishopMoveLegal(Position from, Position to, const GameState* game);
bool isRookMoveLegal(Position from, Position to, const GameState* game);
bool isQueenMoveLegal(Position from, Position to, const GameState* game);
bool isKingMoveLegal(PieceColor color, Position from, Position to,
	const GameState* game);

void undoMove(GameState* game);

void redoMove(GameState* game);

Move moveByNotation(char *notation, GameState* game);

Move checkAndMove(Position from, Position to, PieceType prom,
	GameState* game);

int getLegalMoves(Position from, const GameState* game, Position legalTo[MAX_LEGAL_MOVES]);

bool isUnderCheck(PieceColor color, const GameState* game);

bool isPosUnderAttack(Position target, PieceColor color, const GameState* game);

bool isMated(PieceColor color, const GameState* game);

bool hasLegalMoves(PieceColor color, const GameState* game);

bool isPieceAttacking(Position from, Position to, const GameState* game);

/*
void calculateControlBoard(GameState* game);

// get pieces that control pos from a far (bishops, rooks, queens)
int getLongAttackers(Position pos, GameState* game, PieceRef attackers[]);

// get long attackers affected by move
int getAffectedAttackers(MoveInfo* move, GameState* game,
	PieceRef attackers[]);

void updatePieces(MoveInfo* move, GameState* game);

void updateControl(PieceInfo pieces, int n, GameState* game,
	UpdateControlOption opt);
*/
