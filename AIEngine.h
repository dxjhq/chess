#ifndef AIENGINE_H
#define AIENGINE_H

#include "ChessEngine.h"
#include <vector>
#include <unordered_map>
#include <chrono>
#include <random>

// AI难度级别
enum AIDifficulty {
    AI_EASY = 1,      // 简单：深度2，随机性高
    AI_MEDIUM = 2,    // 中等：深度4，中等随机性
    AI_HARD = 3,      // 困难：深度6，低随机性
    AI_EXPERT = 4     // 专家：深度8，无随机性
};

// AI思考状态
enum AIThinkingState {
    AI_IDLE,
    AI_THINKING,
    AI_FINISHED
};

// 评估结果结构
struct EvaluationResult {
    int score;           // 局面评分
    Move bestMove;       // 最佳走法
    int depth;           // 搜索深度
    int nodesSearched;   // 搜索节点数
    double timeUsed;     // 用时（秒）
    
    EvaluationResult() : score(0), depth(0), nodesSearched(0), timeUsed(0.0) {}
};

// 置换表项
struct TranspositionEntry {
    uint64_t hash;       // 局面哈希值
    int score;           // 评分
    int depth;           // 搜索深度
    Move bestMove;       // 最佳走法
    enum NodeType { EXACT, LOWER_BOUND, UPPER_BOUND } type;
    
    TranspositionEntry() : hash(0), score(0), depth(0), type(EXACT) {}
};

// AI引擎类
class AIEngine {
public:
    AIEngine();
    ~AIEngine();
    
    // AI设置
    void setDifficulty(AIDifficulty difficulty);
    AIDifficulty getDifficulty() const { return difficulty; }
    
    void setMaxDepth(int depth) { maxDepth = depth; }
    int getMaxDepth() const { return maxDepth; }
    
    void setTimeLimit(double seconds) { timeLimit = seconds; }
    double getTimeLimit() const { return timeLimit; }
    
    void setRandomness(double factor) { randomnessFactor = factor; }
    double getRandomness() const { return randomnessFactor; }
    
    // AI思考
    Move getBestMove(const ChessEngine& engine, bool forRed = false);
    EvaluationResult analyzePosition(const ChessEngine& engine, bool forRed = false);
    
    // 异步思考（用于UI响应）
    void startThinking(const ChessEngine& engine, bool forRed = false);
    bool isThinking() const { return thinkingState == AI_THINKING; }
    bool hasResult() const { return thinkingState == AI_FINISHED; }
    Move getThinkingResult();
    void stopThinking();
    
    // 局面评估
    int evaluatePosition(const ChessEngine& engine, bool forRed = false);
    
    // 开局库
    bool hasOpeningMove(const ChessEngine& engine, Move& move);
    void loadOpeningBook(const std::string& filename);
    
    // 残局库
    bool hasEndgameMove(const ChessEngine& engine, Move& move);
    
    // 统计信息
    int getNodesSearched() const { return nodesSearched; }
    double getLastThinkingTime() const { return lastThinkingTime; }
    void clearStatistics();
    
    // 调试功能
    void setDebugMode(bool enabled) { debugMode = enabled; }
    std::string getSearchInfo() const;
    
private:
    // AI参数
    AIDifficulty difficulty;
    int maxDepth;
    double timeLimit;
    double randomnessFactor;
    
    // 搜索状态
    AIThinkingState thinkingState;
    EvaluationResult currentResult;
    std::chrono::steady_clock::time_point searchStartTime;
    bool shouldStop;
    
    // 统计信息
    int nodesSearched;
    double lastThinkingTime;
    bool debugMode;
    
    // 置换表
    std::unordered_map<uint64_t, TranspositionEntry> transpositionTable;
    static const size_t MAX_TT_SIZE = 1000000;  // 最大置换表大小
    
    // 开局库
    std::unordered_map<std::string, std::vector<Move>> openingBook;
    
    // 随机数生成器
    std::mt19937 randomGenerator;
    
    // 核心搜索算法
    int alphaBeta(ChessEngine& engine, int depth, int alpha, int beta, bool maximizing);
    int quiescenceSearch(ChessEngine& engine, int alpha, int beta, bool maximizing, int qDepth = 0);
    
    // 走法排序
    void orderMoves(std::vector<Move>& moves, const ChessEngine& engine, const Move& hashMove);
    int getMoveOrderScore(const Move& move, const ChessEngine& engine);
    
    // 评估函数组件
    int evaluateMaterial(const ChessEngine& engine, bool forRed);
    int evaluatePosition(const ChessEngine& engine, bool forRed);
    int evaluateMobility(const ChessEngine& engine, bool forRed);
    int evaluateKingSafety(const ChessEngine& engine, bool forRed);
    int evaluatePawnStructure(const ChessEngine& engine, bool forRed);
    int evaluateControl(const ChessEngine& engine, bool forRed);
    
    // 棋子价值表
    static const int PIECE_VALUES[15];
    static const int POSITION_VALUES[15][10][9];
    
    // 哈希函数
    uint64_t computeHash(const ChessEngine& engine);
    static uint64_t zobristTable[10][9][15];
    static bool zobristInitialized;
    static void initializeZobrist();
    
    // 时间管理
    bool isTimeUp() const;
    
    // 辅助函数
    bool isCapture(const Move& move, const ChessEngine& engine);
    bool isCheck(const Move& move, ChessEngine& engine);
    bool isQuiet(const Move& move, const ChessEngine& engine);
    
    // 开局库辅助
    std::string positionToString(const ChessEngine& engine);
    
    // 调试输出
    void debugPrint(const std::string& message) const;
};

#endif // AIENGINE_H