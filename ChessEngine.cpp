#include "ChessEngine.h"
#include <sstream>
#include <algorithm>
#include <cmath>

// Move类实现
std::string Move::toString() const {
    if (!isValid()) return "invalid";
    
    // 转换为中国象棋记谱格式
    char fromFile = 'a' + fromCol;
    char toFile = 'a' + toCol;
    int fromRank = 10 - fromRow;
    int toRank = 10 - toRow;
    
    std::stringstream ss;
    ss << fromFile << fromRank << toFile << toRank;
    return ss.str();
}

// ChessEngine类实现
ChessEngine::ChessEngine() : redToMove(true) {
    initializeBoard();
}

ChessEngine::~ChessEngine() {
}

void ChessEngine::initializeBoard() {
    // 清空棋盘
    clearBoard();
    
    // 设置初始棋子位置
    // 黑方（上方）
    board[0][0] = board[0][8] = BLACK_ROOK;
    board[0][1] = board[0][7] = BLACK_KNIGHT;
    board[0][2] = board[0][6] = BLACK_BISHOP;
    board[0][3] = board[0][5] = BLACK_ADVISOR;
    board[0][4] = BLACK_KING;
    board[2][1] = board[2][7] = BLACK_CANNON;
    board[3][0] = board[3][2] = board[3][4] = board[3][6] = board[3][8] = BLACK_PAWN;
    
    // 红方（下方）
    board[9][0] = board[9][8] = RED_ROOK;
    board[9][1] = board[9][7] = RED_KNIGHT;
    board[9][2] = board[9][6] = RED_BISHOP;
    board[9][3] = board[9][5] = RED_ADVISOR;
    board[9][4] = RED_KING;
    board[7][1] = board[7][7] = RED_CANNON;
    board[6][0] = board[6][2] = board[6][4] = board[6][6] = board[6][8] = RED_PAWN;
    
    redToMove = true;
    moveHistory.clear();
}

void ChessEngine::clearBoard() {
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            board[i][j] = NONE;
        }
    }
}

PieceType ChessEngine::getPiece(int row, int col) const {
    if (!isInBounds(row, col)) return NONE;
    return board[row][col];
}

void ChessEngine::setPiece(int row, int col, PieceType piece) {
    if (isInBounds(row, col)) {
        board[row][col] = piece;
    }
}

bool ChessEngine::isInBounds(int row, int col) const {
    return row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS;
}

bool ChessEngine::isRed(PieceType piece) const {
    return piece >= RED_KING && piece <= RED_PAWN;
}

bool ChessEngine::isBlack(PieceType piece) const {
    return piece >= BLACK_KING && piece <= BLACK_PAWN;
}

bool ChessEngine::isSameColor(PieceType p1, PieceType p2) const {
    return (isRed(p1) && isRed(p2)) || (isBlack(p1) && isBlack(p2));
}

bool ChessEngine::isValidMove(const Move& move) const {
    if (!move.isValid() || !isInBounds(move.fromRow, move.fromCol) || !isInBounds(move.toRow, move.toCol)) {
        return false;
    }
    
    PieceType movingPiece = board[move.fromRow][move.fromCol];
    PieceType targetPiece = board[move.toRow][move.toCol];
    
    // 检查是否有棋子可以移动
    if (movingPiece == NONE) return false;
    
    // 检查是否轮到该方下棋
    if ((redToMove && !isRed(movingPiece)) || (!redToMove && !isBlack(movingPiece))) {
        return false;
    }
    
    // 不能吃自己的棋子
    if (targetPiece != NONE && isSameColor(movingPiece, targetPiece)) {
        return false;
    }
    
    // 检查具体棋子的走法规则
    bool validMove = false;
    switch (movingPiece) {
        case RED_KING:
        case BLACK_KING:
            validMove = isValidKingMove(move);
            break;
        case RED_ADVISOR:
        case BLACK_ADVISOR:
            validMove = isValidAdvisorMove(move);
            break;
        case RED_BISHOP:
        case BLACK_BISHOP:
            validMove = isValidBishopMove(move);
            break;
        case RED_KNIGHT:
        case BLACK_KNIGHT:
            validMove = isValidKnightMove(move);
            break;
        case RED_ROOK:
        case BLACK_ROOK:
            validMove = isValidRookMove(move);
            break;
        case RED_CANNON:
        case BLACK_CANNON:
            validMove = isValidCannonMove(move);
            break;
        case RED_PAWN:
        case BLACK_PAWN:
            validMove = isValidPawnMove(move);
            break;
        default:
            return false;
    }
    
    if (!validMove) return false;
    
    // 检查走法后是否会被将军
    return !wouldBeInCheck(move, isRed(movingPiece));
}

bool ChessEngine::isValidKingMove(const Move& move) const {
    int rowDiff = abs(move.toRow - move.fromRow);
    int colDiff = abs(move.toCol - move.fromCol);
    
    // 帅/将只能走一格，且只能在九宫格内
    if ((rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1)) {
        PieceType king = board[move.fromRow][move.fromCol];
        if (isRed(king)) {
            // 红帅在下方九宫格
            return move.toRow >= 7 && move.toRow <= 9 && move.toCol >= 3 && move.toCol <= 5;
        } else {
            // 黑将在上方九宫格
            return move.toRow >= 0 && move.toRow <= 2 && move.toCol >= 3 && move.toCol <= 5;
        }
    }
    
    // 检查将帅对面（飞将）
    if (move.fromCol == move.toCol) {
        PieceType targetPiece = board[move.toRow][move.toCol];
        if ((isRed(board[move.fromRow][move.fromCol]) && targetPiece == BLACK_KING) ||
            (isBlack(board[move.fromRow][move.fromCol]) && targetPiece == RED_KING)) {
            // 检查中间是否有棋子
            int minRow = std::min(move.fromRow, move.toRow);
            int maxRow = std::max(move.fromRow, move.toRow);
            for (int r = minRow + 1; r < maxRow; r++) {
                if (board[r][move.fromCol] != NONE) {
                    return false;
                }
            }
            return true;
        }
    }
    
    return false;
}

bool ChessEngine::isValidAdvisorMove(const Move& move) const {
    int rowDiff = abs(move.toRow - move.fromRow);
    int colDiff = abs(move.toCol - move.fromCol);
    
    // 士只能斜走一格
    if (rowDiff != 1 || colDiff != 1) return false;
    
    PieceType advisor = board[move.fromRow][move.fromCol];
    if (isRed(advisor)) {
        // 红士在下方九宫格
        return move.toRow >= 7 && move.toRow <= 9 && move.toCol >= 3 && move.toCol <= 5;
    } else {
        // 黑士在上方九宫格
        return move.toRow >= 0 && move.toRow <= 2 && move.toCol >= 3 && move.toCol <= 5;
    }
}

bool ChessEngine::isValidBishopMove(const Move& move) const {
    int rowDiff = abs(move.toRow - move.fromRow);
    int colDiff = abs(move.toCol - move.fromCol);
    
    // 象只能斜走两格
    if (rowDiff != 2 || colDiff != 2) return false;
    
    // 检查象眼是否被堵
    int eyeRow = (move.fromRow + move.toRow) / 2;
    int eyeCol = (move.fromCol + move.toCol) / 2;
    if (board[eyeRow][eyeCol] != NONE) return false;
    
    PieceType bishop = board[move.fromRow][move.fromCol];
    if (isRed(bishop)) {
        // 红象不能过河
        return move.toRow >= 5;
    } else {
        // 黑象不能过河
        return move.toRow <= 4;
    }
}

bool ChessEngine::isValidKnightMove(const Move& move) const {
    int rowDiff = abs(move.toRow - move.fromRow);
    int colDiff = abs(move.toCol - move.fromCol);
    
    // 马走日字
    if (!((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2))) {
        return false;
    }
    
    // 检查马腿是否被堵
    int legRow, legCol;
    if (rowDiff == 2) {
        legRow = move.fromRow + (move.toRow > move.fromRow ? 1 : -1);
        legCol = move.fromCol;
    } else {
        legRow = move.fromRow;
        legCol = move.fromCol + (move.toCol > move.fromCol ? 1 : -1);
    }
    
    return board[legRow][legCol] == NONE;
}

bool ChessEngine::isValidRookMove(const Move& move) const {
    // 车只能直线移动
    if (move.fromRow != move.toRow && move.fromCol != move.toCol) {
        return false;
    }
    
    // 检查路径是否畅通
    return isPathClear(move.fromRow, move.fromCol, move.toRow, move.toCol);
}

bool ChessEngine::isValidCannonMove(const Move& move) const {
    // 炮只能直线移动
    if (move.fromRow != move.toRow && move.fromCol != move.toCol) {
        return false;
    }
    
    PieceType targetPiece = board[move.toRow][move.toCol];
    int piecesBetween = countPiecesBetween(move.fromRow, move.fromCol, move.toRow, move.toCol);
    
    if (targetPiece == NONE) {
        // 移动时中间不能有棋子
        return piecesBetween == 0;
    } else {
        // 吃子时中间必须有一个棋子
        return piecesBetween == 1;
    }
}

bool ChessEngine::isValidPawnMove(const Move& move) const {
    PieceType pawn = board[move.fromRow][move.fromCol];
    int rowDiff = move.toRow - move.fromRow;
    int colDiff = abs(move.toCol - move.fromCol);
    
    if (isRed(pawn)) {
        // 红兵只能向前（向上）
        if (rowDiff > 0) return false;
        
        if (move.fromRow > 4) {
            // 未过河，只能直走
            return rowDiff == -1 && colDiff == 0;
        } else {
            // 已过河，可以横走或直走
            return (rowDiff == -1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
        }
    } else {
        // 黑卒只能向前（向下）
        if (rowDiff < 0) return false;
        
        if (move.fromRow < 5) {
            // 未过河，只能直走
            return rowDiff == 1 && colDiff == 0;
        } else {
            // 已过河，可以横走或直走
            return (rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
        }
    }
}

bool ChessEngine::isPathClear(int fromRow, int fromCol, int toRow, int toCol) const {
    if (fromRow == toRow && fromCol == toCol) return true;
    
    int rowStep = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int colStep = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;
    
    int currentRow = fromRow + rowStep;
    int currentCol = fromCol + colStep;
    
    while (currentRow != toRow || currentCol != toCol) {
        if (board[currentRow][currentCol] != NONE) {
            return false;
        }
        currentRow += rowStep;
        currentCol += colStep;
    }
    
    return true;
}

int ChessEngine::countPiecesBetween(int fromRow, int fromCol, int toRow, int toCol) const {
    if (fromRow == toRow && fromCol == toCol) return 0;
    
    int count = 0;
    int rowStep = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int colStep = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;
    
    int currentRow = fromRow + rowStep;
    int currentCol = fromCol + colStep;
    
    while (currentRow != toRow || currentCol != toCol) {
        if (board[currentRow][currentCol] != NONE) {
            count++;
        }
        currentRow += rowStep;
        currentCol += colStep;
    }
    
    return count;
}

bool ChessEngine::makeMove(const Move& move) {
    if (!isValidMove(move)) return false;
    
    // 记录走法
    Move recordMove = move;
    recordMove.movingPiece = board[move.fromRow][move.fromCol];
    recordMove.capturedPiece = board[move.toRow][move.toCol];
    
    // 执行走法
    board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
    board[move.fromRow][move.fromCol] = NONE;
    
    // 切换轮次
    redToMove = !redToMove;
    
    // 添加到历史记录
    moveHistory.push_back(recordMove);
    
    return true;
}

bool ChessEngine::undoMove() {
    if (moveHistory.empty()) return false;
    
    Move lastMove = moveHistory.back();
    moveHistory.pop_back();
    
    // 恢复棋盘状态
    board[lastMove.fromRow][lastMove.fromCol] = lastMove.movingPiece;
    board[lastMove.toRow][lastMove.toCol] = lastMove.capturedPiece;
    
    // 切换轮次
    redToMove = !redToMove;
    
    return true;
}

bool ChessEngine::isInCheck(bool isRed) const {
    // 找到王的位置
    int kingRow = -1, kingCol = -1;
    PieceType targetKing = isRed ? RED_KING : BLACK_KING;
    
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            if (board[r][c] == targetKing) {
                kingRow = r;
                kingCol = c;
                break;
            }
        }
        if (kingRow != -1) break;
    }
    
    if (kingRow == -1) return false; // 没有找到王
    
    // 检查是否有敌方棋子可以攻击到王
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            PieceType piece = board[r][c];
            if (piece != NONE && ((isRed && isBlack(piece)) || (!isRed && this->isRed(piece)))) {
                Move attackMove(r, c, kingRow, kingCol, piece, targetKing);
                // 临时检查走法，不考虑将军状态
                if (isValidMoveIgnoreCheck(attackMove)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool ChessEngine::isValidMoveIgnoreCheck(const Move& move) const {
    if (!move.isValid() || !isInBounds(move.fromRow, move.fromCol) || !isInBounds(move.toRow, move.toCol)) {
        return false;
    }
    
    PieceType movingPiece = board[move.fromRow][move.fromCol];
    PieceType targetPiece = board[move.toRow][move.toCol];
    
    if (movingPiece == NONE) return false;
    if (targetPiece != NONE && isSameColor(movingPiece, targetPiece)) return false;
    
    // 检查具体棋子的走法规则（不检查将军）
    switch (movingPiece) {
        case RED_KING:
        case BLACK_KING:
            return isValidKingMove(move);
        case RED_ADVISOR:
        case BLACK_ADVISOR:
            return isValidAdvisorMove(move);
        case RED_BISHOP:
        case BLACK_BISHOP:
            return isValidBishopMove(move);
        case RED_KNIGHT:
        case BLACK_KNIGHT:
            return isValidKnightMove(move);
        case RED_ROOK:
        case BLACK_ROOK:
            return isValidRookMove(move);
        case RED_CANNON:
        case BLACK_CANNON:
            return isValidCannonMove(move);
        case RED_PAWN:
        case BLACK_PAWN:
            return isValidPawnMove(move);
        default:
            return false;
    }
}

bool ChessEngine::wouldBeInCheck(const Move& move, bool isRed) const {
    // 临时执行走法
    PieceType originalPiece = board[move.toRow][move.toCol];
    const_cast<ChessEngine*>(this)->board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
    const_cast<ChessEngine*>(this)->board[move.fromRow][move.fromCol] = NONE;
    
    bool inCheck = isInCheck(isRed);
    
    // 恢复棋盘
    const_cast<ChessEngine*>(this)->board[move.fromRow][move.fromCol] = board[move.toRow][move.toCol];
    const_cast<ChessEngine*>(this)->board[move.toRow][move.toCol] = originalPiece;
    
    return inCheck;
}

std::vector<Move> ChessEngine::generateLegalMoves(bool forRed) const {
    std::vector<Move> moves;
    
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            PieceType piece = board[r][c];
            if (piece != NONE && ((forRed && isRed(piece)) || (!forRed && isBlack(piece)))) {
                switch (piece) {
                    case RED_KING:
                    case BLACK_KING:
                        generateKingMoves(r, c, moves);
                        break;
                    case RED_ADVISOR:
                    case BLACK_ADVISOR:
                        generateAdvisorMoves(r, c, moves);
                        break;
                    case RED_BISHOP:
                    case BLACK_BISHOP:
                        generateBishopMoves(r, c, moves);
                        break;
                    case RED_KNIGHT:
                    case BLACK_KNIGHT:
                        generateKnightMoves(r, c, moves);
                        break;
                    case RED_ROOK:
                    case BLACK_ROOK:
                        generateRookMoves(r, c, moves);
                        break;
                    case RED_CANNON:
                    case BLACK_CANNON:
                        generateCannonMoves(r, c, moves);
                        break;
                    case RED_PAWN:
                    case BLACK_PAWN:
                        generatePawnMoves(r, c, moves);
                        break;
                }
            }
        }
    }
    
    // 过滤掉会导致自己被将军的走法
    std::vector<Move> legalMoves;
    for (const Move& move : moves) {
        if (!wouldBeInCheck(move, forRed)) {
            legalMoves.push_back(move);
        }
    }
    
    return legalMoves;
}

void ChessEngine::generateKingMoves(int row, int col, std::vector<Move>& moves) const {
    // 帅/将的移动方向
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];
        
        Move move(row, col, newRow, newCol);
        if (isValidMoveIgnoreCheck(move)) {
            moves.push_back(move);
        }
    }
}

void ChessEngine::generateAdvisorMoves(int row, int col, std::vector<Move>& moves) const {
    // 士的移动方向（斜向）
    int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    
    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];
        
        Move move(row, col, newRow, newCol);
        if (isValidMoveIgnoreCheck(move)) {
            moves.push_back(move);
        }
    }
}

void ChessEngine::generateBishopMoves(int row, int col, std::vector<Move>& moves) const {
    // 象的移动方向
    int directions[4][2] = {{-2, -2}, {-2, 2}, {2, -2}, {2, 2}};
    
    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];
        
        Move move(row, col, newRow, newCol);
        if (isValidMoveIgnoreCheck(move)) {
            moves.push_back(move);
        }
    }
}

void ChessEngine::generateKnightMoves(int row, int col, std::vector<Move>& moves) const {
    // 马的移动方向
    int directions[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    
    for (int i = 0; i < 8; i++) {
        int newRow = row + directions[i][0];
        int newCol = col + directions[i][1];
        
        Move move(row, col, newRow, newCol);
        if (isValidMoveIgnoreCheck(move)) {
            moves.push_back(move);
        }
    }
}

void ChessEngine::generateRookMoves(int row, int col, std::vector<Move>& moves) const {
    // 车的移动方向
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < BOARD_ROWS; dist++) {
            int newRow = row + directions[i][0] * dist;
            int newCol = col + directions[i][1] * dist;
            
            if (!isInBounds(newRow, newCol)) break;
            
            Move move(row, col, newRow, newCol);
            if (board[newRow][newCol] != NONE) {
                if (!isSameColor(board[row][col], board[newRow][newCol])) {
                    moves.push_back(move);
                }
                break;
            } else {
                moves.push_back(move);
            }
        }
    }
}

void ChessEngine::generateCannonMoves(int row, int col, std::vector<Move>& moves) const {
    // 炮的移动方向
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int i = 0; i < 4; i++) {
        bool foundPlatform = false;
        
        for (int dist = 1; dist < BOARD_ROWS; dist++) {
            int newRow = row + directions[i][0] * dist;
            int newCol = col + directions[i][1] * dist;
            
            if (!isInBounds(newRow, newCol)) break;
            
            if (!foundPlatform) {
                if (board[newRow][newCol] == NONE) {
                    // 可以移动到空位
                    moves.push_back(Move(row, col, newRow, newCol));
                } else {
                    // 找到炮台
                    foundPlatform = true;
                }
            } else {
                if (board[newRow][newCol] != NONE) {
                    // 可以吃子
                    if (!isSameColor(board[row][col], board[newRow][newCol])) {
                        moves.push_back(Move(row, col, newRow, newCol));
                    }
                    break;
                }
            }
        }
    }
}

void ChessEngine::generatePawnMoves(int row, int col, std::vector<Move>& moves) const {
    PieceType pawn = board[row][col];
    
    if (isRed(pawn)) {
        // 红兵向上移动
        if (row > 0) {
            Move move(row, col, row - 1, col);
            if (isValidMoveIgnoreCheck(move)) {
                moves.push_back(move);
            }
        }
        
        // 过河后可以横移
        if (row <= 4) {
            if (col > 0) {
                Move move(row, col, row, col - 1);
                if (isValidMoveIgnoreCheck(move)) {
                    moves.push_back(move);
                }
            }
            if (col < BOARD_COLS - 1) {
                Move move(row, col, row, col + 1);
                if (isValidMoveIgnoreCheck(move)) {
                    moves.push_back(move);
                }
            }
        }
    } else {
        // 黑卒向下移动
        if (row < BOARD_ROWS - 1) {
            Move move(row, col, row + 1, col);
            if (isValidMoveIgnoreCheck(move)) {
                moves.push_back(move);
            }
        }
        
        // 过河后可以横移
        if (row >= 5) {
            if (col > 0) {
                Move move(row, col, row, col - 1);
                if (isValidMoveIgnoreCheck(move)) {
                    moves.push_back(move);
                }
            }
            if (col < BOARD_COLS - 1) {
                Move move(row, col, row, col + 1);
                if (isValidMoveIgnoreCheck(move)) {
                    moves.push_back(move);
                }
            }
        }
    }
}

bool ChessEngine::isCheckmate(bool isRed) const {
    if (!isInCheck(isRed)) return false;
    
    std::vector<Move> legalMoves = generateLegalMoves(isRed);
    return legalMoves.empty();
}

bool ChessEngine::isStalemate(bool isRed) const {
    if (isInCheck(isRed)) return false;
    
    std::vector<Move> legalMoves = generateLegalMoves(isRed);
    return legalMoves.empty();
}

std::string ChessEngine::toFEN() const {
    std::stringstream fen;
    
    // 棋盘状态
    for (int row = 0; row < BOARD_ROWS; row++) {
        int emptyCount = 0;
        for (int col = 0; col < BOARD_COLS; col++) {
            PieceType piece = board[row][col];
            if (piece == NONE) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                
                char pieceChar;
                switch (piece) {
                    case RED_KING: pieceChar = 'K'; break;
                    case RED_ADVISOR: pieceChar = 'A'; break;
                    case RED_BISHOP: pieceChar = 'B'; break;
                    case RED_KNIGHT: pieceChar = 'N'; break;
                    case RED_ROOK: pieceChar = 'R'; break;
                    case RED_CANNON: pieceChar = 'C'; break;
                    case RED_PAWN: pieceChar = 'P'; break;
                    case BLACK_KING: pieceChar = 'k'; break;
                    case BLACK_ADVISOR: pieceChar = 'a'; break;
                    case BLACK_BISHOP: pieceChar = 'b'; break;
                    case BLACK_KNIGHT: pieceChar = 'n'; break;
                    case BLACK_ROOK: pieceChar = 'r'; break;
                    case BLACK_CANNON: pieceChar = 'c'; break;
                    case BLACK_PAWN: pieceChar = 'p'; break;
                    default: pieceChar = '?'; break;
                }
                fen << pieceChar;
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (row < BOARD_ROWS - 1) {
            fen << "/";
        }
    }
    
    // 轮到谁下棋
    fen << (redToMove ? " w" : " b");
    
    return fen.str();
}

bool ChessEngine::fromFEN(const std::string& fenStr) {
    clearBoard();
    moveHistory.clear();
    
    std::istringstream iss(fenStr);
    std::string boardStr, turn;
    iss >> boardStr >> turn;
    
    int row = 0, col = 0;
    for (char c : boardStr) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (isdigit(c)) {
            col += (c - '0');
        } else {
            PieceType piece = NONE;
            switch (c) {
                case 'K': piece = RED_KING; break;
                case 'A': piece = RED_ADVISOR; break;
                case 'B': piece = RED_BISHOP; break;
                case 'N': piece = RED_KNIGHT; break;
                case 'R': piece = RED_ROOK; break;
                case 'C': piece = RED_CANNON; break;
                case 'P': piece = RED_PAWN; break;
                case 'k': piece = BLACK_KING; break;
                case 'a': piece = BLACK_ADVISOR; break;
                case 'b': piece = BLACK_BISHOP; break;
                case 'n': piece = BLACK_KNIGHT; break;
                case 'r': piece = BLACK_ROOK; break;
                case 'c': piece = BLACK_CANNON; break;
                case 'p': piece = BLACK_PAWN; break;
                default: return false;
            }
            
            if (row >= BOARD_ROWS || col >= BOARD_COLS) return false;
            board[row][col] = piece;
            col++;
        }
    }
    
    redToMove = (turn == "w");
    return true;
}

void ChessEngine::copyBoard(PieceType destBoard[10][9]) const {
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            destBoard[i][j] = board[i][j];
        }
    }
}

void ChessEngine::setBoard(const PieceType srcBoard[10][9]) {
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            board[i][j] = srcBoard[i][j];
        }
    }
}