#include "MoveHistory.h"
#include <sstream>
#include <algorithm>

MoveHistory::MoveHistory() : currentIndex(-1) {
}

MoveHistory::~MoveHistory() {
}

void MoveHistory::addMove(const Move& move) {
    // 如果当前不在最后位置，删除后续走法
    if (currentIndex < static_cast<int>(moves.size()) - 1) {
        moves.erase(moves.begin() + currentIndex + 1, moves.end());
    }
    
    moves.push_back(move);
    currentIndex = static_cast<int>(moves.size()) - 1;
}

Move MoveHistory::getMove(int index) const {
    if (index < 0 || index >= static_cast<int>(moves.size())) {
        return Move(); // 返回无效走法
    }
    return moves[index];
}

Move MoveHistory::getCurrentMove() const {
    return getMove(currentIndex);
}

Move MoveHistory::getLastMove() const {
    if (moves.empty()) {
        return Move();
    }
    return moves.back();
}

bool MoveHistory::goToMove(int index) {
    if (index < -1 || index >= static_cast<int>(moves.size())) {
        return false;
    }
    currentIndex = index;
    return true;
}

bool MoveHistory::goToFirst() {
    return goToMove(-1);
}

bool MoveHistory::goToPrevious() {
    if (currentIndex <= -1) {
        return false;
    }
    currentIndex--;
    return true;
}

bool MoveHistory::goToNext() {
    if (currentIndex >= static_cast<int>(moves.size()) - 1) {
        return false;
    }
    currentIndex++;
    return true;
}

bool MoveHistory::goToLast() {
    if (moves.empty()) {
        return goToMove(-1);
    }
    return goToMove(static_cast<int>(moves.size()) - 1);
}

void MoveHistory::clear() {
    moves.clear();
    currentIndex = -1;
}

void MoveHistory::truncateAfterCurrent() {
    if (currentIndex < static_cast<int>(moves.size()) - 1) {
        moves.erase(moves.begin() + currentIndex + 1, moves.end());
    }
}

std::string MoveHistory::toPGN() const {
    std::stringstream pgn;
    
    for (int i = 0; i < static_cast<int>(moves.size()); i++) {
        const Move& move = moves[i];
        
        // 添加回合数
        if (i % 2 == 0) {
            pgn << (i / 2 + 1) << ". ";
        }
        
        // 添加走法
        pgn << moveToChineseNotation(move, i + 1);
        
        if (i % 2 == 0 && i < static_cast<int>(moves.size()) - 1) {
            pgn << " ";
        } else if (i % 2 == 1) {
            pgn << " ";
        }
    }
    
    return pgn.str();
}

std::string MoveHistory::toMoveList() const {
    std::stringstream moveList;
    
    for (int i = 0; i < static_cast<int>(moves.size()); i++) {
        if (i > 0) moveList << " ";
        moveList << moves[i].toString();
    }
    
    return moveList.str();
}

bool MoveHistory::fromMoveList(const std::string& moveList) {
    clear();
    
    std::istringstream iss(moveList);
    std::string moveStr;
    
    while (iss >> moveStr) {
        // 解析走法字符串（格式：a1b2）
        if (moveStr.length() != 4) {
            clear();
            return false;
        }
        
        int fromCol = moveStr[0] - 'a';
        int fromRow = 10 - (moveStr[1] - '0');
        int toCol = moveStr[2] - 'a';
        int toRow = 10 - (moveStr[3] - '0');
        
        if (fromCol < 0 || fromCol >= 9 || fromRow < 0 || fromRow >= 10 ||
            toCol < 0 || toCol >= 9 || toRow < 0 || toRow >= 10) {
            clear();
            return false;
        }
        
        Move move(fromRow, fromCol, toRow, toCol);
        moves.push_back(move);
    }
    
    currentIndex = static_cast<int>(moves.size()) - 1;
    return true;
}

std::vector<int> MoveHistory::findMoves(const Move& move) const {
    std::vector<int> indices;
    
    for (int i = 0; i < static_cast<int>(moves.size()); i++) {
        const Move& m = moves[i];
        if (m.fromRow == move.fromRow && m.fromCol == move.fromCol &&
            m.toRow == move.toRow && m.toCol == move.toCol) {
            indices.push_back(i);
        }
    }
    
    return indices;
}

int MoveHistory::getRedMoveCount() const {
    return (static_cast<int>(moves.size()) + 1) / 2;
}

int MoveHistory::getBlackMoveCount() const {
    return static_cast<int>(moves.size()) / 2;
}

std::string MoveHistory::moveToChineseNotation(const Move& move, int moveNumber) const {
    // 简化的中文记谱法实现
    std::stringstream notation;
    
    // 获取棋子名称
    std::string pieceName = pieceTypeToChineseName(move.movingPiece);
    
    // 起始位置（用数字表示列）
    int fromFile = move.fromCol + 1;
    int toFile = move.toCol + 1;
    
    notation << pieceName;
    
    // 移动方向和目标位置
    if (move.fromCol == move.toCol) {
        // 直线移动
        notation << fromFile;
        if (move.fromRow > move.toRow) {
            notation << "进" << (move.fromRow - move.toRow);
        } else {
            notation << "退" << (move.toRow - move.fromRow);
        }
    } else {
        // 横向移动
        notation << fromFile;
        if (move.fromRow == move.toRow) {
            notation << "平" << toFile;
        } else {
            if (move.fromRow > move.toRow) {
                notation << "进" << toFile;
            } else {
                notation << "退" << toFile;
            }
        }
    }
    
    return notation.str();
}

std::string MoveHistory::pieceTypeToChineseName(PieceType piece) const {
    switch (piece) {
        case RED_KING: return "帅";
        case RED_ADVISOR: return "仕";
        case RED_BISHOP: return "相";
        case RED_KNIGHT: return "马";
        case RED_ROOK: return "车";
        case RED_CANNON: return "炮";
        case RED_PAWN: return "兵";
        case BLACK_KING: return "将";
        case BLACK_ADVISOR: return "士";
        case BLACK_BISHOP: return "象";
        case BLACK_KNIGHT: return "马";
        case BLACK_ROOK: return "车";
        case BLACK_CANNON: return "炮";
        case BLACK_PAWN: return "卒";
        default: return "?";
    }
}