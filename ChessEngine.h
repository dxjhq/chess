#ifndef CHESSENGINE_H
#define CHESSENGINE_H

#include <vector>
#include <string>
#include <stack>

// 棋子类型枚举（与Chess.h保持一致）
enum PieceType {
    NONE = 0,
    RED_KING = 1, RED_ADVISOR = 2, RED_BISHOP = 3, RED_KNIGHT = 4, RED_ROOK = 5, RED_CANNON = 6, RED_PAWN = 7,
    BLACK_KING = 8, BLACK_ADVISOR = 9, BLACK_BISHOP = 10, BLACK_KNIGHT = 11, BLACK_ROOK = 12, BLACK_CANNON = 13, BLACK_PAWN = 14
};

// 走法结构
struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    PieceType movingPiece;
    PieceType capturedPiece;
    
    Move() : fromRow(-1), fromCol(-1), toRow(-1), toCol(-1), movingPiece(NONE), capturedPiece(NONE) {}
    Move(int fr, int fc, int tr, int tc, PieceType mp = NONE, PieceType cp = NONE)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), movingPiece(mp), capturedPiece(cp) {}
    
    bool isValid() const { return fromRow >= 0 && fromCol >= 0 && toRow >= 0 && toCol >= 0; }
    std::string toString() const;
};

// 象棋引擎类
class ChessEngine {
public:
    ChessEngine();
    ~ChessEngine();
    
    // 初始化棋盘
    void initializeBoard();
    
    // 走法验证
    bool isValidMove(const Move& move) const;
    bool isValidMoveIgnoreCheck(const Move& move) const;
    
    // 生成所有合法走法
    std::vector<Move> generateLegalMoves(bool forRed = true) const;
    
    // 执行走法
    bool makeMove(const Move& move);
    
    // 撤销走法
    bool undoMove();
    
    // 检查将军/将死
    bool isInCheck(bool isRed) const;
    bool isCheckmate(bool isRed) const;
    bool isStalemate(bool isRed) const;
    
    // 获取棋盘状态
    PieceType getPiece(int row, int col) const;
    void setPiece(int row, int col, PieceType piece);
    
    // 获取当前轮到谁下棋
    bool isRedTurn() const { return redToMove; }
    void setRedTurn(bool red) { redToMove = red; }
    
    // FEN字符串支持
    std::string toFEN() const;
    bool fromFEN(const std::string& fen);
    
    // 获取走法历史
    const std::vector<Move>& getMoveHistory() const { return moveHistory; }
    
    // 清空棋盘
    void clearBoard();
    
    // 复制棋盘状态
    void copyBoard(PieceType destBoard[10][9]) const;
    void setBoard(const PieceType srcBoard[10][9]);
    
private:
    static const int BOARD_ROWS = 10;
    static const int BOARD_COLS = 9;
    
    PieceType board[BOARD_ROWS][BOARD_COLS];
    std::vector<Move> moveHistory;
    bool redToMove;
    
    // 辅助函数
    bool isRed(PieceType piece) const;
    bool isBlack(PieceType piece) const;
    bool isSameColor(PieceType p1, PieceType p2) const;
    bool isInBounds(int row, int col) const;
    
    // 各种棋子的走法验证
    bool isValidKingMove(const Move& move) const;
    bool isValidAdvisorMove(const Move& move) const;
    bool isValidBishopMove(const Move& move) const;
    bool isValidKnightMove(const Move& move) const;
    bool isValidRookMove(const Move& move) const;
    bool isValidCannonMove(const Move& move) const;
    bool isValidPawnMove(const Move& move) const;
    
    // 检查路径是否被阻挡
    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) const;
    int countPiecesBetween(int fromRow, int fromCol, int toRow, int toCol) const;
    
    // 检查将军
    bool wouldBeInCheck(const Move& move, bool isRed) const;
    bool isKingFacingKing() const;
    
    // 生成特定棋子的走法
    void generateKingMoves(int row, int col, std::vector<Move>& moves) const;
    void generateAdvisorMoves(int row, int col, std::vector<Move>& moves) const;
    void generateBishopMoves(int row, int col, std::vector<Move>& moves) const;
    void generateKnightMoves(int row, int col, std::vector<Move>& moves) const;
    void generateRookMoves(int row, int col, std::vector<Move>& moves) const;
    void generateCannonMoves(int row, int col, std::vector<Move>& moves) const;
    void generatePawnMoves(int row, int col, std::vector<Move>& moves) const;
};

#endif // CHESSENGINE_H