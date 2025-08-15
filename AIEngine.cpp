#include "AIEngine.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <climits>

// 棋子价值表（厘兵为单位）
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
const int AIEngine::POSITION_VALUES[15][10][9] = {
    // NONE
    {{{0}}},
    // RED_KING - 帅的位置价值
    {
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0}},
        {{0,0,0,1,3,1,0,0,0}},
        {{0,0,0,2,5,2,0,0,0}},
        {{0,0,0,1,3,1,0,0,0}}
    },
    // 其他棋子的位置价值表（简化处理）
    {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}},
    {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}, {{{0}}}
};

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
    
    // 获取所有合法走法
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
            
            // 执行走法
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
        
        // 如果完成了这个深度的搜索，更新最佳走法
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
            std::uniform_int_distribution<> dist(0, goodMoves.size() - 1);
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
    
    if (isTimeUp() || depth <= 0) {
        return quiescenceSearch(engine, alpha, beta, maximizing);
    }
    
    // 检查置换表
    uint64_t hash = computeHash(engine);
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        const TranspositionEntry& entry = it->second;
        if (entry.type == TranspositionEntry::EXACT) {
            return entry.score;
        } else if (entry.type == TranspositionEntry::LOWER_BOUND && entry.score >= beta) {
            return entry.score;
        } else if (entry.type == TranspositionEntry::UPPER_BOUND && entry.score <= alpha) {
            return entry.score;
        }
    }
    
    std::vector<Move> moves = engine.generateLegalMoves(maximizing);
    
    if (moves.empty()) {
        // 检查是否被将死
        if (engine.isInCheck(maximizing)) {
            return maximizing ? (INT_MIN + depth) : (INT_MAX - depth);
        } else {
            return 0; // 和棋
        }
    }
    
    // 走法排序
    Move hashMove;
    if (it != transpositionTable.end()) {
        hashMove = it->second.bestMove;
    }
    orderMoves(moves, engine, hashMove);
    
    int bestScore = maximizing ? INT_MIN : INT_MAX;
    Move bestMove;
    
    for (const Move& move : moves) {
        if (isTimeUp()) break;
        
        if (!engine.makeMove(move)) continue;
        
        int score = alphaBeta(engine, depth - 1, alpha, beta, !maximizing);
        
        engine.undoMove();
        
        if (maximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
            alpha = std::max(alpha, score);
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
            beta = std::min(beta, score);
        }
        
        if (beta <= alpha) break;
    }
    
    // 存储到置换表
    if (transpositionTable.size() < MAX_TT_SIZE) {
        TranspositionEntry entry;
        entry.hash = hash;
        entry.score = bestScore;
        entry.depth = depth;
        entry.bestMove = bestMove;
        
        if (bestScore <= alpha) {
            entry.type = TranspositionEntry::UPPER_BOUND;
        } else if (bestScore >= beta) {
            entry.type = TranspositionEntry::LOWER_BOUND;
        } else {
            entry.type = TranspositionEntry::EXACT;
        }
        
        transpositionTable[hash] = entry;
    }
    
    return bestScore;
}

int AIEngine::quiescenceSearch(ChessEngine& engine, int alpha, int beta, bool maximizing, int qDepth) {
    nodesSearched++;
    
    if (qDepth > 4) { // 限制静态搜索深度
        return evaluatePosition(engine, maximizing);
    }
    
    int standPat = evaluatePosition(engine, maximizing);
    
    if (maximizing) {
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);
    } else {
        if (standPat <= alpha) return alpha;
        beta = std::min(beta, standPat);
    }
    
    // 只考虑吃子走法
    std::vector<Move> moves = engine.generateLegalMoves(maximizing);
    std::vector<Move> captures;
    
    for (const Move& move : moves) {
        if (isCapture(move, engine)) {
            captures.push_back(move);
        }
    }
    
    orderMoves(captures, engine, Move());
    
    for (const Move& move : captures) {
        if (!engine.makeMove(move)) continue;
        
        int score = quiescenceSearch(engine, alpha, beta, !maximizing, qDepth + 1);
        
        engine.undoMove();
        
        if (maximizing) {
            alpha = std::max(alpha, score);
        } else {
            beta = std::min(beta, score);
        }
        
        if (beta <= alpha) break;
    }
    
    return maximizing ? alpha : beta;
}

int AIEngine::evaluatePosition(const ChessEngine& engine, bool forRed) {
    int score = 0;
    
    // 子力评估
    score += evaluateMaterial(engine, forRed);
    
    // 位置评估
    score += evaluatePosition(engine, forRed);
    
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
            PieceType piece = engine.getPiece(row, col);
            if (piece != NONE) {
                score += PIECE_VALUES[piece];
            }
        }
    }
    
    return score;
}

int AIEngine::evaluateMobility(const ChessEngine& engine, bool forRed) {
    ChessEngine tempEngine = engine;
    
    std::vector<Move> redMoves = tempEngine.generateLegalMoves(true);
    std::vector<Move> blackMoves = tempEngine.generateLegalMoves(false);
    
    int mobilityScore = (redMoves.size() - blackMoves.size()) * 2;
    
    return forRed ? mobilityScore : -mobilityScore;
}

int AIEngine::evaluateKingSafety(const ChessEngine& engine, bool forRed) {
    int score = 0;
    
    // 简单的王安全评估：检查是否被将军
    if (engine.isInCheck(true)) {
        score -= 50;
    }
    if (engine.isInCheck(false)) {
        score += 50;
    }
    
    return forRed ? score : -score;
}

void AIEngine::orderMoves(std::vector<Move>& moves, const ChessEngine& engine, const Move& hashMove) {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        // 优先考虑置换表中的最佳走法
        if (hashMove.isValid()) {
            if (a.fromRow == hashMove.fromRow && a.fromCol == hashMove.fromCol &&
                a.toRow == hashMove.toRow && a.toCol == hashMove.toCol) {
                return true;
            }
            if (b.fromRow == hashMove.fromRow && b.fromCol == hashMove.fromCol &&
                b.toRow == hashMove.toRow && b.toCol == hashMove.toCol) {
                return false;
            }
        }
        
        return getMoveOrderScore(a, engine) > getMoveOrderScore(b, engine);
    });
}

int AIEngine::getMoveOrderScore(const Move& move, const ChessEngine& engine) {
    int score = 0;
    
    // 吃子走法优先
    if (isCapture(move, engine)) {
        PieceType captured = engine.getPiece(move.toRow, move.toCol);
        PieceType moving = engine.getPiece(move.fromRow, move.fromCol);
        score += std::abs(PIECE_VALUES[captured]) - std::abs(PIECE_VALUES[moving]) / 10;
    }
    
    // 将军走法优先
    ChessEngine tempEngine = engine;
    if (tempEngine.makeMove(move)) {
        if (tempEngine.isInCheck(!tempEngine.isRedTurn())) {
            score += 100;
        }
        tempEngine.undoMove();
    }
    
    return score;
}

bool AIEngine::hasOpeningMove(const ChessEngine& engine, Move& move) {
    std::string position = positionToString(engine);
    auto it = openingBook.find(position);
    
    if (it != openingBook.end() && !it->second.empty()) {
        std::uniform_int_distribution<> dist(0, it->second.size() - 1);
        move = it->second[dist(randomGenerator)];
        return true;
    }
    
    return false;
}

bool AIEngine::hasEndgameMove(const ChessEngine& engine, Move& move) {
    // 简单的残局处理：当棋子数量很少时
    int pieceCount = 0;
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            if (engine.getPiece(row, col) != NONE) {
                pieceCount++;
            }
        }
    }
    
    // 如果棋子数量少于10个，认为是残局
    if (pieceCount < 10) {
        // 这里可以实现残局库查询
        // 暂时返回false
    }
    
    return false;
}

uint64_t AIEngine::computeHash(const ChessEngine& engine) {
    uint64_t hash = 0;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            PieceType piece = engine.getPiece(row, col);
            if (piece != NONE) {
                hash ^= zobristTable[row][col][piece];
            }
        }
    }
    
    if (engine.isRedTurn()) {
        hash ^= zobristTable[0][0][0]; // 用特殊值表示轮到红方
    }
    
    return hash;
}

void AIEngine::initializeZobrist() {
    std::mt19937_64 gen(12345); // 固定种子确保一致性
    std::uniform_int_distribution<uint64_t> dist;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            for (int piece = 0; piece < 15; piece++) {
                zobristTable[row][col][piece] = dist(gen);
            }
        }
    }
    
    zobristInitialized = true;
}

bool AIEngine::isTimeUp() const {
    if (timeLimit <= 0) return false;
    
    auto currentTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(currentTime - searchStartTime).count();
    return elapsed >= timeLimit;
}

bool AIEngine::isCapture(const Move& move, const ChessEngine& engine) {
    return engine.getPiece(move.toRow, move.toCol) != NONE;
}

std::string AIEngine::positionToString(const ChessEngine& engine) {
    return engine.toFEN();
}

void AIEngine::debugPrint(const std::string& message) const {
    if (debugMode) {
        std::cout << "[AI] " << message << std::endl;
    }
}

void AIEngine::clearStatistics() {
    nodesSearched = 0;
    lastThinkingTime = 0.0;
    transpositionTable.clear();
}

EvaluationResult AIEngine::analyzePosition(const ChessEngine& engine, bool forRed) {
    EvaluationResult result;
    
    auto startTime = std::chrono::steady_clock::now();
    result.bestMove = getBestMove(engine, forRed);
    auto endTime = std::chrono::steady_clock::now();
    
    result.score = evaluatePosition(engine, forRed);
    result.depth = maxDepth;
    result.nodesSearched = nodesSearched;
    result.timeUsed = std::chrono::duration<double>(endTime - startTime).count();
    
    return result;
}

std::string AIEngine::getSearchInfo() const {
    std::ostringstream oss;
    oss << "深度: " << maxDepth
        << ", 节点: " << nodesSearched
        << ", 时间: " << lastThinkingTime << "s"
        << ", 置换表: " << transpositionTable.size();
    return oss.str();
}