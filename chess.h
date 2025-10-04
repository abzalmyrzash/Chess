#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "bitboard.h"

#define NONE -1

#define MAX_GAME_LENGTH 10000 // probably enough
#define NEVER MAX_GAME_LENGTH
#define MAX_PAWN_OR_CAPS 128 // 30 captures + 16 * 6 pawn moves + 2 extra

#define NUM_HALF_MOVES_FOR_DRAW 100

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

#define PIECE(COLOR, TYPE) ((COLOR) << 3 | (TYPE))

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
	PAWNMOVE,
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
	STALEMATE,
	DRAW
} GameState;

typedef struct {
	GameState state;
	uint8_t cntPieces[2];
	Piece pieces[2][16];
	PieceInfo board[64];
	PieceRef refBoard[64];
	Bitboard piecesBB[2]; // bitboard of pieces by color
	Bitboard attacksBB[2][16]; // squares attacked by each individual piece
	Bitboard totalAttacksBB[2]; // squares attacked by color in total
	uint8_t numCheckers[2];
	Position checkerPos[2]; // where the king is being checked from
	Bitboard checkXRayBB[2]; // squares X-Rayed through the king during check
	Bitboard pinnedBB[2];
	Bitboard legalMovesBB[2][16]; // legal moves for each piece
	uint16_t moveCnt;
	uint16_t totalMoves;
	uint16_t minMove;
	PieceColor colorToMove;
	int8_t enPassantFile;
	int8_t ogEnPassantFile;
	uint16_t whenLostCR[2][2]; // when each king lost castling rights to each side
	
	Move history[MAX_GAME_LENGTH];
	uint16_t pawnOrCaps[MAX_PAWN_OR_CAPS];
	uint8_t iPawnOrCap;
} Game;

typedef enum : int8_t {
	SUBTRACT_CONTROL = -1,
	ADD_CONTROL = 1
} UpdateControlOption;

void initGame(Game* game);

void initGameFEN(Game* game, char* FEN);

void gameToFEN(Game* game, char* FEN);

void printBoard(const Game* game);

void printBoardFlipped(const Game* game);

void printMoveHistory(const Game* game);

bool isPosValid(Position pos);

bool isRankFileValid(int rank, int file);

RankFile getRankFile(Position pos);

uint8_t getRank(Position pos);

uint8_t getFile(Position pos);

Position getPos(uint8_t rank, uint8_t file);

Position notationToPos(char* notation);

void posToNotation(Position pos, char* notation);

void moveToNotation(Position from, Position to, PieceType prom, char* notation);

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

void takeCastlingRights(PieceColor c, CastleSide side, Game* game);

void updateCastlingRights(Position from, Position to, Game* game);

void changeType(Position pos, PieceType type, Game* game);

MoveType getMoveType(Position from, Position to, Game* game);

bool isMovePawnOrCap(Move move);

void pushPawnOrCap(Game* game);

void popPawnOrCap(Game* game);

uint16_t peekPawnOrCap(Game* game);

uint8_t getHalfMoveClock(Game* game);

bool isMovePromotion(Position from, Position to, const Game* game);

bool isPromotionValid(Position from, Position to, PieceType prom,
	const Game* game);

// function assumes move is legal, so check move legality before calling
Move makeMove(Position from, Position to, PieceType promotion, Game* game);

bool isMoveLegal(Position from, Position to, const Game* game);

bool isMoveGenerallyValid(Position from, Position to, const Game* game);

void undoMove(Game* game);

void redoMove(Game* game);

Move moveByNotation(char *notation, Game* game);

Move checkAndMove(Position from, Position to, PieceType prom,
	Game* game);

Bitboard getAttacks(Position from, PieceColor color, PieceType type,
	const Game* game);

void calculateAllAttacks(Game* game);

Bitboard getAttackers(Position target, PieceColor color, const Game* game);

Bitboard getCastleMoves(Position from, PieceColor color, const Game* game);

Bitboard getPawnMoves(Position from, PieceColor color, const Game* game);

Bitboard getEnPassantMoves(Position from, PieceColor color, const Game* game);

Bitboard getLegalMoves(Position from, const Game* game);

void calculateAllLegalMoves(Game* game);

void calculateGame(Game* game);

bool isInCheck(PieceColor color, const Game* game);

