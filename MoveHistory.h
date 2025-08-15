#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include <vector>
#include <string>
#include "ChessEngine.h"

class MoveHistory {
public:
    MoveHistory();
    ~MoveHistory();
    
    // 添加走法
    void addMove(const Move& move);
    
    // 获取走法
    Move getMove(int index) const;
    Move getCurrentMove() const;
    Move getLastMove() const;
    
    // 导航
    bool goToMove(int index);
    bool goToFirst();
    bool goToPrevious();
    bool goToNext();
    bool goToLast();
    
    // 状态查询
    int getCurrentIndex() const { return currentIndex; }
    int getMoveCount() const { return static_cast<int>(moves.size()); }
    bool isEmpty() const { return moves.empty(); }
    bool isAtFirst() const { return currentIndex <= 0; }
    bool isAtLast() const { return currentIndex >= static_cast<int>(moves.size()) - 1; }
    
    // 清空历史
    void clear();
    
    // 删除当前位置之后的所有走法（用于分支处理）
    void truncateAfterCurrent();
    
    // 获取走法列表
    const std::vector<Move>& getMoves() const { return moves; }
    
    // 导出/导入
    std::string toPGN() const;
    std::string toMoveList() const;
    bool fromMoveList(const std::string& moveList);
    
    // 搜索
    std::vector<int> findMoves(const Move& move) const;
    
    // 统计信息
    int getRedMoveCount() const;
    int getBlackMoveCount() const;
    
private:
    std::vector<Move> moves;    // 走法列表
    int currentIndex;           // 当前位置索引（-1表示初始位置）
    
    // 辅助函数
    std::string moveToChineseNotation(const Move& move, int moveNumber) const;
    std::string pieceTypeToChineseName(PieceType piece) const;
};

#endif // MOVEHISTORY_H