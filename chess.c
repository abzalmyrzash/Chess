#include "chess.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

void initGame(GameState* game)
{
	for (int i = 2; i < 6; i++) {
		for (int j = 0; j < 8; j++) {
			game->board[i][j] = NONE;
		}
	}
	for (int color = 0; color < 2; color++) {
		int cnt = 0;
		createPiece(color, KING, color * 7, 4, game);
		createPiece(color, QUEEN, color * 7, 3, game);
		createPiece(color, BISHOP, color * 7, 2, game);
		createPiece(color, BISHOP, color * 7, 5, game);
		createPiece(color, KNIGHT, color * 7, 1, game);
		createPiece(color, KNIGHT, color * 7, 6, game);
		createPiece(color, ROOK, color * 7, 0, game);
		createPiece(color, ROOK, color * 7, 7, game);
		for (int i = 0; i < 8; i++) {
			createPiece(color, PAWN, color * 5 + 1, i, game);
		}
	}

	game->moveCnt = 0;
	game->colorToMove = WHITE;
	game->enPassantFile = NONE;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			game->whenLostCR[i][j] = NEVER;
		}
	}
	// calculateControlBoard(game);
}

void createPiece(PieceColor color, PieceType type,
	uint8_t rank, uint8_t file, GameState* game)
{
	static uint8_t cnt[2] = { 0, 0 };
	Piece p;
	p.info = getPieceInfo(color, type);
	p.pos = compPos((Position){rank, file});
	game->pieces[color][cnt[color]] = p;
	game->board[rank][file] = color * 16 + cnt[color];
	cnt[color]++;
}

bool isEmpty(Position pos, GameState* game)
{
	return game->board[pos.rank][pos.file] == NONE;
}

Piece getPiece(Position pos, GameState* game)
{
	return *((Piece*)game->pieces + game->board[pos.rank][pos.file]);
}

static Piece* getPiecePtr(Position pos, GameState* game)
{
	return ((Piece*)game->pieces + game->board[pos.rank][pos.file]);
}

PieceColor getPieceColor(PieceInfo p)
{
	return p >> 3;
}

PieceColor getEnemyColor(PieceColor color)
{
	if (color == WHITE)
		return BLACK;
	else
		return WHITE;
}

PieceType getPieceType(PieceInfo p)
{
	return p & 0b111;
}

PieceInfo getPieceInfo(PieceColor color, PieceType type)
{
	return color << 3 | type;
}

Position getPos(CompactPosition pos)
{
	return (Position) { .rank = pos >> 3, .file = pos & 0b111 };
}

CompactPosition compPos(Position pos)
{
	return pos.rank << 3 | pos.file;
}

bool isPosValid(Position pos)
{
	return pos.rank >= 0 && pos.rank < 8 && pos.file >= 0 && pos.file < 8;
}

bool isPosEqual(Position p1, Position p2) {
	return p1.rank == p2.rank && p1.file == p2.rank;
}

Position notationToPos(char* notation)
{
	Position pos;
	pos.file = notation[0] - 'a';
	pos.rank = notation[1] - '1';
	return pos;
}

void posToNotation(Position pos, char* notation)
{
	notation[0] = pos.file + 'a';
	notation[1] = pos.rank + '1';
}

void movePiece(Position from, Position to, GameState* game)
{
	game->board[to.rank][to.file] = game->board[from.rank][from.file];
	game->board[from.rank][from.file] = NONE;
	getPiecePtr(from, game)->pos = compPos(to);
}

PieceRef capturePiece(Position pos, GameState* game)
{
	getPiecePtr(pos, game)->pos = NONE;
	PieceRef piece = game->board[pos.rank][pos.file];
	game->board[pos.rank][pos.file] = NONE;
	return piece;
}

void undoCapture(Position pos, PieceRef piece, GameState* game)
{
	getPiecePtr(pos, game)->pos = compPos(pos);
	game->board[pos.rank][pos.file] = piece;
}

PieceRef enPassant(Position from, Position to, GameState* game)
{
	movePiece(from, to, game);
	return capturePiece((Position){from.rank, to.file}, game);
}

void undoEnPassant(Position from, Position to, PieceRef captured, GameState* game)
{
	movePiece(to, from, game);
	undoCapture((Position){from.rank, to.file}, captured, game);
}

void castle(Position from, Position to, GameState* game)
{
	movePiece(from, to, game);
	Position rook, rookTo;
	rook.rank = from.rank;
	rookTo.rank = to.rank;
	if (to.file == 2) {
		rook.file = 0;
		rookTo.file = 3;
	}
	else {
		rook.file = 7;
		rookTo.file = 5;
	}
	movePiece(rook, rookTo, game);
	if (from.rank == 0) {
		takeCastlingRights(WHITE, 0, game);
		takeCastlingRights(WHITE, 1, game);
	}
	else {
		takeCastlingRights(BLACK, 0, game);
		takeCastlingRights(BLACK, 1, game);
	}
}

void undoCastle(Position from, Position to, GameState* game)
{
	movePiece(to, from, game);
	Position rook, rookTo;
	rook.rank = from.rank;
	rookTo.rank = to.rank;
	PieceColor c;
	uint8_t side;
	if (from.rank == 0) c = WHITE;
	else c = BLACK;
	if (to.file == 2) {
		rook.file = 0;
		rookTo.file = 3;
		side = 0;
	}
	else {
		rook.file = 7;
		rookTo.file = 5;
		side = 1;
	}
	movePiece(rookTo, rook, game);
}

void changeType(Position pos, PieceType type, GameState* game)
{
	Piece* p = getPiecePtr(pos, game);
	p->info += type - getPieceType(p->info);
}

char getPieceSymbol(PieceInfo p)
{
	PieceType type = getPieceType(p);
	PieceColor color = getPieceColor(p);
	switch (type)
	{
	case PAWN:
		return 'P' + 32 * color;
	case KNIGHT:
		return 'N' + 32 * color;
	case BISHOP:
		return 'B' + 32 * color;
	case ROOK:
		return 'R' + 32 * color;
	case QUEEN:
		return 'Q' + 32 * color;
	case KING:
		return 'K' + 32 * color;
	default:
		return '0' + type;
	}
}

PieceInfo symbolToPiece(char c)
{
	PieceColor color;
	if (c >= 'A' && 'Z') {
		color = WHITE;
	} else if (c >= 'a' && 'z') {
		color = BLACK;
		c -= 32; // to uppercase
	} else {
		return NONE;
	}
	switch (c)
	{
	case 'P':
		return getPieceInfo(color, PAWN);
	case 'N':
		return getPieceInfo(color, KNIGHT);
	case 'B':
		return getPieceInfo(color, BISHOP);
	case 'R':
		return getPieceInfo(color, ROOK);
	case 'Q':
		return getPieceInfo(color, QUEEN);
	case 'K':
		return getPieceInfo(color, KING);
	default:
		return NONE;
	}
}

void printBoard(GameState* game)
{
	Position pos;
	printf(" +");
	for (int j = 0; j < 8; j++) {
		printf("-+");
	}
	printf("\n");
	for (int i = 7; i >= 0; i--) {
		pos.rank = i;
		printf("%d|", i + 1);
		for (int j = 0; j < 8; j++) {
			pos.file = j;
			if (isEmpty(pos, game)) printf(" |");
			else printf("%c|", getPieceSymbol(getPiece(pos, game).info));
		}
		printf("\n");
		printf(" +");
		for (int j = 0; j < 8; j++) {
			printf("-+");
		}
		printf("\n");
	}
	printf("  ");
	for (int j = 0; j < 8; j++) {
		printf("%c ", 'a' + j);
	}
	printf("\n");
}

void printBoardFlipped(GameState *game)
{
	Position pos;
	printf(" +");
	for (int j = 0; j < 8; j++) {
		printf("-+");
	}
	printf("\n");
	for (int i = 0; i < 8; i++) {
		pos.rank = i;
		printf("%d|", i + 1);
		for (int j = 7; j >= 0; j--) {
			pos.file = j;
			if (isEmpty(pos, game)) printf(" |");
			else printf("%c|", getPieceSymbol(getPiece(pos, game).info));
		}
		printf("\n");
		printf(" +");
		for (int j = 7; j >= 0; j--) {
			printf("-+");
		}
		printf("\n");
	}
	printf("  ");
	for (int j = 7; j >= 0; j--) {
		printf("%c ", 'a' + j);
	}
	printf("\n");
}

void printControlBoard(GameState *game)
{
	for (int c = 0; c < 2; c++) {
		printf(" +");
		for (int j = 0; j < 8; j++) {
			printf("-+");
		}
		printf("\n");
		for (int i = 7; i >= 0; i--) {
			printf("%d|", i + 1);
			for (int j = 0; j < 8; j++) {
				printf("%d|", game->controlBoard[i][j][c]);
			}
			printf("\n");
			printf(" +");
			for (int j = 0; j < 8; j++) {
				printf("-+");
			}
			printf("\n");
		}
		printf("  ");
		for (int j = 0; j < 8; j++) {
			printf("%c ", 'a' + j);
		}
		printf("\n");
	}
}

/*
Piece checkAndGetPiece(Position pos, const char board[8][8])
{
	if (isPositionValid(pos) == false)
		return (Piece){.type = TYPE_NONE, .color = COLOR_NONE};
	return getPiece(board[pos.rank][pos.file]);
}
*/

// function assumes move is legal, so check move legality before calling
Move makeMove(Position from, Position to, PieceType promotion, GameState* game)
{
	Move move;
	move.from = compPos(from);
	move.to = compPos(to);
	move.type = getMoveType(from, to, game);
	move.captured = NONE;

	game->enPassantFile = NONE;
	switch (move.type)
	{
	case REGMOVE:
		if (isEmpty(to, game) == false) {
			move.captured = capturePiece(to, game);
		}
		movePiece(from, to, game);
		if (promotion != NONE) {
			printf("Promoted to %d!\n", promotion);
			changeType(to, promotion, game);
			move.type = promotion;
		}
		break;
	case DOUBLESTEP:
		movePiece(from, to, game);
		game->enPassantFile = from.file;
		break;
	case ENPASSANT:
		move.captured = enPassant(from, to, game);
		break;
	case CASTLE:
		castle(from, to, game);
		break;
	default:
		break;
	}

	updateCastlingRights(from, game);
	game->colorToMove = getEnemyColor(game->colorToMove);
	game->history[game->moveCnt++] = move;
	game->totalMoves = game->moveCnt;

	return move;
}

void undoMove(GameState* game)
{
	if (game->moveCnt == 0) return;
	Move move = game->history[--game->moveCnt];
	Position from = getPos(move.from);
	Position to = getPos(move.to);
	switch(move.type)
	{
	case REGMOVE:
		movePiece(to, from, game);
		if (move.captured != NONE) {
			undoCapture(to, move.captured, game);
		}
		break;
	case DOUBLESTEP:
		movePiece(to, from, game);
		break;
	case ENPASSANT:
		undoEnPassant(from, to, move.captured, game);
		break;
	case CASTLE:
		undoCastle(from, to, game);
		break;
	default: // if PROMOTION
		changeType(to, PAWN, game);
		movePiece(to, from, game);
		break;
	}
	game->colorToMove = getEnemyColor(game->colorToMove);
}

void redoMove(GameState* game)
{
	if (game->moveCnt == game->totalMoves) return;
	Move move = game->history[game->moveCnt];
	Position from = getPos(move.from);
	Position to = getPos(move.to);

	game->enPassantFile = NONE;
	switch(move.type)
	{
	case REGMOVE:
		if (move.captured != NONE) {
			capturePiece(to, game);
		}
		movePiece(from, to, game);
		break;
	case DOUBLESTEP:
		movePiece(from, to, game);
		game->enPassantFile = from.file;
		break;
	case ENPASSANT:
		enPassant(from, to, game);
		break;
	case CASTLE:
		castle(from, to, game);
		break;
	default: // if PROMOTION
		movePiece(from, to, game);
		changeType(to, move.type, game);
		break;
	}
	game->colorToMove = getEnemyColor(game->colorToMove);
	game->moveCnt++;
}

MoveType getMoveType(Position from, Position to, GameState* game)
{
	PieceInfo p = getPiece(from, game).info;
	switch(getPieceType(p))
	{
	case PAWN:
		if (abs(from.rank - to.rank) == 2)
			return DOUBLESTEP;
		if (game->enPassantFile != to.file) break;
		if (from.rank == 4 && to.rank == 5) {
			return ENPASSANT;
		}
		if (from.rank == 3 && to.rank == 2) {
			return ENPASSANT;
		}

	case KING:
		if (abs(from.file - to.file) == 2)
			return CASTLE;
		break;
	}
	return REGMOVE;
}

void takeCastlingRights(PieceColor c, uint8_t side, GameState* game)
{
	if (game->whenLostCR[c][side] > game->moveCnt)
		game->whenLostCR[c][side] = game->moveCnt;
}

void updateCastlingRights(Position pos, GameState* game)
{
	PieceColor c;
	if (pos.rank == 0) c = WHITE;
	else if (pos.rank == 7) c = BLACK;
	else return;
	if (pos.file == 0) { // left rook
		takeCastlingRights(c, 0, game);
	} else if (pos.file == 7) { // right rook
		takeCastlingRights(c, 1, game);
	} else if (pos.file == 4) { // king
		takeCastlingRights(c, 0, game);
		takeCastlingRights(c, 1, game);
	}
}

Move moveByNotation(char *notation, GameState* game)
{
	Position from = notationToPos(notation);
	Position to = notationToPos(notation + 2);
	PieceInfo piece = symbolToPiece(notation[4]);
	PieceType prom = piece == NONE ? NONE : getPieceType(piece);
	return makeMove(from, to, prom, game);
}

/*
bool isWhitePawnMoveLegal(Position from, Position to, const GameState* game)
{
	char destination = game->board[to.rank][to.file];
	if (destination == EMPTY) {
		if (from.rank + 1 == to.rank && from.file == to.file) return true;
		if (from.rank == 1 && to.rank == 3 && from.file == to.file) {
			char between = game->board[2][from.file];
			if (between == EMPTY) {
				return true;
			}
		}
	}
	if (from.rank + 1 == to.rank && abs(from.file - to.file) == 1) {
		if (destination != EMPTY) {
			return true;
		}
		if (game->enPassantFile == to.file && from.rank == 4) {
			return true;
		}
	}
	return false;
}

bool isBlackPawnMoveLegal(Position from, Position to, const GameState* game)
{
	char destination = game->board[to.rank][to.file];
	if (destination == EMPTY) {
		if (from.rank - 1 == to.rank && from.file == to.file) return true;
		if (from.rank == 6 && to.rank == 4 && from.file == to.file) {
			char between = game->board[5][from.file];
			if (between == EMPTY) {
				return true;
			}
		}
	}
	if (from.rank - 1 == to.rank && abs(from.file - to.file) == 1) {
		if (destination != EMPTY) {
			return true;
		}
		if (game->enPassantFile == to.file && from.rank == 3) {
			return true;
		}
	}
	return false;
}

bool isKnightMoveLegal(Position from, Position to)
{
	if (abs(from.rank - to.rank) == 2 && abs(from.file - to.file) == 1) return true;
	if (abs(from.rank - to.rank) == 1 && abs(from.file - to.file) == 2) return true;
	return false;
}

bool isBishopMoveLegal(Position from, Position to, const char board[8][8])
{
	if (abs(from.rank - to.rank) != abs(from.file - to.file)) return false;
	int rankdir = from.rank > to.rank ? -1 : 1;
	int filedir = from.file > to.file ? -1 : 1;
	int j = from.file + filedir;
	for (int i = from.rank + rankdir; i != to.rank; i += rankdir) {
		if (board[i][j] != EMPTY) return false;
		j += filedir;
	}
	return true;
}

bool isRookMoveLegal(Position from, Position to, const char board[8][8])
{
	if (from.rank != to.rank && from.file == to.file) {
		int rankdir = from.rank > to.rank ? -1 : 1;
		for (int i = from.rank + rankdir; i != to.rank; i += rankdir) {
			if (board[i][from.file] != EMPTY) return false;
		}
	}
	else if (from.rank == to.rank && from.file != to.file) {
		int filedir = from.file > to.file ? -1 : 1;
		for (int j = from.file + filedir; j != to.file; j += filedir) {
			if (board[from.rank][j] != EMPTY) return false;
		}
	}
	return true;
}

bool isQueenMoveLegal(Position from, Position to, const char board[8][8])
{
	return isBishopMoveLegal(from, to, board) || isRookMoveLegal(from, to, board);
}

bool isKingMoveLegal(Position from, Position to, const char board[8][8])
{
	return abs(from.rank - to.rank) <= 1 && abs(from.file - to.file) <= 1;
}

bool isMoveGenerallyValid(Position from, Position to, const GameState* game)
{
	if (isPositionValid(from) == false || isPositionValid(to) == false)
		return false;

	if (from.rank == to.rank && from.file == to.file) return false;

	char* square1 = getSquare(from, game->board);
	char* square2 = getSquare(to, game->board);

	if (*square1 == EMPTY) {
		return false;
	}

	PieceColor pieceColor = getPieceColor(*square1);
	if (pieceColor != game->colorToMove) {
		printf("Not your piece!\n");
		return false;
	}

	if (*square2 != EMPTY) {
		if (pieceColor == getPieceColor(*square2)) {
			printf("Your other piece is blocking!\n");
			return false;
		}
	}

	return true;
}

bool isMoveLegal(Position from, Position to, const GameState* game)
{
	if (isMoveGenerallyValid(from, to, game) == false) return false;
	Piece piece = getPiece(game->board[from.rank][from.file]);

	switch (piece.type)
	{
	case TYPE_NONE:
		return false;
	case PAWN:
		if (piece.color == WHITE)
			return isWhitePawnMoveLegal(from, to, game);
		else
			return isBlackPawnMoveLegal(from, to, game);
	case KNIGHT:
		return isKnightMoveLegal(from, to);
	case BISHOP:
		return isBishopMoveLegal(from, to, game->board);
	case ROOK:
		return isRookMoveLegal(from, to, game->board);
	case QUEEN:
		return isQueenMoveLegal(from, to, game->board);
	case KING:
		return isKingMoveLegal(from, to, game->board);
	default:
		return false;
	}
}

MoveInfo checkAndMove(Position from, Position to, GameState* game)
{
	MoveInfo move;
	if (isMoveLegal(from, to, game) == false) {
		move.isLegal = false;
		return move;
	}

	PieceColor colorThatMoved = game->colorToMove;

	move = makeMove(from, to, game);

	PieceInfo king = getKingInfo(colorThatMoved, game->board);
	if (isUnderCheck(king, game->board)) {
		undoMove(&move, game);
		move.isLegal = false;
	}

	return move;
}

int getLegalMoves(Position from, GameState* game, Position legalTo[MAX_LEGAL_MOVES])
{
	
}

bool isUnderCheck(PieceInfo king, const char board[8][8])
{
	PieceColor enemyColor = getEnemyColor(king.piece.color);
	Piece piece;
	Position pos;

	// check for pawns
	if (king.piece.color == WHITE) pos.rank = king.pos.rank + 1;
	else pos.rank = king.pos.rank - 1;
	for (int i = -1; i <= 1; i += 2) {
		pos.file = king.pos.file + i;
		piece = checkAndGetPiece(pos, board);
		if (piece.color == enemyColor && piece.type == PAWN) {
			return true;
		}
	}

	// check for knights
	for (int i = 0; i < 8; i++) {
		pos.rank = king.pos.rank + knightMoves[i][0];
		pos.file = king.pos.file + knightMoves[i][1];
		piece = checkAndGetPiece(pos, board);
		if (piece.color == enemyColor && piece.type == KNIGHT) {
			return true;
		}
	}

	// check diagonals for bishops and queens
	for (int i = -1; i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			for (int step = 1; step < 8; step++) {
				pos.rank = king.pos.rank + i * step;
				pos.file = king.pos.file + j * step;
				if (isPositionValid(pos) == false) break;
				piece = getPieceByPos(pos, board);
				if (piece.color == enemyColor) {
					if (piece.type == BISHOP || piece.type == QUEEN) {
						return true;
					}
					else break;
				}
				else {
					break;
				}
			}
		}
	}

	// check rank and file for rooks and queens
	pos.rank = king.pos.rank;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.file = king.pos.file + i * step;
			if (isPositionValid(pos) == false) break;
			piece = getPieceByPos(pos, board);
			if (piece.color == enemyColor) {
				if (piece.type == ROOK || piece.type == QUEEN) {
					return true;
				}
				else break;
			}
			else {
				break;
			}
		}
	}
	pos.file = king.pos.file;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.rank = king.pos.rank + i * step;
			if (isPositionValid(pos) == false) break;
			piece = getPieceByPos(pos, board);
			if (piece.color == enemyColor) {
				if (piece.type == ROOK || piece.type == QUEEN) {
					return true;
				}
				else break;
			}
			else {
				break;
			}
		}
	}

	return false;
}

bool isPieceAttacking(Position from, Position to, const char board[8][8]) {
	Piece piece = getPiece(*getSquare(from, board));
	switch(piece.type)
	{
	case TYPE_NONE:
		return false;
	case PAWN:
		if (piece.color == WHITE) {
			return from.rank + 1 == to.rank && abs(from.file - to.file) == 1;
		}
		else {
			return from.rank - 1 == to.rank && abs(from.file - to.file) == 1;
		}
		return false;
	case KNIGHT:
		return isKnightMoveLegal(from, to);
	case BISHOP:
		return isBishopMoveLegal(from, to, board);
	case ROOK:
		return isRookMoveLegal(from, to, board);
	case QUEEN:
		return isQueenMoveLegal(from, to, board);
	case KING:
		return isKingMoveLegal(from, to, board);
	default:
		return false;
	}
}

int getPawnAttackMoves(Position from, PieceColor color,
		const char board[8][8], Position moves[])
{
	Position pos;
	int cnt = 0;
	if (color == WHITE) pos.rank = from.rank + 1;
	else pos.rank = from.rank - 1;
	for (int i = -1; i <= 1; i += 2) {
		pos.file = from.file + i;
		if (isPositionValid(pos)) moves[cnt++] = pos;
	}
	return cnt;
}

int getKnightMoves(Position from, const char board[8][8], Position moves[])
{
	Position pos;
	int cnt = 0;
	for (int i = 0; i < 8; i++) {
		pos.rank = from.rank + knightMoves[i][0];
		pos.file = from.file + knightMoves[i][1];
		if (isPositionValid(pos)) moves[cnt++] = pos;
	}
	return cnt;
}

int getBishopMoves(Position from, const char board[8][8], Position moves[])
{
	Piece piece;
	Position pos;
	int cnt = 0;
	for (int i = -1; i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			for (int step = 1; step < 8; step++) {
				pos.rank = from.rank + i * step;
				pos.file = from.file + j * step;
				if (isPositionValid(pos) == false) break;
				if (*getSquare(pos, board) == EMPTY) {
					moves[cnt++] = pos;
				} else {
					moves[cnt++] = pos;
					break;
				}
			}
		}
	}
	return cnt;
}

int getRookMoves(Position from, const char board[8][8], Position moves[])
{
	Piece piece;
	Position pos;
	int cnt = 0;
	pos.rank = from.rank;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.file = from.file + i * step;
			if (isPositionValid(pos) == false) break;
			if (*getSquare(pos, board) == EMPTY) {
				moves[cnt++] = pos;
			} else {
				moves[cnt++] = pos;
				break;
			}
		}
	}
	pos.file = from.file;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.rank = from.rank + i * step;
			if (isPositionValid(pos) == false) break;
			if (*getSquare(pos, board) == EMPTY) {
				moves[cnt++] = pos;
			} else {
				moves[cnt++] = pos;
				break;
			}
		}
	}
	return cnt;
}

int getQueenMoves(Position from, const char board[8][8], Position moves[])
{
	int nBishopMoves = getBishopMoves(from, board, moves);
	int nRookMoves = getRookMoves(from, board, moves + nBishopMoves);
	return nBishopMoves + nRookMoves;
}

int getKingMoves(Position from, const char board[8][8], Position moves[])
{
	Position pos;
	int cnt = 0;
	for (int i = 0; i < 8; i++) {
		pos.rank = from.rank + kingMoves[i][0];
		pos.file = from.file + kingMoves[i][1];
		if (isPositionValid(pos)) moves[cnt++] = pos;
	}
	return cnt;
}

int getControlledPositions(PieceInfo piece, const char board[8][8], Position controlled[])
{
	switch(piece.piece.type)
	{
	case TYPE_NONE:
		return 0;

	case PAWN:
		return getPawnAttackMoves(piece.pos, piece.piece.color, board, controlled);

	case KNIGHT:
		return getKnightMoves(piece.pos, board, controlled);

	case BISHOP:
		return getBishopMoves(piece.pos, board, controlled);

	case ROOK:
		return getRookMoves(piece.pos, board, controlled);

	case QUEEN:
		return getQueenMoves(piece.pos, board, controlled);

	case KING:
		return getKingMoves(piece.pos, board, controlled);

	default:
		return 0;
	}
}

PieceInfo getKingInfo(PieceColor color, const char board[8][8])
{
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			Piece piece = getPiece(board[i][j]);
			if (piece.color == color)
				return (PieceInfo){piece, (Position){.rank=i, .file=j}};
		}
	}
}

int getPiecesByColor(PieceColor color, const char board[8][8], PieceInfo pieces[16])
{
	int cnt = 0;
	Piece piece;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			piece = getPiece(board[i][j]);
			if (piece.color == color) {
				pieces[cnt++] = (PieceInfo){piece, (Position){.rank=i, .file=j}};
			}
		}
	}
	return cnt;
}

int updatePieces(Move* move, PieceInfo pieces[2][16]) {
	PieceColor color = getPieceColor(move->piece);
	PieceColor enemyColor = getEnemyColor(color);
	for (int i = 0; i < nPieces[color]; i++) {
		Piece* p = &pieces[color][i];
		if (isPositionEqual(p->pos, move->from)) {
			p->pos = move->to;
		}
	}
	Position capPos;
	if (move->isEnPassant) {
		capPos.rank = move->to.rank;
		capPos.file = move->from.file;
	} else capPos = move->to;

	for (int i = 0; i < nPieces[enemyColor]; i++) {
		Piece* p = &pieces[enemyColor][i];
		if (isPositionEqual(p->pos, capPos)) {
			p->pos.rank = -1;
			p->pos.file = -1;
		}
	}
}

int getIndexToKing(const PieceInfo pieces[16], int n)
{
	for(int i = 0; i < n; i++) {
		if (pieces[i].piece.type == KING) {
			return i;
		}
	}
	return -1;
}

void calculateControlBoard(GameState* game)
{
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int c = 0; c < 2; c++) {
				game->controlBoard[i][j][c] = 0;
			}
		}
	}
	Position pos[MAX_LEGAL_MOVES];
	uint8_t rank, file;
	for (int i = 0; i < 2; i++) {
		printf("%d\n", game->nPieces[i]);
		for (int j = 0; j < game->nPieces[i]; j++) {
			Piece p = game->pieces[i][j];
			if (p.pos == NONE) continue;
			int nPos = getControlledPositions(p, game->board, pos);
//			printf("%d%c @ %c%d - %d\n", p.piece.color, p.piece.type,
//				p.pos.file + 'a', p.pos.rank + 1, nPos);
			for (int k = 0; k < nPos; k++) {
				rank = pos[k].rank;
				file = pos[k].file;
				game->controlBoard[rank][file][i] += 1;
			}
		}
	}
}

void updateControl(PieceInfo pieces, int n, GameState* game,
	UpdateControlOption opt)
{
	for (int i = 0; i < n; i++) {
		int nPos = getControlledPositions(p, game->board, pos);
		pieces[];
	}
}

int getLongAttackers(Position target, const char board[8][8], PieceInfo attackers[])
{
	int cnt = 0;
	Piece piece;
	Position pos;

	for (int i = -1; i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			for (int step = 1; step < 8; step++) {
				pos.rank = target.rank + i * step;
				pos.file = target.file + j * step;
				if (isPositionValid(pos) == false) break;
				piece = getPiece(*getSquare(pos, board));
				if (piece.type == BISHOP || piece.type == QUEEN) {
					attackers[cnt++] = (PieceInfo){piece, pos};
					break;
				}
			}
		}
	}

	pos.rank = target.rank;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.file = target.file + i * step;
			if (isPositionValid(pos) == false) break;
			piece = getPiece(*getSquare(pos, board));
			if (piece.type == ROOK || piece.type == QUEEN) {
				attackers[cnt++] = (PieceInfo){piece, pos};
				break;
			}
		}
	}

	pos.file = target.file;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.rank = target.rank + i * step;
			if (isPositionValid(pos) == false) break;
			piece = getPiece(*getSquare(pos, board));
			if (piece.type == ROOK || piece.type == QUEEN) {
				attackers[cnt++] = (PieceInfo){piece, pos};
				break;
			}
		}
	}
	return cnt;
}

int getAffectedAttackers(MoveInfo* move, const char board[8][8],
	PieceInfo attackers[]);
{
	int cnt = 0;
	Piece piece;
	Position pos1, pos2;

	for (int i = -1; i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			for (int step = 1; step < 8; step++) {
				pos1.rank = move->from.rank + i * step;
				pos1.file = move->from.file + j * step;
				pos2.rank = move->to.rank + i * step;
				pos2.file = move->to.file + j * step;
				if (isPositionValid(pos) == false) break;
				if (isPositionEqual(pos, move->to)) {
					break;
				}
				piece = getPiece(*getSquare(pos, board));
				if (piece.type == BISHOP || piece.type == QUEEN) {
					attackers[cnt++] = (PieceInfo){piece, pos};
					break;
				}
			}
		}
	}

	pos.rank = move->from.rank;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.file = move->from.file + i * step;
			if (isPositionValid(pos) == false) break;
			if (isPositionEqual(pos, move->to)) {
				break;
			}
			piece = getPiece(*getSquare(pos, board));
			if (piece.type == ROOK || piece.type == QUEEN) {
				move->affectedAttackers[cnt++] = (PieceInfo){piece, pos};
				break;
			}
		}
	}

	pos.file = move->from.file;
	for (int i = -1; i <= 1; i += 2) {
		for (int step = 1; step < 8; step++) {
			pos.rank = move->from.rank + i * step;
			if (isPositionValid(pos) == false) break;
			if (isPositionEqual(pos, move->to)) {
				break;
			}
			piece = getPiece(*getSquare(pos, board));
			if (piece.type == ROOK || piece.type == QUEEN) {
				move->affectedAttackers[cnt++] = (PieceInfo){piece, pos};
				break;
			}
		}
	}
	return cnt;
}
*/
