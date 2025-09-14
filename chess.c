#include "chess.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

void initGame(Game* game)
{
	Position pos;
	for (int i = 2; i < 6; i++) {
		pos = i << 3;
		for (int j = 0; j < 8; j++) {
			game->refBoard[pos] = NONE;
			game->board[pos] = NONE;
			pos++;
		}
	}

	game->cntPieces[0] = game->cntPieces[1] = 0;
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

	game->state = ONGOING;
	game->moveCnt = 0;
	game->totalMoves = 0;
	game->lastPawnOrCapture = 0;
	game->colorToMove = WHITE;
	game->enPassantFile = NONE;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			game->whenLostCR[i][j] = NEVER;
		}
		game->numCheckers[i] = 0;
	}
	calculateGame(game);
	/*
	for (int i = 0; i < 2; i++) {
		printBitboard(game->piecesBB[i]);
	}
	*/
}

void createPiece(PieceColor color, PieceType type,
	uint8_t rank, uint8_t file, Game* game)
{
	Piece p;
	p.info = getPieceInfo(color, type);
	p.pos = getPos(rank, file);
	game->pieces[color][game->cntPieces[color]] = p;
	game->board[p.pos] = p.info;
	game->refBoard[p.pos] = color * 16 + game->cntPieces[color];
	game->cntPieces[color]++;
	setBit(game->piecesBB[color], p.pos);
}

bool isPosEmpty(Position pos, const Game* game)
{
	return game->refBoard[pos] == NONE;
}

bool isPosEmptyOrInvalid(Position pos, const Game* game)
{
	return !isPosValid(pos) || isPosEmpty(pos, game);
}

Piece getPiece(Position pos, const Game* game)
{
	assert(isPosValid(pos) && !isPosEmpty(pos, game));
	return *((Piece*)game->pieces + game->refBoard[pos]);
}

static Piece* getPiecePtr(Position pos, Game* game)
{
	assert(isPosValid(pos) && !isPosEmpty(pos, game));
	return ((Piece*)game->pieces + game->refBoard[pos]);
}

Piece getPieceByRef(PieceRef ref, const Game* game)
{
	assert(ref >= 0 && ref < 32);
	return *((Piece*)game->pieces + ref);
}

static Piece* getPiecePtrByRef(PieceRef ref, Game* game)
{
	assert(ref >= 0 && ref < 32);
	return ((Piece*)game->pieces + ref);
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

bool isPosValid(Position pos)
{
	return pos >= 0 && pos < 64;
}

RankFile getRankFile(Position pos)
{
	return (RankFile){ .rank = pos >> 3, .file = pos & 0b111 };
}

uint8_t getRank(Position pos)
{
	return pos >> 3;
}

uint8_t getFile(Position pos)
{
	return pos & 0b111;
}

Position getPos(uint8_t rank, uint8_t file)
{
	return rank << 3 | file;
}

Position notationToPos(char* notation)
{
	Position pos;
	uint8_t file = notation[0] - 'a';
	uint8_t rank = notation[1] - '1';
	return getPos(rank, file);
}

void posToNotation(Position pos, char* notation)
{
	RankFile rf = getRankFile(pos);
	notation[0] = rf.file + 'a';
	notation[1] = rf.rank + '1';
}

void movePiece(Position from, Position to, Game* game)
{
	Piece* piece = getPiecePtr(from, game);
	piece->pos = to;

	game->board[to] = game->board[from];
	game->board[from] = NONE;

	game->refBoard[to] = game->refBoard[from];
	game->refBoard[from] = NONE;
	
	PieceColor color = getPieceColor(piece->info);

	resetBit(game->piecesBB[color], from);
	setBit(game->piecesBB[color], to);
}

PieceRef capturePiece(Position pos, Game* game)
{
	PieceRef ref = game->refBoard[pos];
	Piece* piece = getPiecePtrByRef(ref, game);
	piece->pos = NONE;
	game->board[pos] = NONE;
	game->refBoard[pos] = NONE;

	PieceColor color = getPieceColor(piece->info);
	PieceType type = getPieceType(piece->info);

	resetBit(game->piecesBB[color], pos);
	return ref;
}

void undoCapture(Position pos, PieceRef piece, Game* game)
{
	getPiecePtrByRef(piece, game)->pos = pos;
	game->refBoard[pos] = piece;
	game->board[pos] = getPieceByRef(piece, game).info;
}

PieceRef enPassant(Position from, Position to, Game* game)
{
	movePiece(from, to, game);
	return capturePiece(getPos(getRank(from), getFile(to)), game);
}

void undoEnPassant(Position from, Position to, PieceRef captured, Game* game)
{
	movePiece(to, from, game);
	undoCapture(getPos(getRank(from), getFile(to)), captured, game);
}

void castle(Position from, Position to, Game* game)
{
	movePiece(from, to, game);
	RankFile rook, rookTo;
	rook.rank = getRank(from);
	rookTo.rank = rook.rank;
	if (getFile(to) == 2) {
		rook.file = 0;
		rookTo.file = 3;
	}
	else {
		rook.file = 7;
		rookTo.file = 5;
	}
	movePiece(getPos(rook.rank, rook.file),
		getPos(rookTo.rank, rookTo.file), game);
	if (rook.rank == 0) {
		takeCastlingRights(WHITE, QUEENSIDE, game);
		takeCastlingRights(WHITE, KINGSIDE, game);
	}
	else {
		takeCastlingRights(BLACK, QUEENSIDE, game);
		takeCastlingRights(BLACK, KINGSIDE, game);
	}
}

void undoCastle(Position from, Position to, Game* game)
{
	movePiece(to, from, game);
	RankFile rook, rookTo;
	rook.rank = getRank(from);
	rookTo.rank = rook.rank;
	PieceColor c;
	CastleSide side;
	if (getRank(from) == 0) c = WHITE;
	else c = BLACK;
	if (getFile(to) == 2) {
		rook.file = 0;
		rookTo.file = 3;
		side = QUEENSIDE;
	}
	else {
		rook.file = 7;
		rookTo.file = 5;
		side = KINGSIDE;
	}
	movePiece(getPos(rookTo.rank, rookTo.file),
		getPos(rook.rank, rook.file), game);
}

void takeCastlingRights(PieceColor c, CastleSide side, Game* game)
{
	if (game->whenLostCR[c][side] > game->moveCnt)
		game->whenLostCR[c][side] = game->moveCnt;
}

// must be called when making a new move
// must not be called when redoing an undone move
void updateCastlingRights(Position pos, Game* game)
{
	if (game->moveCnt < game->totalMoves) {
	// if we're moving after undoing,
	// this condition will be true.
	// here we check if we lost castling rights during
	// the moves that we undid, if so reset whenLostCR to NEVER.
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				if (game->whenLostCR[i][j] >= game->moveCnt) {
					game->whenLostCR[i][j] = NEVER;
				}
			}
		}
	}

	PieceColor c;
	uint8_t rank = getRank(pos);
	uint8_t file = getFile(pos);

	if (rank == 0) c = WHITE;
	else if (rank == 7) c = BLACK;
	else return;
	if (file == 0) { // queenside rook
		takeCastlingRights(c, QUEENSIDE, game);
	} else if (file == 7) { // kingside rook
		takeCastlingRights(c, KINGSIDE, game);
	} else if (file == 4) { // king
		takeCastlingRights(c, QUEENSIDE, game);
		takeCastlingRights(c, KINGSIDE, game);
	}

}

void changeType(Position pos, PieceType type, Game* game)
{
	Piece* p = getPiecePtr(pos, game);
	p->info += type - getPieceType(p->info);
	game->board[p->pos] = p->info;
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
	if (c >= 'A' && c <= 'Z') {
		color = WHITE;
	} else if (c >= 'a' && c <= 'z') {
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

void printBoard(const Game* game)
{
	Position pos;
	printf(" +");
	for (int j = 0; j < 8; j++) {
		printf("-+");
	}
	printf("\n");
	for (int i = 7; i >= 0; i--) {
		pos = i << 3;
		printf("%d|", i + 1);
		for (int j = 0; j < 8; j++) {
			if (isPosEmpty(pos, game)) printf(" |");
			else printf("%c|", getPieceSymbol(game->board[pos]));
			pos++;
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

void printBoardFlipped(const Game *game)
{
	Position pos;
	printf(" +");
	for (int j = 0; j < 8; j++) {
		printf("-+");
	}
	printf("\n");
	for (int i = 0; i < 8; i++) {
		pos = i << 3 | 7;
		printf("%d|", i + 1);
		for (int j = 7; j >= 0; j--) {
			if (isPosEmpty(pos, game)) printf(" |");
			else printf("%c|", getPieceSymbol(game->board[pos]));
			pos--;
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

bool isMovePromotion(Position from, Position to, const Game* game)
{
	PieceInfo p = game->board[from];
	uint8_t rank = getRank(to);
	return (p == PIECE(WHITE, PAWN) && rank == 7) ||
		(p == PIECE(BLACK, PAWN) && rank == 0);
}

bool isPromotionValid(Position from, Position to, PieceType prom,
	const Game* game)
{
	if (isMovePromotion(from, to, game) == false) return prom == NONE;
	return prom >= KNIGHT && prom <= QUEEN;
}

// function assumes move is legal, so check move legality before calling
Move makeMove(Position from, Position to, PieceType promotion, Game* game)
{
	Move move;
	move.from = from;
	move.to = to;
	move.type = getMoveType(from, to, game);
	move.captured = NONE;

	game->enPassantFile = NONE;
	switch (move.type)
	{
	case REGMOVE:
		if (getPieceType(game->board[from] == PAWN))
			game->lastPawnOrCapture = game->moveCnt + 1;
		if (isPosEmpty(to, game) == false) {
			move.captured = capturePiece(to, game);
			game->lastPawnOrCapture = game->moveCnt + 1;
		}
		movePiece(from, to, game);
		if (promotion != NONE) {
			changeType(to, promotion, game);
			move.type = promotion;
		}
		break;
	case DOUBLESTEP:
		movePiece(from, to, game);
		game->enPassantFile = getFile(from);
		game->lastPawnOrCapture = game->moveCnt + 1;
		break;
	case ENPASSANT:
		move.captured = enPassant(from, to, game);
		game->lastPawnOrCapture = game->moveCnt + 1;
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
	calculateGame(game);

	return move;
}

void undoMove(Game* game)
{
	if (game->moveCnt == 0) return;
	Move move = game->history[--game->moveCnt];
	Position from = move.from;
	Position to = move.to;
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

	if (game->moveCnt == 0) {
		game->enPassantFile = NONE;
	}
	else {
		Move prevMove = game->history[game->moveCnt - 1];
		if (prevMove.type == DOUBLESTEP) {
			game->enPassantFile = getFile(prevMove.from);
		} else {
			game->enPassantFile = NONE;
		}
	}

	calculateGame(game);
}

void redoMove(Game* game)
{
	if (game->moveCnt == game->totalMoves) return;
	Move move = game->history[game->moveCnt];
	Position from = move.from;
	Position to = move.to;

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
		game->enPassantFile = getFile(from);
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
	calculateGame(game);
}

MoveType getMoveType(Position from, Position to, Game* game)
{
	PieceInfo p = game->board[from];
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	switch(getPieceType(p))
	{
	case PAWN:
		if (abs(from_rank - to_rank) == 2)
			return DOUBLESTEP;
		if (game->enPassantFile != to_file) break;
		if (from_rank == 4 && to_rank == 5) {
			return ENPASSANT;
		}
		if (from_rank == 3 && to_rank == 2) {
			return ENPASSANT;
		}

	case KING:
		if (abs(from_file - to_file) == 2)
			return CASTLE;
		break;
	}
	return REGMOVE;
}

Move moveByNotation(char *notation, Game* game)
{
	Position from = notationToPos(notation);
	Position to = notationToPos(notation + 2);
	PieceInfo piece = symbolToPiece(notation[4]);
	PieceType prom = getPieceType(piece);
	if (prom < KNIGHT || prom > QUEEN) prom = NONE;
	return checkAndMove(from, to, prom, game);
}


bool isWhitePawnMoveLegal(Position from, Position to, const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	PieceRef capture = game->refBoard[to];
	if (capture == NONE) {
		if (from_rank + 1 == to_rank && from_file == to_file) return true;
		if (from_rank == 1 && to_rank == 3 && from_file == to_file) {
			PieceRef between = game->refBoard[getPos(2, from_file)];
			if (between == NONE) {
				return true;
			}
		}
	}
	if (from_rank + 1 == to_rank && abs(from_file - to_file) == 1) {
		if (capture != NONE) {
			return true;
		}
		if (game->enPassantFile == to_file && from_rank == 4) {
			return true;
		}
	}
	return false;
}

bool isBlackPawnMoveLegal(Position from, Position to, const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	PieceRef capture = game->refBoard[to];
	if (capture == NONE) {
		if (from_rank - 1 == to_rank && from_file == to_file) return true;
		if (from_rank == 6 && to_rank == 4 && from_file == to_file) {
			PieceRef between = game->refBoard[getPos(5, from_file)];
			if (between == NONE) {
				return true;
			}
		}
	}
	if (from_rank - 1 == to_rank && abs(from_file - to_file) == 1) {
		if (capture != NONE) {
			return true;
		}
		if (game->enPassantFile == to_file && from_rank == 3) {
			return true;
		}
	}
	return false;
}

bool isPawnMoveLegal(Position from, Position to, const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	PieceColor color = getPieceColor(game->board[from]);
	if (color == WHITE) return isWhitePawnMoveLegal(from, to, game);
	else return isBlackPawnMoveLegal(from, to, game);
}

bool isKnightMoveLegal(Position from, Position to)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	if (abs(from_rank - to_rank) == 2 && abs(from_file - to_file) == 1) return true;
	if (abs(from_rank - to_rank) == 1 && abs(from_file - to_file) == 2) return true;
	return false;
}

bool isBishopMoveLegal(Position from, Position to, const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	if (abs(from_rank - to_rank) != abs(from_file - to_file)) return false;
	int rankdir = from_rank > to_rank ? -1 : 1;
	int filedir = from_file > to_file ? -1 : 1;
	int j = from_file + filedir;
	for (int i = from_rank + rankdir; i != to_rank; i += rankdir) {
		if (game->refBoard[getPos(i, j)] != NONE) return false;
		j += filedir;
	}
	return true;
}

bool isRookMoveLegal(Position from, Position to, const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	if (from_rank != to_rank && from_file == to_file) {
		int rankdir = from_rank > to_rank ? -1 : 1;
		for (int i = from_rank + rankdir; i != to_rank; i += rankdir) {
			if (game->refBoard[getPos(i, from_file)] != NONE) return false;
		}
	}
	else if (from_rank == to_rank && from_file != to_file) {
		int filedir = from_file > to_file ? -1 : 1;
		for (int j = from_file + filedir; j != to_file; j += filedir) {
			if (game->refBoard[getPos(from_rank, j)] != NONE) return false;
		}
	}
	else {
		return false;
	}
	return true;
}

bool isQueenMoveLegal(Position from, Position to, const Game* game)
{
	return isBishopMoveLegal(from, to, game) || isRookMoveLegal(from, to, game);
}

bool isKingMoveLegal(PieceColor color, Position from, Position to,
	const Game* game)
{
	uint8_t from_rank = getRank(from);
	uint8_t from_file = getFile(from);
	uint8_t to_rank = getRank(to);
	uint8_t to_file = getFile(to);

	PieceColor enemy = getEnemyColor(color);
	if (getAttackers(to, enemy, game)) return false;
	if (from_rank == color * 7 && from_rank == to_rank && from_file == 4) {
		if (getAttackers(from, enemy, game)) return false;
		uint8_t between_file;
		if (to_file == 2) {
			between_file = 3;
			return isPosEmpty(getPos(from_rank, between_file), game) &&
				!getAttackers(getPos(from_rank, between_file), enemy, game)
				&& game->moveCnt <= game->whenLostCR[color][QUEENSIDE];
		}
		else if (to_file == 6) {
			between_file = 5;
			return isPosEmpty(getPos(from_rank, between_file), game) &&
				!getAttackers(getPos(from_rank, between_file), enemy, game)
				&& game->moveCnt <= game->whenLostCR[color][KINGSIDE];
		}
		else {
			return (to_file == 3 || from_file == 5);
		}

	}
	return abs(from_rank - to_rank) <= 1 && abs(from_file - to_file) <= 1;
}

bool isMoveGenerallyValid(Position from, Position to, const Game* game)
{
	if (isPosValid(from) == false || isPosValid(to) == false)
		return false;

	if (from == to) return false;

	if (isPosEmpty(from, game)) {
		return false;
	}

	PieceColor pieceColor = getPieceColor(game->board[from]);
	if (pieceColor != game->colorToMove) {
		return false;
	}

	if (isPosEmpty(to, game) == false) {
		if (pieceColor == getPieceColor(game->board[to])) {
			return false;
		}
	}

	return true;
}

bool isMoveLegal(Position from, Position to, const Game* game)
{
	if (isMoveGenerallyValid(from, to, game) == false) return false;
	PieceRef pieceRef = game->refBoard[from];
	Bitboard legalMoves = *((Bitboard*)game->legalMovesBB + pieceRef);
	return testBit(legalMoves, to);

/*
	if (isMoveGenerallyValid(from, to, game) == false) return false;
	PieceInfo piece = game->board[from];

	switch (getPieceType(piece))
	{
	case PAWN:
		if (getPieceColor(piece) == WHITE)
			return isWhitePawnMoveLegal(from, to, game);
		else
			return isBlackPawnMoveLegal(from, to, game);
	case KNIGHT:
		return isKnightMoveLegal(from, to);
	case BISHOP:
		return isBishopMoveLegal(from, to, game);
	case ROOK:
		return isRookMoveLegal(from, to, game);
	case QUEEN:
		return isQueenMoveLegal(from, to, game);
	case KING:
		return isKingMoveLegal(getPieceColor(piece), from, to, game);
	default:
		return false;
	}
*/
}

Move checkAndMove(Position from, Position to, PieceType prom,
	Game* game)
{
	Move move;
	if (isMoveLegal(from, to, game) == false) {
		move.type = ILLEGAL;
		return move;
	}
	if (isPromotionValid(from, to, prom, game) == false) {
		printf("Invalid promotion!\n");
		move.type = ILLEGAL;
		return move;
	}

	PieceColor colorThatMoved = game->colorToMove;

	move = makeMove(from, to, prom, game);

//	if (isUnderCheck(colorThatMoved, game)) {
//		undoMove(game);
//		move.type = ILLEGAL;
//	}

	return move;
}

const bool isSlider[6] = {false, false, true, true, true, false};
const uint8_t offsets[6] = {0, 8, 4, 4, 8, 8}; /* knight or ray directions */
const int8_t offset[6][8] = {
	{   0,   0,  0,  0, 0,  0,  0,  0 },
	{ -21, -19,-12, -8, 8, 12, 19, 21 }, /* KNIGHT */
	{ -11,  -9,  9, 11, 0,  0,  0,  0 }, /* BISHOP */
	{ -10,  -1,  1, 10, 0,  0,  0,  0 }, /* ROOK */
	{ -11, -10, -9, -1, 1,  9, 10, 11 }, /* QUEEN */
	{ -11, -10, -9, -1, 1,  9, 10, 11 }  /* KING */
};

const int8_t mailbox[120] = {
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
     -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
     -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
     -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
     -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
     -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
     -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
     -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const uint8_t mailbox64[64] = {
    21, 22, 23, 24, 25, 26, 27, 28,
    31, 32, 33, 34, 35, 36, 37, 38,
    41, 42, 43, 44, 45, 46, 47, 48,
    51, 52, 53, 54, 55, 56, 57, 58,
    61, 62, 63, 64, 65, 66, 67, 68,
    71, 72, 73, 74, 75, 76, 77, 78,
    81, 82, 83, 84, 85, 86, 87, 88,
    91, 92, 93, 94, 95, 96, 97, 98
};

Bitboard getAttacks(Position from, PieceColor color, PieceType type,
	const Game* game)
{
	uint8_t rank = getRank(from);
	uint8_t file = getFile(from);

	Bitboard bb = C64(0);

	int8_t i, n;

	if (type != PAWN) {
		for (i = 0; i < offsets[type]; i++) {
			for (n = from;;) {
				n = mailbox[mailbox64[n] + offset[type][i]];
				if (n == -1) break; /* outside board */
				if (game->board[n] != NONE) { /* not empty */
					setBit(bb, n);
					break;
				}
				setBit(bb, n);
				if (!isSlider[type]) break; /* next direction */
			}
		}
	}

	else {  // if pawn
		if (color == WHITE && rank < 7) {
			if (file > 0) setBit(bb, from + 7);
			if (file < 7) setBit(bb, from + 9);
		}

		else if (color == BLACK && rank > 0) {
			if (file > 0) setBit(bb, from - 9);
			if (file < 7) setBit(bb, from - 7);
		}
	}

	return bb;
}

// get the direction that points from A to B
Position getDirection(Position from, Position to)
{
	int8_t distance = mailbox64[to] - mailbox64[from];
	int8_t sign = (distance > 0 ? 1 : -1);

	if (abs(distance) < 8) {
		return sign;
	}
	else if (distance % 9 == 0) {
		return sign * 9;
	}
	else if (distance % 10 == 0) {
		return sign * 10;
	}
	else if (distance % 11 == 0) {
		return sign * 11;
	}
	else {
		return 0;
	}

}

// used when the king is in check and we have to determine the square
// that is behind the king that is not technically being attacked but
// will become attacked once the king moves there
Position getBehindSquare(Position from, Position to)
{
	int8_t direction = getDirection(from, to);
	if (direction == 0) return -1;
	Position behind = mailbox[mailbox64[to] + direction];

	return behind;
}

// used for determining squares where a piece can move to block a check
// or where a pinned piece is
Bitboard getMiddleSquares(Position from, Position to)
{
	Bitboard bb = C64(0);

	int8_t direction = getDirection(from, to);
	if (direction == 0) return bb;

	for (int8_t i = from; ;) {
		i = mailbox[mailbox64[i] + direction];
		if (i == -1) break;
		if (i == to) break;
		setBit(bb, i);
	}

	return bb;
}

// used for determining squares where a pinned piece can move
Bitboard getExtendedLine(Position from, Position to)
{
	Bitboard bb = C64(0);

	int8_t direction = getDirection(from, to);
	if (direction == 0) return bb;

	for (int8_t i = from; ;) {
		i = mailbox[mailbox64[i] + direction];
		if (i == -1) break;
		setBit(bb, i);
	}
	return bb;
}

void calculateAllAttacks(Game* game)
{
	PieceColor enemy = BLACK;

	for (PieceColor color = WHITE; color <= BLACK; color++) {
		Position enemyKingPos = game->pieces[enemy][0].pos;
		game->numCheckers[enemy] = 0;
		game->totalAttacksBB[color] = C64(0);
		game->checkXRayBB[enemy] = C64(0);
		game->pinnedBB[enemy] = C64(0);

		for (int i = 0; i < 16; i++) {
			Piece p = game->pieces[color][i];

			// if piece has been captured, skip
			if (p.pos == NONE) continue;

			PieceType type = getPieceType(p.info);

			// get squares attacked by piece
			Bitboard attacksBB = getAttacks(p.pos, color, type, game);
			game->attacksBB[color][i] = attacksBB;
			game->totalAttacksBB[color] |= attacksBB;

			// get squares X-Rayed by sliding piece
			Bitboard xRayBB = C64(0);

			// if attacking enemy king
			if (testBit(attacksBB, enemyKingPos)) {
				game->checkerPos[enemy] = p.pos;
				game->numCheckers[enemy]++;

				// get X-rayed squares
				if (isSlider[type]) {
					Position behind = getBehindSquare(p.pos, enemyKingPos);
					if (behind != -1) {
						setBit(game->checkXRayBB[enemy], behind);
					}
				}
			}

			if (!isSlider[type]) continue;

			// calculate middle squares between enemy king and piece
			Bitboard middleSquaresBB = getMiddleSquares(enemyKingPos, p.pos);

			// a piece is pinned if it's attacked and it's in the middle
			// and it's an enemy piece (and it's the only one in the middle)
			Bitboard pinBB = middleSquaresBB & attacksBB & game->piecesBB[enemy];
			Bitboard middlePieces = middleSquaresBB &
				(game->piecesBB[enemy] | game->piecesBB[color]);
			if (middlePieces == pinBB) {
				game->pinnedBB[enemy] |= pinBB;
			}
		}
		if (game->numCheckers[enemy] > 0) {
			if (enemy == WHITE) {
				printf("White is in check! (%d)\n", game->numCheckers[enemy]);
			} else {
				printf("Black is in check! (%d)\n", game->numCheckers[enemy]);
			}
		}

		enemy = WHITE;
	}
}

Bitboard getAttackers(Position target, PieceColor color, const Game* game)
{
	Bitboard bb = C64(0);
	for (int i = 0; i < 16; i++) {
		if (testBit(game->attacksBB[color][i], target)) {
			setBit(bb, game->pieces[color][i].pos);
		}
	}
	return bb;
}

Bitboard getCastleMoves(Position from, PieceColor color, const Game* game)
{
	Bitboard bb = C64(0);
	uint8_t rank = getRank(from);
	uint8_t file = getFile(from);
	if (rank == color * 7 && file == 4) {
		if (game->moveCnt <= game->whenLostCR[color][QUEENSIDE] &&
			game->board[from - 1] == NONE &&
			game->board[from - 2] == NONE &&
			getAttackers(from - 1, !color, game) == 0 &&
			getAttackers(from - 2, !color, game) == 0)
		{
			setBit(bb, from - 2);
		}
		if (game->moveCnt <= game->whenLostCR[color][KINGSIDE] &&
			game->board[from + 1] == NONE &&
			game->board[from + 2] == NONE &&
			getAttackers(from + 1, !color, game) == 0 &&
			getAttackers(from + 2, !color, game) == 0)
		{
			setBit(bb, from + 2);
		}
	}
	return bb;
}

Bitboard getPawnMoves(Position from, PieceColor color, const Game* game)
{
	Bitboard bb = C64(0);
	uint8_t rank = getRank(from);
	uint8_t file = getFile(from);

	if (color == WHITE) {
		if (rank < 7 && game->board[from + 8] == NONE) {
			setBit(bb, from + 8);
			if (rank == 1) {
				if (game->board[from + 16] == NONE) {
					setBit(bb, from + 16);
				}
			}
		}
	}

	else { // if black pawn
		if (rank > 0 && game->board[from - 8] == NONE) {
			setBit(bb, from - 8);
			if (rank == 6) {
				if (game->board[from - 16] == NONE) {
					setBit(bb, from - 16);
				}
			}
		}
	}

	return bb;
}

Bitboard getEnPassantMoves(Position from, PieceColor color, const Game* game)
{
	Bitboard bb = C64(0);
	uint8_t rank = getRank(from);
	uint8_t file = getFile(from);

	if (color == WHITE) {
		if (game->enPassantFile != NONE && rank == 4) {
			if (game->enPassantFile == file + 1)
				setBit(bb, from + 9);
			else if (game->enPassantFile == file - 1)
				setBit(bb, from + 7);
		}

	}

	else {
		if (game->enPassantFile != NONE && rank == 3) {
			if (game->enPassantFile == file + 1)
				setBit(bb, from - 7);
			else if (game->enPassantFile == file - 1)
				setBit(bb, from - 9);
		}
	}

	return bb;
}

Bitboard getLegalMoves(Position from, const Game* game)
{
	if (isPosEmptyOrInvalid(from, game)) return C64(0);

	PieceInfo p = game->board[from];
	PieceColor color = getPieceColor(p);
	if (color != game->colorToMove) return C64(0);
	PieceColor enemy = getEnemyColor(color);
	PieceType type = getPieceType(p);
	uint8_t numCheckers = game->numCheckers[color];

	// if in double check, only king can move
	if (numCheckers == 2 && type != KING) return C64(0);

	PieceRef ref = game->refBoard[from];
	Bitboard attacksBB = game->attacksBB[color][ref & 0b1111];

	Bitboard bb, castleBB, pawnBB, enPassantBB;
	// if not pawn
	if (type != PAWN) {
		// move is valid if it isn't occupied by same color piece
		attacksBB &= ~game->piecesBB[color];
		// if king, can't move to squares attacked by enemy pieces
		if (type == KING) {
			attacksBB &= ~game->totalAttacksBB[enemy];
		}
	}
	else { // if pawn
		// attack is valid only if enemy piece is there
		attacksBB &= game->piecesBB[enemy];
		// get normal pawn moves and en passant moves
		pawnBB = getPawnMoves(from, color, game);
		enPassantBB = getEnPassantMoves(from, color, game);
	}

	Position kingPos = game->pieces[color][0].pos;

	if (numCheckers > 0) {
		if (type == KING) {
			// can't move to squares X-rayed by checking pieces
			return attacksBB & ~game->checkXRayBB[color];
		}

		Position checkerPos = game->checkerPos[color];
		Bitboard checkerBB = C64(1) << checkerPos;
		Bitboard blockBB = getMiddleSquares(kingPos, checkerPos);
		attacksBB &= (blockBB | checkerBB);

		if (type == PAWN) {
			pawnBB &= blockBB;
			enPassantBB &= blockBB;
		}
	}

	if (type == KING) {
		castleBB = getCastleMoves(from, color, game);
		return attacksBB | castleBB;
	}
	else if (type == PAWN) {
		if (enPassantBB) {
			// position of the to-be-captured pawn (en passant)
			Position cap = getPos(getRank(from), game->enPassantFile);
			Bitboard capBB = C64(1) << cap;
			Bitboard fromBB = C64(1) << from;
			// get enemy attackers of our pawn and the to-be-captured pawn
			Bitboard atkersBB = getAttackers(from, enemy, game) |
				getAttackers(cap, enemy, game);
			printBitboard(atkersBB);

			if (atkersBB) do {
				Position atkerPos = bitScanForward(atkersBB);
				PieceType atkerType =
					getPieceType(game->board[atkerPos]);

				if (isSlider[atkerType]) {
					// check if the direction from attacker to king
					// coincides with the attacker's move direction
					int8_t dir = getDirection(atkerPos, kingPos);
					bool xRaysKing = false;
					for (int i = 0; i < offsets[atkerType]; i++) {
						if (dir == offset[atkerType][i]) {
							xRaysKing = true;
						}
					}
					// check next attacker
					if (xRaysKing == false) continue;

					Bitboard middleBB = getMiddleSquares(atkerPos, kingPos);
					printBitboard(middleBB);
					Bitboard otherPiecesInMiddle = middleBB
						& (game->piecesBB[color] | game->piecesBB[enemy])
						& ~(capBB | fromBB);
					printBitboard(otherPiecesInMiddle);
					// if there are no other pieces in the middle
					// and if the destination of the move isn't either,
					// then the move is illegal.
					if (otherPiecesInMiddle == 0
						&& (middleBB & enPassantBB) == 0)
					{
						enPassantBB = C64(0);
						break;
					}
				}
			} while (atkersBB &= atkersBB - 1);
		}

		bb = attacksBB | pawnBB | enPassantBB;
	}
	else {
		bb = attacksBB;
	}

	// if the piece is pinned
	if (testBit(game->pinnedBB[color], from)) {
		bb &= getExtendedLine(kingPos, from);
	}

	return bb;
}

void calculateAllLegalMoves(Game* game)
{
	PieceColor color = game->colorToMove;
	int cnt = 0;
	for (int i = 0; i < 16; i++) {
		Piece p = game->pieces[color][i];
		Bitboard legalMoves = getLegalMoves(p.pos, game);
		cnt += popCount(legalMoves);
		game->legalMovesBB[color][i] = legalMoves;
	}
	if (cnt == 0) {
		if (!isUnderCheck(color, game)) {
			game->state = DRAW;
			printf("Stalemate!\n");
			return;
		}
		if (color == WHITE) {
			game->state = BLACK_WON;
			printf("Black wins!\n");
		}
		else {
			game->state = WHITE_WON;
			printf("White wins!\n");
		}
	}
}

void calculateGame(Game* game)
{
	calculateAllAttacks(game);
	calculateAllLegalMoves(game);
}

bool isUnderCheck(PieceColor color, const Game* game)
{
	return game->numCheckers[color] != 0;
}
