#include "ChessEngine.h"
#include "MoveHistory.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== 象棋引擎测试程序 ===" << std::endl;
    
    // 测试ChessEngine
    ChessEngine engine;
    engine.initializeBoard();
    std::cout << "\n1. 初始化棋盘:" << std::endl;
    std::cout << "FEN: " << engine.toFEN() << std::endl;
    std::cout << "轮到红方: " << (engine.isRedTurn() ? "是" : "否") << std::endl;
    
    // 测试走法生成
    std::cout << "\n2. 生成红方合法走法:" << std::endl;
    std::vector<Move> redMoves = engine.generateLegalMoves(true);
    std::cout << "红方有 " << redMoves.size() << " 个合法走法" << std::endl;
    
    // 显示前5个走法
    for (int i = 0; i < std::min(5, (int)redMoves.size()); i++) {
        std::cout << "  " << (i+1) << ". " << redMoves[i].toString() << std::endl;
    }
    
    // 测试走法执行
    std::cout << "\n3. 执行走法测试:" << std::endl;
    if (!redMoves.empty()) {
        Move firstMove = redMoves[0];
        std::cout << "执行走法: " << firstMove.toString() << std::endl;
        
        if (engine.makeMove(firstMove)) {
            std::cout << "走法执行成功" << std::endl;
            std::cout << "现在轮到: " << (engine.isRedTurn() ? "红方" : "黑方") << std::endl;
            
            // 撤销走法
            std::cout << "\n4. 撤销走法测试:" << std::endl;
            if (engine.undoMove()) {
                std::cout << "撤销成功" << std::endl;
                std::cout << "现在轮到: " << (engine.isRedTurn() ? "红方" : "黑方") << std::endl;
            }
        }
    }
    
    // 测试MoveHistory
    std::cout << "\n5. 测试走法历史:" << std::endl;
    MoveHistory history;
    
    // 添加几个测试走法
    if (redMoves.size() >= 2) {
        Move move1 = redMoves[0];
        Move move2 = redMoves[1];
        
        history.addMove(move1);
        history.addMove(move2);
        
        std::cout << "添加了 " << history.getMoveCount() << " 个走法" << std::endl;
        std::cout << "走法列表: " << history.toMoveList() << std::endl;
        
        // 测试导航
        std::cout << "\n6. 测试历史导航:" << std::endl;
        std::cout << "当前位置: " << history.getCurrentIndex() << std::endl;
        std::cout << "是否在开始: " << (history.isAtFirst() ? "是" : "否") << std::endl;
        std::cout << "是否在结尾: " << (history.isAtLast() ? "是" : "否") << std::endl;
        
        if (history.goToPrevious()) {
            std::cout << "后退成功，当前位置: " << history.getCurrentIndex() << std::endl;
        }
        
        if (history.goToNext()) {
            std::cout << "前进成功，当前位置: " << history.getCurrentIndex() << std::endl;
        }
    }
    
    // 测试特殊局面
    std::cout << "\n7. 测试将军检测:" << std::endl;
    std::cout << "红方被将军: " << (engine.isInCheck(true) ? "是" : "否") << std::endl;
    std::cout << "黑方被将军: " << (engine.isInCheck(false) ? "是" : "否") << std::endl;
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}