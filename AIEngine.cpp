#include "AIEngine.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <climits>

// 棋子价值表
const int AIEngine::PIECE_VALUES[15] = {
    0,      // NONE
    10000,  // RED_KING
    200,    // RED_ADVISOR
    200,    // RED_BISHOP
    400,    // RED_KNIGHT
    600,    // RED_ROOK
    300,    // RED_CANNON
    100,    // RED_PAWN
    -10000, // BLACK_KING
    -200,   // BLACK_ADVISOR
    -200,   // BLACK_BISHOP
    -400,   // BLACK_KNIGHT
    -600,   // BLACK_ROOK
    -300,   // BLACK_CANNON
    -100    // BLACK_PAWN
};

// 位置价值表（简化版本）
const int AIEngine::POSITION_VALUES[15][10][9] = {};

// Zobrist哈希表
uint64_t AIEngine::zobristTable[10][9][15];
bool AIEngine::zobristInitialized = false;

AIEngine::AIEngine() 
    : difficulty(AI_MEDIUM), maxDepth(4), timeLimit(5.0), randomnessFactor(0.1),
      thinkingState(AI_IDLE), shouldStop(false), nodesSearched(0), 
      lastThinkingTime(0.0), debugMode(false), randomGenerator(std::chrono::steady_clock::now().time_since_epoch().count())
{
    if (!zobristInitialized) {
        initializeZobrist();
    }
}

AIEngine::~AIEngine() {
}

void AIEngine::setDifficulty(AIDifficulty diff) {
    difficulty = diff;
    
    // 根据难度设置参数
    switch (difficulty) {
        case AI_EASY:
            maxDepth = 2;
            randomnessFactor = 0.3;
            timeLimit = 1.0;
            break;
        case AI_MEDIUM:
            maxDepth = 4;
            randomnessFactor = 0.1;
            timeLimit = 3.0;
            break;
        case AI_HARD:
            maxDepth = 6;
            randomnessFactor = 0.05;
            timeLimit = 5.0;
            break;
        case AI_EXPERT:
            maxDepth = 8;
            randomnessFactor = 0.0;
            timeLimit = 10.0;
            break;
    }
}

Move AIEngine::getBestMove(const ChessEngine& engine, bool forRed) {
    debugPrint("开始AI思考...");
    
    // 重置统计信息
    nodesSearched = 0;
    searchStartTime = std::chrono::steady_clock::now();
    shouldStop = false;
    
    // 检查开局库
    Move openingMove;
    if (hasOpeningMove(engine, openingMove)) {
        debugPrint("使用开局库走法");
        return openingMove;
    }
    
    // 检查残局库
    Move endgameMove;
    if (hasEndgameMove(engine, endgameMove)) {
        debugPrint("使用残局库走法");
        return endgameMove;
    }
    
    // 生成所有合法走法
    ChessEngine tempEngine = engine;
    std::vector<Move> legalMoves = tempEngine.generateLegalMoves(forRed);
    
    if (legalMoves.empty()) {
        debugPrint("没有合法走法");
        return Move();
    }
    
    if (legalMoves.size() == 1) {
        debugPrint("只有一个合法走法");
        return legalMoves[0];
    }
    
    Move bestMove;
    int bestScore = forRed ? INT_MIN : INT_MAX;
    
    // 迭代加深搜索
    for (int depth = 1; depth <= maxDepth && !isTimeUp(); depth++) {
        debugPrint("搜索深度: " + std::to_string(depth));
        
        int alpha = INT_MIN;
        int beta = INT_MAX;
        Move currentBestMove;
        int currentBestScore = forRed ? INT_MIN : INT_MAX;
        
        // 走法排序
        orderMoves(legalMoves, tempEngine, bestMove);
        
        for (const Move& move : legalMoves) {
            if (isTimeUp()) break;
            
            // 尝试走法
            if (!tempEngine.makeMove(move)) continue;
            
            // 搜索
            int score = alphaBeta(tempEngine, depth - 1, alpha, beta, !forRed);
            
            // 撤销走法
            tempEngine.undoMove();
            
            // 更新最佳走法
            if (forRed) {
                if (score > currentBestScore) {
                    currentBestScore = score;
                    currentBestMove = move;
                }
                alpha = std::max(alpha, score);
            } else {
                if (score < currentBestScore) {
                    currentBestScore = score;
                    currentBestMove = move;
                }
                beta = std::min(beta, score);
            }
            
            if (beta <= alpha) break; // Alpha-Beta剪枝
        }
        
        // 如果没有超时，更新最佳走法
        if (!isTimeUp()) {
            bestMove = currentBestMove;
            bestScore = currentBestScore;
        }
    }
    
    // 添加随机性
    if (randomnessFactor > 0.0) {
        std::vector<Move> goodMoves;
        int threshold = bestScore - static_cast<int>(100 * randomnessFactor);
        
        for (const Move& move : legalMoves) {
            ChessEngine testEngine = tempEngine;
            if (testEngine.makeMove(move)) {
                int score = evaluatePosition(testEngine, forRed);
                if (forRed ? (score >= threshold) : (score <= threshold)) {
                    goodMoves.push_back(move);
                }
            }
        }
        
        if (!goodMoves.empty()) {
            std::uniform_int_distribution<> dist(0, static_cast<int>(goodMoves.size()) - 1);
            bestMove = goodMoves[dist(randomGenerator)];
        }
    }
    
    // 记录思考时间
    auto endTime = std::chrono::steady_clock::now();
    lastThinkingTime = std::chrono::duration<double>(endTime - searchStartTime).count();
    
    debugPrint("AI思考完成，用时: " + std::to_string(lastThinkingTime) + "秒");
    debugPrint("搜索节点数: " + std::to_string(nodesSearched));
    
    return bestMove;
}

int AIEngine::alphaBeta(ChessEngine& engine, int depth, int alpha, int beta, bool maximizing) {
    nodesSearched++;
    
    if (depth == 0 || isTimeUp()) {
        return quiescenceSearch(engine, alpha, beta, maximizing);
    }
    
    std::vector<Move> moves = engine.generateLegalMoves(maximizing);
    if (moves.empty()) {
        // 无子可走，判断是否被将军
        if (engine.isInCheck(maximizing)) {
            return maximizing ? -10000 + depth : 10000 - depth; // 被将死
        } else {
            return 0; // 和棋
        }
    }
    
    orderMoves(moves, engine, Move());
    
    if (maximizing) {
        int maxEval = INT_MIN;
        for (const Move& move : moves) {
            if (isTimeUp()) break;
            
            if (engine.makeMove(move)) {
                int eval = alphaBeta(engine, depth - 1, alpha, beta, false);
                engine.undoMove();
                
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                
                if (beta <= alpha) break;
            }
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (const Move& move : moves) {
            if (isTimeUp()) break;
            
            if (engine.makeMove(move)) {
                int eval = alphaBeta(engine, depth - 1, alpha, beta, true);
                engine.undoMove();
                
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                
                if (beta <= alpha) break;
            }
        }
        return minEval;
    }
}

int AIEngine::quiescenceSearch(ChessEngine& engine, int alpha, int beta, bool maximizing, int qDepth) {
    nodesSearched++;
    
    int standPat = evaluatePosition(engine, maximizing);
    
    if (qDepth > 4) return standPat;
    
    if (maximizing) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
    }
    
    std::vector<Move> captures;
    std::vector<Move> allMoves = engine.generateLegalMoves(maximizing);
    
    for (const Move& move : allMoves) {
        if (isCapture(move, engine)) {
            captures.push_back(move);
        }
    }
    
    if (captures.empty()) return standPat;
    
    orderMoves(captures, engine, Move());
    
    for (const Move& move : captures) {
        if (engine.makeMove(move)) {
            int score = quiescenceSearch(engine, alpha, beta, !maximizing, qDepth + 1);
            engine.undoMove();
            
            if (maximizing) {
                if (score >= beta) return beta;
                alpha = std::max(alpha, score);
            } else {
                if (score <= alpha) return alpha;
                beta = std::min(beta, score);
            }
        }
    }
    
    return maximizing ? alpha : beta;
}

int AIEngine::evaluatePosition(const ChessEngine& engine, bool forRed) {
    int score = 0;
    
    // 物质评估
    score += evaluateMaterial(engine, forRed);
    
    // 机动性评估
    score += evaluateMobility(engine, forRed);
    
    // 王安全评估
    score += evaluateKingSafety(engine, forRed);
    
    return forRed ? score : -score;
}

int AIEngine::evaluateMaterial(const ChessEngine& engine, bool forRed) {
    int score = 0;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            int piece = engine.getPiece(row, col);
            if (piece != 0) {
                score += PIECE_VALUES[piece];
            }
        }
    }
    
    return score;
}

int AIEngine::evaluateMobility(const ChessEngine& engine, bool forRed) {
    std::vector<Move> redMoves = engine.generateLegalMoves(true);
    std::vector<Move> blackMoves = engine.generateLegalMoves(false);
    
    int mobilityScore = static_cast<int>(redMoves.size()) - static_cast<int>(blackMoves.size());
    return mobilityScore * 2;
}

int AIEngine::evaluateKingSafety(const ChessEngine& engine, bool forRed) {
    int score = 0;
    
    // 简单的王安全评估
    if (engine.isInCheck(true)) score -= 50;
    if (engine.isInCheck(false)) score += 50;
    
    return score;
}

void AIEngine::orderMoves(std::vector<Move>& moves, const ChessEngine& engine, const Move& hashMove) {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return getMoveOrderScore(a, engine) > getMoveOrderScore(b, engine);
    });
}

int AIEngine::getMoveOrderScore(const Move& move, const ChessEngine& engine) {
    int score = 0;
    
    // 优先考虑吃子
    if (isCapture(move, engine)) {
        int capturedPiece = engine.getPiece(move.toRow, move.toCol);
        int movingPiece = engine.getPiece(move.fromRow, move.fromCol);
        score += abs(PIECE_VALUES[capturedPiece]) - abs(PIECE_VALUES[movingPiece]) / 10;
    }
    
    return score;
}

bool AIEngine::hasOpeningMove(const ChessEngine& engine, Move& move) {
    // 简单的开局库实现
    std::string position = positionToString(engine);
    
    auto it = openingBook.find(position);
    if (it != openingBook.end() && !it->second.empty()) {
        std::uniform_int_distribution<> dist(0, static_cast<int>(it->second.size()) - 1);
        move = it->second[dist(randomGenerator)];
        return true;
    }
    
    return false;
}

bool AIEngine::hasEndgameMove(const ChessEngine& engine, Move& move) {
    // 简单的残局库实现
    int pieceCount = 0;
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            if (engine.getPiece(row, col) != 0) {
                pieceCount++;
            }
        }
    }
    
    // 如果棋子数量少于8个，认为是残局
    if (pieceCount < 8) {
        // 这里可以添加残局库查询逻辑
        return false;
    }
    
    return false;
}

uint64_t AIEngine::computeHash(const ChessEngine& engine) {
    uint64_t hash = 0;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            int piece = engine.getPiece(row, col);
            if (piece != 0) {
                hash ^= zobristTable[row][col][piece];
            }
        }
    }
    
    return hash;
}

void AIEngine::initializeZobrist() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            for (int piece = 0; piece < 15; piece++) {
                zobristTable[row][col][piece] = gen();
            }
        }
    }
    
    zobristInitialized = true;
}

bool AIEngine::isTimeUp() const {
    auto currentTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(currentTime - searchStartTime).count();
    return elapsed >= timeLimit || shouldStop;
}

bool AIEngine::isCapture(const Move& move, const ChessEngine& engine) {
    return engine.getPiece(move.toRow, move.toCol) != 0;
}

std::string AIEngine::positionToString(const ChessEngine& engine) {
    return "position_string"; // 简化实现
}

void AIEngine::debugPrint(const std::string& message) const {
    if (debugMode) {
        std::cout << "[AI] " << message << std::endl;
    }
}

void AIEngine::clearStatistics() {
    nodesSearched = 0;
    lastThinkingTime = 0.0;
}

EvaluationResult AIEngine::analyzePosition(const ChessEngine& engine, bool forRed) {
    EvaluationResult result;
    result.score = evaluatePosition(engine, forRed);
    result.bestMove = getBestMove(engine, forRed);
    result.depth = maxDepth;
    result.nodesSearched = this->nodesSearched;
    result.timeUsed = lastThinkingTime;
    return result;
}

std::string AIEngine::getSearchInfo() const {
    return "Nodes: " + std::to_string(nodesSearched) + 
           ", Time: " + std::to_string(lastThinkingTime) + "s";
}

void AIEngine::stopThinking() {
    shouldStop = true;
    thinkingState = AI_IDLE;
}

void AIEngine::startThinking(const ChessEngine& engine, bool forRed) {
    // 简单实现，直接调用getBestMove
    thinkingState = AI_THINKING;
    Move result = getBestMove(engine, forRed);
    thinkingResult = result;
    thinkingState = AI_FINISHED;
}

Move AIEngine::getThinkingResult() {
    if (thinkingState == AI_FINISHED) {
        thinkingState = AI_IDLE;
        return thinkingResult;
    }
    return Move();
}