#include "Chess.h"
#include "ConnectionDialog.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QSplitter>
#include <QFont>
#include <QVBoxLayout>
#include <QComboBox>
// #include <QtSvg/QSvgRenderer>
#include <QPixmap>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QSvgRenderer>
#include <QDebug>
#include <vector>

// PieceStyleManager 实现
PieceStyleManager& PieceStyleManager::getInstance() {
    static PieceStyleManager instance;
    return instance;
}

PieceStyleManager::PieceStyleManager() : currentStyle(STYLE_FONT_TRADITIONAL) {
}

void PieceStyleManager::setPieceStyle(PieceStyle style) {
    currentStyle = style;
}

PieceStyle PieceStyleManager::getPieceStyle() const {
    return currentStyle;
}

QString PieceStyleManager::getPieceText(PieceType piece) const {
    switch (currentStyle) {
        case STYLE_FONT_TRADITIONAL:
            return getTraditionalText(piece);
        case STYLE_FONT_MODERN:
            return getModernText(piece);
        default:
            return getTraditionalText(piece);
    }
}

QString PieceStyleManager::getTraditionalText(PieceType piece) const {
    switch (piece) {
        case RED_KING: return QString(QChar(0x5E25));     // 帥
        case RED_ADVISOR: return QString(QChar(0x4ED5));   // 仕
        case RED_BISHOP: return QString(QChar(0x76F8));    // 相
        case RED_KNIGHT: return QString(QChar(0x99AC));    // 馬
        case RED_ROOK: return QString(QChar(0x8ECA));      // 車
        case RED_CANNON: return QString(QChar(0x70AE));    // 炮
        case RED_PAWN: return QString(QChar(0x5175));      // 兵
        case BLACK_KING: return QString(QChar(0x5C07));    // 將
        case BLACK_ADVISOR: return QString(QChar(0x58EB));  // 士
        case BLACK_BISHOP: return QString(QChar(0x8C61));   // 象
        case BLACK_KNIGHT: return QString(QChar(0x99AC));   // 馬
        case BLACK_ROOK: return QString(QChar(0x8ECA));     // 車
        case BLACK_CANNON: return QString(QChar(0x7832));   // 砲
        case BLACK_PAWN: return QString(QChar(0x5352));     // 卒
        default: return "";
    }
}

QString PieceStyleManager::getModernText(PieceType piece) const {
    switch (piece) {
        case RED_KING: return "K";
        case RED_ADVISOR: return "A";
        case RED_BISHOP: return "B";
        case RED_KNIGHT: return "N";
        case RED_ROOK: return "R";
        case RED_CANNON: return "C";
        case RED_PAWN: return "P";
        case BLACK_KING: return "k";
        case BLACK_ADVISOR: return "a";
        case BLACK_BISHOP: return "b";
        case BLACK_KNIGHT: return "n";
        case BLACK_ROOK: return "r";
        case BLACK_CANNON: return "c";
        case BLACK_PAWN: return "p";
        default: return "";
    }
}

QString PieceStyleManager::getPieceImagePath(PieceType piece) const {
    QString basePath = ":/images/pieces/";
    QString style = (currentStyle == STYLE_IMAGE_CLASSIC) ? "classic/" : "modern/";
    
    switch (piece) {
        case RED_KING: return basePath + style + "red_king.svg";
        case RED_ADVISOR: return basePath + style + "red_advisor.svg";
        case RED_BISHOP: return basePath + style + "red_bishop.svg";
        case RED_KNIGHT: return basePath + style + "red_knight.svg";
        case RED_ROOK: return basePath + style + "red_rook.svg";
        case RED_CANNON: return basePath + style + "red_cannon.svg";
        case RED_PAWN: return basePath + style + "red_pawn.svg";
        case BLACK_KING: return basePath + style + "black_king.svg";
        case BLACK_ADVISOR: return basePath + style + "black_advisor.svg";
        case BLACK_BISHOP: return basePath + style + "black_bishop.svg";
        case BLACK_KNIGHT: return basePath + style + "black_knight.svg";
        case BLACK_ROOK: return basePath + style + "black_rook.svg";
        case BLACK_CANNON: return basePath + style + "black_cannon.svg";
        case BLACK_PAWN: return basePath + style + "black_pawn.svg";
        default: return "";
    }
}

QFont PieceStyleManager::getPieceFont() const {
    switch (currentStyle) {
        case STYLE_FONT_TRADITIONAL:
            return QFont("Microsoft YaHei", 14, QFont::Bold);
        case STYLE_FONT_MODERN:
            return QFont("Arial", 16, QFont::Bold);
        default:
            return QFont("Microsoft YaHei", 14, QFont::Bold);
    }
}

QColor PieceStyleManager::getPieceTextColor(PieceType piece) const {
    if (piece >= RED_KING && piece <= RED_PAWN) {
        return Qt::darkRed;
    } else {
        return Qt::black;
    }
}

QColor PieceStyleManager::getPieceBackgroundColor(PieceType piece) const {
    if (piece >= RED_KING && piece <= RED_PAWN) {
        return QColor(255, 240, 240); // 浅红色
    } else {
        return QColor(240, 240, 240); // 浅灰色
    }
}

// ChessBoard 类实现
ChessBoard::ChessBoard(QWidget *parent)
    : QWidget(parent), selectedPos(-1, -1), pieceSelected(false), mousePos(0, 0), styleManager(PieceStyleManager::getInstance())
{
    setFixedSize(BOARD_WIDTH * CELL_SIZE + 2 * BOARD_MARGIN, 
                 BOARD_HEIGHT * CELL_SIZE + 2 * BOARD_MARGIN);
    setMouseTracking(true);
    initializeBoard();
}

ChessBoard::~ChessBoard()
{
}

void ChessBoard::setPieceStyle(PieceStyle style)
{
    styleManager.setPieceStyle(style);
    update(); // 重新绘制棋盘
}

void ChessBoard::initializeBoard()
{
    engine.initializeBoard();
    moveHistory.clear();
    selectedPos = QPoint(-1, -1);
    pieceSelected = false;
    validMoves.clear();
    updateGameStatus();
    update();
}

bool ChessBoard::makeMove(int fromRow, int fromCol, int toRow, int toCol)
{
    Move move(fromRow, fromCol, toRow, toCol);
    if (engine.makeMove(move)) {
        moveHistory.addMove(move);
        selectedPos = QPoint(-1, -1);
        pieceSelected = false;
        validMoves.clear();
        updateGameStatus();
        emit moveExecuted(move);
        emit boardChanged();
        update();
        return true;
    }
    return false;
}

void ChessBoard::undoMove()
{
    if (engine.undoMove()) {
        if (!moveHistory.isEmpty()) {
            moveHistory.goToPrevious();
        }
        selectedPos = QPoint(-1, -1);
        pieceSelected = false;
        validMoves.clear();
        updateGameStatus();
        emit boardChanged();
        update();
    }
}

void ChessBoard::redoMove()
{
    if (moveHistory.goToNext()) {
        Move move = moveHistory.getCurrentMove();
        if (move.isValid()) {
            engine.makeMove(move);
            updateGameStatus();
            emit boardChanged();
            update();
        }
    }
}

void ChessBoard::goToMove(int index)
{
    if (moveHistory.goToMove(index)) {
        // 重建棋盘状态到指定走法
        engine.initializeBoard();
        for (int i = 0; i <= index; i++) {
            Move move = moveHistory.getMove(i);
            if (move.isValid()) {
                engine.makeMove(move);
            }
        }
        selectedPos = QPoint(-1, -1);
        pieceSelected = false;
        validMoves.clear();
        updateGameStatus();
        emit boardChanged();
        update();
    }
}

bool ChessBoard::isGameOver() const
{
    bool redTurn = engine.isRedTurn();
    return engine.isCheckmate(redTurn) || engine.isStalemate(redTurn);
}

bool ChessBoard::isRedTurn() const
{
    return engine.isRedTurn();
}

QString ChessBoard::getGameStatus() const
{
    return gameStatus;
}

void ChessBoard::setBoard(const PieceType board[10][9])
{
    engine.setBoard(board);
    selectedPos = QPoint(-1, -1);
    pieceSelected = false;
    validMoves.clear();
    updateGameStatus();
    update();
}

void ChessBoard::copyBoard(PieceType board[10][9]) const
{
    engine.copyBoard(board);
}

void ChessBoard::setMoveHistory(const MoveHistory& history)
{
    moveHistory = history;
    // 重建棋盘状态
    engine.initializeBoard();
    for (int i = 0; i <= moveHistory.getCurrentIndex(); i++) {
        Move move = moveHistory.getMove(i);
        if (move.isValid()) {
            engine.makeMove(move);
        }
    }
    updateGameStatus();
    update();
}

void ChessBoard::updateGameStatus()
{
    bool redTurn = engine.isRedTurn();
    
    if (engine.isCheckmate(redTurn)) {
        gameStatus = redTurn ? "红方被将死，黑方获胜" : "黑方被将死，红方获胜";
    } else if (engine.isStalemate(redTurn)) {
        gameStatus = "和棋";
    } else if (engine.isInCheck(redTurn)) {
        gameStatus = redTurn ? "红方被将军" : "黑方被将军";
    } else {
        gameStatus = redTurn ? "轮到红方" : "轮到黑方";
    }
    
    emit gameStatusChanged(gameStatus);
}

void ChessBoard::highlightValidMoves()
{
    validMoves.clear();
    if (pieceSelected && selectedPos.x() >= 0 && selectedPos.y() >= 0) {
        PieceType piece = engine.getPiece(selectedPos.x(), selectedPos.y());
        if (piece != NONE) {
            std::vector<Move> allMoves = engine.generateLegalMoves(engine.isRedTurn());
            for (const Move& move : allMoves) {
                if (move.fromRow == selectedPos.x() && move.fromCol == selectedPos.y()) {
                    validMoves.push_back(move);
                }
            }
        }
    }
}

void ChessBoard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置背景色
    painter.fillRect(rect(), QColor(245, 222, 179)); // 木色背景
    
    drawBoard(painter);
    drawPieces(painter);
    drawSelection(painter);
    drawValidMoves(painter);
}

void ChessBoard::drawBoard(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 2));
    
    // 绘制棋盘线条
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        QPoint start = boardToPixel(i, 0);
        QPoint end = boardToPixel(i, BOARD_WIDTH - 1);
        painter.drawLine(start, end);
    }
    
    for (int j = 0; j < BOARD_WIDTH; j++) {
        QPoint start = boardToPixel(0, j);
        QPoint end = boardToPixel(BOARD_HEIGHT - 1, j);
        
        // 中间的河界不连通
        if (j == 0 || j == BOARD_WIDTH - 1) {
            painter.drawLine(start, end);
        } else {
            painter.drawLine(start, boardToPixel(4, j));
            painter.drawLine(boardToPixel(5, j), end);
        }
    }
    
    // 绘制九宫格对角线
    painter.setPen(QPen(Qt::black, 1));
    
    // 上方九宫格
    painter.drawLine(boardToPixel(0, 3), boardToPixel(2, 5));
    painter.drawLine(boardToPixel(0, 5), boardToPixel(2, 3));
    
    // 下方九宫格
    painter.drawLine(boardToPixel(7, 3), boardToPixel(9, 5));
    painter.drawLine(boardToPixel(7, 5), boardToPixel(9, 3));
    
    // 绘制河界文字
    painter.setPen(QPen(Qt::darkRed, 2));
    QFont font("SimHei", 16, QFont::Bold);
    painter.setFont(font);
    
    QPoint riverLeft = boardToPixel(4, 1);
    QPoint riverRight = boardToPixel(4, 7);
    riverLeft.setY(riverLeft.y() + CELL_SIZE / 2 + 8);
    riverRight.setY(riverRight.y() + CELL_SIZE / 2 + 8);
    
    painter.drawText(riverLeft, "楚河");
    painter.drawText(riverRight, "汉界");
}

void ChessBoard::drawPieces(QPainter &painter)
{
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            PieceType piece = engine.getPiece(i, j);
            if (piece != NONE) {
                drawPiece(painter, i, j, piece);
            }
        }
    }
}

void ChessBoard::drawSelection(QPainter &painter)
{
    if (pieceSelected && selectedPos.x() >= 0 && selectedPos.y() >= 0) {
        QPoint center = boardToPixel(selectedPos.x(), selectedPos.y());
        painter.setPen(QPen(Qt::red, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center.x() - 22, center.y() - 22, 44, 44);
    }
}

void ChessBoard::drawValidMoves(QPainter &painter)
{
    painter.setPen(QPen(Qt::green, 2));
    painter.setBrush(QBrush(QColor(0, 255, 0, 50)));
    
    for (const Move& move : validMoves) {
        QPoint center = boardToPixel(move.toRow, move.toCol);
        painter.drawEllipse(center.x() - 15, center.y() - 15, 30, 30);
    }
}

void ChessBoard::drawPiece(QPainter &painter, int row, int col, PieceType piece)
{
    PieceStyle style = styleManager.getPieceStyle();
    
    if (style == STYLE_IMAGE_CLASSIC || style == STYLE_IMAGE_MODERN) {
        drawPieceWithImage(painter, row, col, piece);
    } else {
        drawPieceWithFont(painter, row, col, piece);
    }
}

void ChessBoard::drawPieceWithFont(QPainter &painter, int row, int col, PieceType piece)
{
    QPoint center = boardToPixel(row, col);
    
    // 绘制棋子圆形背景
    QColor bgColor = styleManager.getPieceBackgroundColor(piece);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(bgColor));
    painter.drawEllipse(center.x() - 20, center.y() - 20, 40, 40);
    
    // 绘制棋子文字
    QColor textColor = styleManager.getPieceTextColor(piece);
    painter.setPen(QPen(textColor, 2));
    QFont font = styleManager.getPieceFont();
    painter.setFont(font);
    
    QString text = styleManager.getPieceText(piece);
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    
    QPoint textPos = center;
    textPos.setX(textPos.x() - textRect.width() / 2);
    textPos.setY(textPos.y() + textRect.height() / 2 - 2);
    
    painter.drawText(textPos, text);
}

void ChessBoard::drawPieceWithImage(QPainter &painter, int row, int col, PieceType piece)
{
    // 暂时注释掉SVG功能，直接使用字体显示
    drawPieceWithFont(painter, row, col, piece);
    
    /*
    QPoint center = boardToPixel(row, col);
    QString imagePath = styleManager.getPieceImagePath(piece);
    
    // 如果图片不存在，回退到字体显示
    if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
        drawPieceWithFont(painter, row, col, piece);
        return;
    }
    
    // 加载并绘制SVG图片
    QSvgRenderer renderer(imagePath);
    if (renderer.isValid()) {
        QRect rect(center.x() - 20, center.y() - 20, 40, 40);
        renderer.render(&painter, rect);
    } else {
        // SVG加载失败，回退到字体显示
        drawPieceWithFont(painter, row, col, piece);
    }
    */
}

QPoint ChessBoard::boardToPixel(int row, int col)
{
    int x = BOARD_MARGIN + col * CELL_SIZE;
    int y = BOARD_MARGIN + row * CELL_SIZE;
    return QPoint(x, y);
}

QPoint ChessBoard::pixelToBoard(QPoint pixel)
{
    int col = (pixel.x() - BOARD_MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    int row = (pixel.y() - BOARD_MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    
    if (row < 0 || row >= BOARD_HEIGHT || col < 0 || col >= BOARD_WIDTH) {
        return QPoint(-1, -1);
    }
    
    return QPoint(row, col);
}



QColor ChessBoard::getPieceColor(PieceType piece)
{
    if (piece >= RED_KING && piece <= RED_PAWN) {
        return QColor(255, 200, 200); // 红方浅红色
    } else if (piece >= BLACK_KING && piece <= BLACK_PAWN) {
        return QColor(200, 200, 200); // 黑方浅灰色
    }
    return Qt::white;
}

void ChessBoard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint boardPos = pixelToBoard(event->pos());
        
        if (boardPos.x() >= 0 && boardPos.x() < BOARD_HEIGHT && 
            boardPos.y() >= 0 && boardPos.y() < BOARD_WIDTH) {
            
            if (pieceSelected) {
                // 如果已经选中了棋子，尝试移动
                if (boardPos != selectedPos) {
                    if (makeMove(selectedPos.x(), selectedPos.y(), boardPos.x(), boardPos.y())) {
                        // 移动成功
                    } else {
                        // 移动失败，重新选择
                        PieceType piece = engine.getPiece(boardPos.x(), boardPos.y());
                        if (piece != NONE && 
                            ((engine.isRedTurn() && piece >= RED_KING && piece <= RED_PAWN) ||
                             (!engine.isRedTurn() && piece >= BLACK_KING && piece <= BLACK_PAWN))) {
                            selectedPos = boardPos;
                            highlightValidMoves();
                        } else {
                            pieceSelected = false;
                            selectedPos = QPoint(-1, -1);
                            validMoves.clear();
                        }
                    }
                } else {
                    // 点击同一个位置，取消选择
                    pieceSelected = false;
                    selectedPos = QPoint(-1, -1);
                    validMoves.clear();
                }
            } else {
                // 选择棋子
                PieceType piece = engine.getPiece(boardPos.x(), boardPos.y());
                if (piece != NONE && 
                    ((engine.isRedTurn() && piece >= RED_KING && piece <= RED_PAWN) ||
                     (!engine.isRedTurn() && piece >= BLACK_KING && piece <= BLACK_PAWN))) {
                    selectedPos = boardPos;
                    pieceSelected = true;
                    highlightValidMoves();
                }
            }
            
            update();
        }
    }
}

void ChessBoard::mouseMoveEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    // 可以在这里添加鼠标悬停效果
}

// Chess 主窗口类实现
Chess::Chess(QWidget *parent)
    : QMainWindow(parent), chessBoard(nullptr), styleComboBox(nullptr),
      aiEngine(nullptr), aiEnabled(false), aiThinking(false), aiTimer(nullptr),
      connectionDialog(nullptr)
{
    ui.setupUi(this);
    
    // 创建棋盘并设置到UI中的chessBoard标签位置
    chessBoard = new ChessBoard(this);
    
    // 将棋盘设置到UI设计中的chessBoard标签位置
    QVBoxLayout *boardLayout = new QVBoxLayout(ui.chessBoard);
    boardLayout->setContentsMargins(0, 0, 0, 0);
    boardLayout->addWidget(chessBoard);
    ui.chessBoard->setLayout(boardLayout);
    
    // 连接棋盘信号
    connect(chessBoard, &ChessBoard::moveExecuted, this, &Chess::onMoveExecuted);
    connect(chessBoard, &ChessBoard::gameStatusChanged, this, &Chess::onGameStatusChanged);
    connect(chessBoard, &ChessBoard::boardChanged, this, &Chess::onBoardChanged);
    
    // 设置字体以支持中文显示
    QFont font("Microsoft YaHei", 10);
    this->setFont(font);
    
    // 设置样式选择菜单
    setupStyleMenu();
    
    // 初始化新增的UI控件
    initializeNewControls();
    
    // 设置工具栏
    setupToolBar();
    setupConnectionToolBar();
    
    // 设置AI引擎
    setupAIEngine();
    
    // 显示状态栏消息
    statusBar()->showMessage("欢迎使用鲨鱼象棋 V1.8.0", 3000);
    
    // 初始化游戏信息显示
    updateGameInfo();
    
    // 连接菜单信号槽
    connect(ui.actionConnection, &QAction::triggered, this, &Chess::onConnection);
    
    // 连接新增的菜单项信号槽
    // File菜单
    connect(ui.actionImportPGN, &QAction::triggered, this, &Chess::onImportPGN);
    connect(ui.actionExportPGN, &QAction::triggered, this, &Chess::onExportPGN);
    
    // Edit菜单
    connect(ui.actionCopyPosition, &QAction::triggered, this, &Chess::onCopyPosition);
    connect(ui.actionPastePosition, &QAction::triggered, this, &Chess::onPastePosition);
    connect(ui.actionSetupPosition, &QAction::triggered, this, &Chess::onSetupPosition);
    
    // View菜单
    connect(ui.actionShowCoordinates, &QAction::toggled, this, &Chess::onShowCoordinates);
    connect(ui.actionShowMoveHistory, &QAction::toggled, this, &Chess::onShowMoveHistory);
    connect(ui.actionShowEvaluation, &QAction::toggled, this, &Chess::onShowEvaluation);
    connect(ui.actionFullScreen, &QAction::triggered, this, &Chess::onFullScreen);
    
    // Game菜单
    connect(ui.actionEngineMatch, &QAction::triggered, this, &Chess::onEngineMatch);
    connect(ui.actionAnalyzeGame, &QAction::triggered, this, &Chess::onAnalyzeGame);
    
    // Engine菜单
    connect(ui.actionLoadEngine, &QAction::triggered, this, &Chess::onLoadEngine);
    connect(ui.actionStartEngine, &QAction::triggered, this, &Chess::onStartEngine);
    connect(ui.actionStopEngine, &QAction::triggered, this, &Chess::onStopEngine);
    connect(ui.actionEngineSettings, &QAction::triggered, this, &Chess::onEngineSettings);
    connect(ui.actionAnalyzePosition, &QAction::triggered, this, &Chess::onAnalyzePosition);
    
    // Database菜单
    connect(ui.actionOpenDatabase, &QAction::triggered, this, &Chess::onOpenDatabase);
    connect(ui.actionCreateDatabase, &QAction::triggered, this, &Chess::onCreateDatabase);
    connect(ui.actionSearchPosition, &QAction::triggered, this, &Chess::onSearchPosition);
    // actionOpeningBook 在UI文件中不存在，暂时注释掉
    // connect(ui.actionOpeningBook, &QAction::triggered, this, &Chess::onOpeningBook);
    
    // Tools菜单
    connect(ui.actionOptions, &QAction::triggered, this, &Chess::onOptions);
    connect(ui.actionStatistics, &QAction::triggered, this, &Chess::onStatistics);
    
    // Help菜单
    connect(ui.actionHelp, &QAction::triggered, this, &Chess::onHelp);
}

Chess::~Chess()
{
    if (aiEngine) {
        delete aiEngine;
    }
    if (aiTimer) {
        delete aiTimer;
    }
    if (connectionDialog) {
        delete connectionDialog;
    }
}

void Chess::setupStyleMenu()
{
    // 创建样式选择下拉框
    styleComboBox = new QComboBox(this);
    styleComboBox->addItem("传统字体", STYLE_FONT_TRADITIONAL);
    styleComboBox->addItem("现代字体", STYLE_FONT_MODERN);
    styleComboBox->addItem("经典图片", STYLE_IMAGE_CLASSIC);
    styleComboBox->addItem("现代图片", STYLE_IMAGE_MODERN);
    
    // 连接信号槽
    connect(styleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Chess::onStyleChanged);
    
    // 添加到工具栏
    QLabel* styleLabel = new QLabel("棋子样式:", this);
    ui.mainToolBar->addWidget(styleLabel);
    ui.mainToolBar->addWidget(styleComboBox);
    
    // 设置默认样式
    styleComboBox->setCurrentIndex(0);
}

void Chess::onStyleChanged()
{
    if (!styleComboBox || !chessBoard) return;
    
    int index = styleComboBox->currentIndex();
    PieceStyle style = static_cast<PieceStyle>(styleComboBox->itemData(index).toInt());
    chessBoard->setPieceStyle(style);
}

void Chess::updateGameInfo()
{
    // 更新游戏信息显示 - 这些控件已经从UI中移除
    // 可以在这里添加其他游戏信息更新逻辑
    
    // 更新玩家时间显示
    ui.player1Time->setText("00:00:00");
    ui.player2Time->setText("00:00:00");
}

void Chess::initializeNewControls()
{
    // 创建游戏控制dock窗口
    gameControlDockWidget = new QDockWidget("游戏控制", this);
    gameControlDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    gameControlDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    
    gameControlGroup = new QGroupBox("游戏控制");
    QGridLayout* gameControlLayout = new QGridLayout(gameControlGroup);
    
    newGameBtn = new QPushButton("新游戏", gameControlGroup);
    loadGameBtn = new QPushButton("载入游戏", gameControlGroup);
    saveGameBtn = new QPushButton("保存游戏", gameControlGroup);
    undoBtn = new QPushButton("悔棋", gameControlGroup);
    
    gameControlLayout->addWidget(newGameBtn, 0, 0);
    gameControlLayout->addWidget(loadGameBtn, 0, 1);
    gameControlLayout->addWidget(saveGameBtn, 1, 0);
    gameControlLayout->addWidget(undoBtn, 1, 1);
    
    gameControlDockWidget->setWidget(gameControlGroup);
    addDockWidget(Qt::RightDockWidgetArea, gameControlDockWidget);
    
    // 创建时间控制dock窗口
    timeControlDockWidget = new QDockWidget("时间控制", this);
    timeControlDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    timeControlDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    
    timeControlGroup = new QGroupBox("时间控制");
    QGridLayout* timeControlLayout = new QGridLayout(timeControlGroup);
    
    blackTimeLabel = new QLabel("黑方:　　人类", timeControlGroup);
    blackUsedTimeLabel = new QLabel("已用时:　　000:00", timeControlGroup);
    blackLastMoveLabel = new QLabel("上步时:　　000:00", timeControlGroup);
    blackRemainingLabel = new QLabel("剩余时:　　009:48", timeControlGroup);
    
    redTimeLabel = new QLabel("红方　　　人类", timeControlGroup);
    redUsedTimeLabel = new QLabel("已用时:　　000:00", timeControlGroup);
    redLastMoveLabel = new QLabel("上步时:　　000:00", timeControlGroup);
    redRemainingLabel = new QLabel("剩余时:　　009:48", timeControlGroup);
    
    timeControlLayout->addWidget(blackTimeLabel, 0, 0);
    timeControlLayout->addWidget(blackUsedTimeLabel, 1, 0);
    timeControlLayout->addWidget(blackLastMoveLabel, 2, 0);
    timeControlLayout->addWidget(blackRemainingLabel, 3, 0);
    timeControlLayout->addWidget(redTimeLabel, 0, 1);
    timeControlLayout->addWidget(redUsedTimeLabel, 1, 1);
    timeControlLayout->addWidget(redLastMoveLabel, 2, 1);
    timeControlLayout->addWidget(redRemainingLabel, 3, 1);
    
    timeControlDockWidget->setWidget(timeControlGroup);
    addDockWidget(Qt::RightDockWidgetArea, timeControlDockWidget);
    tabifyDockWidget(gameControlDockWidget, timeControlDockWidget);
    
    // 创建引擎控制dock窗口
    engineControlDockWidget = new QDockWidget("引擎控制", this);
    engineControlDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    engineControlDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    
    QWidget* engineWidget = new QWidget();
    QVBoxLayout* engineMainLayout = new QVBoxLayout(engineWidget);
    
    // 引擎控制组
    engineGroup = new QGroupBox("引擎控制");
    QGridLayout* engineLayout = new QGridLayout(engineGroup);
    
    addEngineButton = new QPushButton("添加引擎", engineGroup);
    engineManageButton = new QPushButton("引擎管理", engineGroup);
    multiEngineButton = new QPushButton("多引擎策略", engineGroup);
    deleteEngineButton = new QPushButton("删除引擎", engineGroup);
    engineEnabledCheck = new QCheckBox("启用引擎", engineGroup);
    engineComboBox = new QComboBox(engineGroup);
    engineDepthSpinBox = new QSpinBox(engineGroup);
    engineTimeSpinBox = new QSpinBox(engineGroup);
    
    engineDepthSpinBox->setRange(1, 20);
    engineDepthSpinBox->setValue(5);
    engineTimeSpinBox->setRange(1, 10000);
    engineTimeSpinBox->setValue(1000);
    
    engineLayout->addWidget(addEngineButton, 0, 0);
    engineLayout->addWidget(engineManageButton, 0, 1);
    engineLayout->addWidget(multiEngineButton, 1, 0);
    engineLayout->addWidget(engineEnabledCheck, 1, 1);
    engineLayout->addWidget(engineComboBox, 2, 0);
    engineLayout->addWidget(new QLabel("深度:"), 2, 1);
    engineLayout->addWidget(engineDepthSpinBox, 3, 0);
    engineLayout->addWidget(new QLabel("时间:"), 3, 1);
    engineLayout->addWidget(engineTimeSpinBox, 4, 0);
    engineLayout->addWidget(deleteEngineButton, 4, 1);
    
    engineMainLayout->addWidget(engineGroup);
    
    // 导航组
    navigationGroup = new QGroupBox("导航");
    QHBoxLayout* navigationLayout = new QHBoxLayout(navigationGroup);
    
    firstMoveButton = new QPushButton("|<", navigationGroup);
    prevMoveButton = new QPushButton("<", navigationGroup);
    nextMoveButton = new QPushButton(">", navigationGroup);
    lastMoveButton = new QPushButton(">|", navigationGroup);
    openBookButton = new QPushButton("开局库", navigationGroup);
    
    navigationLayout->addWidget(firstMoveButton);
    navigationLayout->addWidget(prevMoveButton);
    navigationLayout->addWidget(nextMoveButton);
    navigationLayout->addWidget(lastMoveButton);
    navigationLayout->addWidget(openBookButton);
    
    engineMainLayout->addWidget(navigationGroup);
    engineMainLayout->addStretch();
    
    engineControlDockWidget->setWidget(engineWidget);
    addDockWidget(Qt::RightDockWidgetArea, engineControlDockWidget);
    tabifyDockWidget(timeControlDockWidget, engineControlDockWidget);
    
    // 创建棋谱dock窗口
    moveHistoryDockWidget = new QDockWidget("棋谱", this);
    moveHistoryDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    moveHistoryDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    
    QWidget* moveHistoryWidget = new QWidget();
    QVBoxLayout* moveHistoryMainLayout = new QVBoxLayout(moveHistoryWidget);
    
    moveHistoryGroup = new QGroupBox("棋谱");
    QVBoxLayout* moveHistoryLayout = new QVBoxLayout(moveHistoryGroup);
    
    moveHistoryTable = new QTableWidget(0, 5, moveHistoryGroup);
    QStringList headers;
    headers << "招法" << "类型" << "分数" << "时间" << "注释(可编辑)";
    moveHistoryTable->setHorizontalHeaderLabels(headers);
    
    QHBoxLayout* moveNavLayout = new QHBoxLayout();
    moveFirstButton = new QPushButton("<<", moveHistoryGroup);
    movePrevButton = new QPushButton("<", moveHistoryGroup);
    moveNextButton = new QPushButton(">", moveHistoryGroup);
    moveLastButton = new QPushButton(">>", moveHistoryGroup);
    flipBoardButton = new QPushButton("翻转本局", moveHistoryGroup);
    copyButton = new QPushButton("播放", moveHistoryGroup);
    
    moveNavLayout->addWidget(moveFirstButton);
    moveNavLayout->addWidget(movePrevButton);
    moveNavLayout->addWidget(moveNextButton);
    moveNavLayout->addWidget(moveLastButton);
    moveNavLayout->addWidget(flipBoardButton);
    moveNavLayout->addWidget(copyButton);
    
    // 连接走法导航按钮
    connect(moveFirstButton, &QPushButton::clicked, this, &Chess::onMoveFirst);
    connect(movePrevButton, &QPushButton::clicked, this, &Chess::onMovePrevious);
    connect(moveNextButton, &QPushButton::clicked, this, &Chess::onMoveNext);
    connect(moveLastButton, &QPushButton::clicked, this, &Chess::onMoveLast);
    connect(flipBoardButton, &QPushButton::clicked, this, &Chess::onFlipBoard);
    connect(copyButton, &QPushButton::clicked, this, &Chess::onCopyMoves);
    
    moveHistoryLayout->addWidget(moveHistoryTable);
    moveHistoryLayout->addLayout(moveNavLayout);
    
    moveHistoryMainLayout->addWidget(moveHistoryGroup);
    moveHistoryMainLayout->addStretch();
    
    moveHistoryDockWidget->setWidget(moveHistoryWidget);
    addDockWidget(Qt::RightDockWidgetArea, moveHistoryDockWidget);
    tabifyDockWidget(engineControlDockWidget, moveHistoryDockWidget);
    
    // 初始化引擎控件
    engineEnabledCheck->setChecked(false);
    engineComboBox->setCurrentText("CCStockFish-bm");
    engineDepthSpinBox->setValue(4);
    engineTimeSpinBox->setValue(256);
    
    // 棋谱表格已经在上面初始化过了
    
    // 设置表格列宽
    moveHistoryTable->setColumnWidth(0, 60);
    moveHistoryTable->setColumnWidth(1, 40);
    moveHistoryTable->setColumnWidth(2, 50);
    moveHistoryTable->setColumnWidth(3, 50);
    moveHistoryTable->setColumnWidth(4, 80);
    
    // 连接信号槽
    connect(addEngineButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "添加引擎功能待实现");
    });
    
    connect(engineManageButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "引擎管理功能待实现");
    });
    
    connect(multiEngineButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "多引擎策略功能待实现");
    });
    
    connect(deleteEngineButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "删除引擎功能待实现");
    });
    
    connect(firstMoveButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "时间功能待实现");
    });
    
    connect(prevMoveButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "引擎功能待实现");
    });
    
    connect(nextMoveButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "导航功能待实现");
    });
    
    connect(lastMoveButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "趋势图功能待实现");
    });
    
    connect(openBookButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "开局库功能待实现");
    });
    
    connect(moveFirstButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "跳转到第一步功能待实现");
    });
    
    connect(movePrevButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "上一步功能待实现");
    });
    
    connect(moveNextButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "下一步功能待实现");
    });
    
    connect(moveLastButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "跳转到最后一步功能待实现");
    });
    
    connect(flipBoardButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "翻转棋盘功能待实现");
    });
    
    connect(copyButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "播放功能待实现");
    });
}

/*
void Chess::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建棋盘
    chessBoard = new ChessBoard(this);
    
    // 创建控制面板
    controlPanel = new QWidget(this);
    controlPanel->setFixedWidth(300);
    controlPanel->setStyleSheet("QWidget { background-color: #f0f0f0; }");
}
*/

/*
void Chess::setupMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *newGameAction = new QAction("新游戏(&N)", this);
    newGameAction->setShortcut(QKeySequence::New);
    connect(newGameAction, &QAction::triggered, this, &Chess::onNewGame);
    fileMenu->addAction(newGameAction);
    
    fileMenu->addSeparator();
    
    QAction *loadGameAction = new QAction("加载棋谱(&L)", this);
    loadGameAction->setShortcut(QKeySequence::Open);
    connect(loadGameAction, &QAction::triggered, this, &Chess::onLoadGame);
    fileMenu->addAction(loadGameAction);
    
    QAction *saveGameAction = new QAction("保存棋谱(&S)", this);
    saveGameAction->setShortcut(QKeySequence::Save);
    connect(saveGameAction, &QAction::triggered, this, &Chess::onSaveGame);
    fileMenu->addAction(saveGameAction);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction *aboutAction = new QAction("关于(&A)", this);
    connect(aboutAction, &QAction::triggered, this, &Chess::onAbout);
    helpMenu->addAction(aboutAction);
}
*/

/*
void Chess::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar("主工具栏");
    
    newGameBtn = new QPushButton("新游戏", this);
    connect(newGameBtn, &QPushButton::clicked, this, &Chess::onNewGame);
    mainToolBar->addWidget(newGameBtn);
    
    mainToolBar->addSeparator();
    
    undoBtn = new QPushButton("悔棋", this);
    mainToolBar->addWidget(undoBtn);
    
    redoBtn = new QPushButton("重做", this);
    mainToolBar->addWidget(redoBtn);
    
    mainToolBar->addSeparator();
    
    analyzeBtn = new QPushButton("分析", this);
    mainToolBar->addWidget(analyzeBtn);
    
    hintBtn = new QPushButton("提示", this);
    mainToolBar->addWidget(hintBtn);
}
*/

/*
void Chess::setupStatusBar()
{
    statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(statusLabel);
    
    thinkingProgress = new QProgressBar(this);
    thinkingProgress->setVisible(false);
    statusBar()->addPermanentWidget(thinkingProgress);
}
*/

/*
void Chess::setupMainLayout()
{
    QWidget *centralWidget = this->centralWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // 左侧棋盘
    mainLayout->addWidget(chessBoard);
    
    // 右侧控制面板
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    
    // 游戏控制组
    gameControlGroup = new QGroupBox("游戏控制", this);
    QVBoxLayout *gameLayout = new QVBoxLayout(gameControlGroup);
    
    loadGameBtn = new QPushButton("加载棋谱", this);
    saveGameBtn = new QPushButton("保存棋谱", this);
    connect(loadGameBtn, &QPushButton::clicked, this, &Chess::onLoadGame);
    connect(saveGameBtn, &QPushButton::clicked, this, &Chess::onSaveGame);
    
    gameLayout->addWidget(loadGameBtn);
    gameLayout->addWidget(saveGameBtn);
    
    // 引擎设置组
    engineGroup = new QGroupBox("引擎设置", this);
    QVBoxLayout *engineLayout = new QVBoxLayout(engineGroup);
    
    autoPlayCheck = new QCheckBox("自动下棋", this);
    showHintsCheck = new QCheckBox("显示提示", this);
    
    QHBoxLayout *difficultyLayout = new QHBoxLayout();
    difficultyLayout->addWidget(new QLabel("难度:", this));
    difficultyCombo = new QComboBox(this);
    difficultyCombo->addItems({"简单", "中等", "困难", "专家"});
    difficultyCombo->setCurrentIndex(1);
    difficultyLayout->addWidget(difficultyCombo);
    
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(new QLabel("思考时间(秒):", this));
    thinkTimeSpinBox = new QSpinBox(this);
    thinkTimeSpinBox->setRange(1, 300);
    thinkTimeSpinBox->setValue(10);
    timeLayout->addWidget(thinkTimeSpinBox);
    
    engineLayout->addWidget(autoPlayCheck);
    engineLayout->addWidget(showHintsCheck);
    engineLayout->addLayout(difficultyLayout);
    engineLayout->addLayout(timeLayout);
    
    // 分析组
    analysisGroup = new QGroupBox("局面分析", this);
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisGroup);
    
    moveHistory = new QTextEdit(this);
    moveHistory->setMaximumHeight(150);
    moveHistory->setPlaceholderText("棋谱记录...");
    
    engineOutput = new QTextEdit(this);
    engineOutput->setMaximumHeight(150);
    engineOutput->setPlaceholderText("引擎输出...");
    
    analysisLayout->addWidget(new QLabel("走法历史:", this));
    analysisLayout->addWidget(moveHistory);
    analysisLayout->addWidget(new QLabel("引擎分析:", this));
    analysisLayout->addWidget(engineOutput);
    
    // 添加到控制面板
    controlLayout->addWidget(gameControlGroup);
    controlLayout->addWidget(engineGroup);
    controlLayout->addWidget(analysisGroup);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel);
    
    // 设置布局比例
    mainLayout->setStretch(0, 2); // 棋盘占2/3
    mainLayout->setStretch(1, 1); // 控制面板占1/3
}
*/

void Chess::onNewGame()
{
    chessBoard->initializeBoard();
    moveHistoryTable->clearContents();
    moveHistoryTable->setRowCount(0);
    statusBar()->showMessage("新游戏开始", 3000);
    updateGameInfo();
}

void Chess::onMoveExecuted(const Move& move)
{
    // 添加走法到历史表格
    int row = moveHistoryTable->rowCount();
    moveHistoryTable->insertRow(row);
    
    QString moveStr = QString::fromStdString(move.toString());
    QString moveType = "普通";
    if (move.capturedPiece != NONE) {
        moveType = "吃子";
    }
    
    moveHistoryTable->setItem(row, 0, new QTableWidgetItem(moveStr));
    moveHistoryTable->setItem(row, 1, new QTableWidgetItem(moveType));
    moveHistoryTable->setItem(row, 2, new QTableWidgetItem(""));
    moveHistoryTable->setItem(row, 3, new QTableWidgetItem(""));
    moveHistoryTable->setItem(row, 4, new QTableWidgetItem(""));
    
    // 滚动到最新走法
    moveHistoryTable->scrollToBottom();
    
    updateGameInfo();
}

void Chess::onGameStatusChanged(const QString& status)
{
    statusBar()->showMessage(status);
}

void Chess::onBoardChanged()
{
    updateGameInfo();
}

void Chess::onMoveFirst()
{
    chessBoard->goToMove(-1);
}

void Chess::onMovePrevious()
{
    const MoveHistory& history = chessBoard->getMoveHistory();
    if (!history.isAtFirst()) {
        chessBoard->goToMove(history.getCurrentIndex() - 1);
    }
}

void Chess::onMoveNext()
{
    const MoveHistory& history = chessBoard->getMoveHistory();
    if (!history.isAtLast()) {
        chessBoard->goToMove(history.getCurrentIndex() + 1);
    }
}

void Chess::onMoveLast()
{
    const MoveHistory& history = chessBoard->getMoveHistory();
    chessBoard->goToMove(history.getMoveCount() - 1);
}

void Chess::onFlipBoard()
{
    // 翻转棋盘功能待实现
    statusBar()->showMessage("翻转棋盘功能待实现", 3000);
}

void Chess::onCopyMoves()
{
    const MoveHistory& history = chessBoard->getMoveHistory();
    QString moveList = QString::fromStdString(history.toMoveList());
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(moveList);
    statusBar()->showMessage("走法已复制到剪贴板", 3000);
}

void Chess::onLoadGame()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "加载棋谱", "", "棋谱文件 (*.pgn *.txt);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        statusLabel->setText("棋谱加载功能待实现");
    }
}

void Chess::onSaveGame()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "保存棋谱", "", "棋谱文件 (*.pgn *.txt);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        statusLabel->setText("棋谱保存功能待实现");
    }
}

void Chess::onSettings()
{
    statusLabel->setText("设置功能待实现");
}

void Chess::onAbout()
{
    QMessageBox::about(this, "关于", 
        "象棋辅助程序 v1.0\n\n"
        "基于Qt开发的中国象棋辅助工具\n"
        "参考VinXiangQi项目设计\n\n"
        "功能特点:\n"
        "• 标准象棋棋盘显示\n"
        "• 棋谱记录与分析\n"
        "• 引擎对战支持\n"
        "• 局面评估与提示");
}

// AI引擎相关方法实现
void Chess::setupAIEngine()
{
    // 创建AI引擎
    aiEngine = new AIEngine();
    aiEngine->setDifficulty(AI_MEDIUM);
    aiEngine->setDebugMode(true);
    
    // 创建AI定时器
    aiTimer = new QTimer(this);
    aiTimer->setSingleShot(true);
    connect(aiTimer, &QTimer::timeout, this, &Chess::makeAIMove);
    
    // 在引擎控制组中添加AI相关控件
    if (engineGroup) {
        QVBoxLayout* engineLayout = qobject_cast<QVBoxLayout*>(engineGroup->layout());
        if (engineLayout) {
            // AI难度选择
            QHBoxLayout* difficultyLayout = new QHBoxLayout();
            difficultyLayout->addWidget(new QLabel("AI难度:"));
            
            aiDifficultyCombo = new QComboBox();
            aiDifficultyCombo->addItem("简单", AI_EASY);
            aiDifficultyCombo->addItem("中等", AI_MEDIUM);
            aiDifficultyCombo->addItem("困难", AI_HARD);
            aiDifficultyCombo->addItem("专家", AI_EXPERT);
            aiDifficultyCombo->setCurrentIndex(1); // 默认中等
            
            connect(aiDifficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &Chess::onAIDifficultyChanged);
            
            difficultyLayout->addWidget(aiDifficultyCombo);
            engineLayout->addLayout(difficultyLayout);
            
            // AI控制按钮
            QHBoxLayout* aiButtonLayout = new QHBoxLayout();
            
            aiMoveButton = new QPushButton("AI走棋");
            connect(aiMoveButton, &QPushButton::clicked, this, &Chess::onAIMove);
            aiButtonLayout->addWidget(aiMoveButton);
            
            stopAIButton = new QPushButton("停止AI");
            stopAIButton->setEnabled(false);
            connect(stopAIButton, &QPushButton::clicked, this, &Chess::onStopAI);
            aiButtonLayout->addWidget(stopAIButton);
            
            engineLayout->addLayout(aiButtonLayout);
            
            // AI状态标签
            aiStatusLabel = new QLabel("AI就绪");
            aiStatusLabel->setStyleSheet("color: green;");
            engineLayout->addWidget(aiStatusLabel);
        }
    }
    
    // 连接引擎启用复选框
    if (engineEnabledCheck) {
        connect(engineEnabledCheck, &QCheckBox::toggled, this, &Chess::onAIEnabled);
    }
    
    // 连接深度和时间控制
    if (engineDepthSpinBox) {
        connect(engineDepthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &Chess::onAIDepthChanged);
    }
    
    if (engineTimeSpinBox) {
        connect(engineTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &Chess::onAITimeChanged);
    }
}

void Chess::updateAIControls()
{
    if (!aiEngine) return;
    
    bool gameActive = !chessBoard->isGameOver();
    bool canAIMove = gameActive && aiEnabled && !aiThinking;
    
    if (aiMoveButton) {
        aiMoveButton->setEnabled(canAIMove);
    }
    
    if (stopAIButton) {
        stopAIButton->setEnabled(aiThinking);
    }
    
    if (aiStatusLabel) {
        if (aiThinking) {
            aiStatusLabel->setText("AI思考中...");
            aiStatusLabel->setStyleSheet("color: orange;");
        } else if (aiEnabled) {
            aiStatusLabel->setText("AI就绪");
            aiStatusLabel->setStyleSheet("color: green;");
        } else {
            aiStatusLabel->setText("AI已禁用");
            aiStatusLabel->setStyleSheet("color: gray;");
        }
    }
    
    if (thinkingProgress) {
        thinkingProgress->setVisible(aiThinking);
        if (aiThinking) {
            thinkingProgress->setRange(0, 0); // 无限进度条
        }
    }
}

void Chess::makeAIMove()
{
    if (!aiEngine || !chessBoard || aiThinking) return;
    
    aiThinking = true;
    updateAIControls();
    
    // 获取当前棋盘状态
    ChessEngine& engine = chessBoard->getEngine();
    bool isRedTurn = engine.isRedTurn();
    
    // AI思考
    Move aiMove = aiEngine->getBestMove(engine, isRedTurn);
    
    aiThinking = false;
    updateAIControls();
    
    if (aiMove.isValid()) {
        // 执行AI走法
        bool success = chessBoard->makeMove(aiMove.fromRow, aiMove.fromCol, 
                                          aiMove.toRow, aiMove.toCol);
        
        if (success) {
            QString moveStr = QString("AI走法: %1").arg(QString::fromStdString(aiMove.toString()));
            statusBar()->showMessage(moveStr, 3000);
            
            // 显示AI分析信息
            if (aiStatusLabel) {
                QString info = QString("评分: %1, %2")
                    .arg(aiEngine->evaluatePosition(engine, isRedTurn))
                    .arg(QString::fromStdString(aiEngine->getSearchInfo()));
                aiStatusLabel->setToolTip(info);
            }
        } else {
            statusBar()->showMessage("AI走法执行失败", 3000);
        }
    } else {
        statusBar()->showMessage("AI无法找到合法走法", 3000);
    }
}

void Chess::onAIEnabled(bool enabled)
{
    aiEnabled = enabled;
    updateAIControls();
    
    if (enabled) {
        statusBar()->showMessage("AI引擎已启用", 2000);
    } else {
        statusBar()->showMessage("AI引擎已禁用", 2000);
        if (aiThinking) {
            onStopAI();
        }
    }
}

void Chess::onAIDifficultyChanged()
{
    if (!aiEngine || !aiDifficultyCombo) return;
    
    AIDifficulty difficulty = static_cast<AIDifficulty>(aiDifficultyCombo->currentData().toInt());
    aiEngine->setDifficulty(difficulty);
    
    QString difficultyText = aiDifficultyCombo->currentText();
    statusBar()->showMessage(QString("AI难度设置为: %1").arg(difficultyText), 2000);
}

void Chess::onAIDepthChanged(int depth)
{
    if (!aiEngine) return;
    
    aiEngine->setMaxDepth(depth);
    statusBar()->showMessage(QString("AI搜索深度设置为: %1").arg(depth), 2000);
}

void Chess::onAITimeChanged(int timeMs)
{
    if (!aiEngine) return;
    
    double timeSeconds = timeMs / 1000.0;
    aiEngine->setTimeLimit(timeSeconds);
    statusBar()->showMessage(QString("AI思考时间设置为: %1秒").arg(timeSeconds), 2000);
}

void Chess::onAIMove()
{
    if (!aiEngine || !chessBoard || aiThinking || chessBoard->isGameOver()) {
        return;
    }
    
    // 延迟500ms开始AI思考，让UI有时间更新
    aiTimer->start(500);
}

void Chess::onStopAI()
{
    if (aiEngine) {
        aiEngine->stopThinking();
    }
    
    if (aiTimer && aiTimer->isActive()) {
        aiTimer->stop();
    }
    
    aiThinking = false;
    updateAIControls();
    
    statusBar()->showMessage("AI思考已停止", 2000);
}

/**
 * @brief 打开平台连线对话框
 * 创建并显示连线配置对话框，支持多平台象棋连线功能
 */
void Chess::onConnection()
{
    // 如果对话框不存在，则创建新的对话框
    if (!connectionDialog) {
        connectionDialog = new ConnectionDialog(this);
    }
    
    // 显示连线对话框
    connectionDialog->show();
    connectionDialog->raise();
    connectionDialog->activateWindow();
    
    // 更新状态栏信息
    statusBar()->showMessage("打开平台连线配置", 2000);
}

// ==================== 新增的槽函数实现 ====================

/**
 * @brief 导入PGN文件
 * 打开文件对话框选择PGN文件并导入棋谱
 */
void Chess::onImportPGN()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "导入PGN文件", "", "PGN文件 (*.pgn);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: 实现PGN文件导入功能
        statusBar()->showMessage("PGN文件导入功能待实现: " + fileName, 3000);
    }
}

/**
 * @brief 导出PGN文件
 * 将当前棋谱导出为PGN格式文件
 */
void Chess::onExportPGN()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "导出PGN文件", "", "PGN文件 (*.pgn);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: 实现PGN文件导出功能
        statusBar()->showMessage("PGN文件导出功能待实现: " + fileName, 3000);
    }
}

/**
 * @brief 复制当前局面
 * 将当前棋盘局面复制到剪贴板
 */
void Chess::onCopyPosition()
{
    // TODO: 实现局面复制功能
    statusBar()->showMessage("复制局面功能待实现", 2000);
}

/**
 * @brief 粘贴局面
 * 从剪贴板粘贴局面到棋盘
 */
void Chess::onPastePosition()
{
    // TODO: 实现局面粘贴功能
    statusBar()->showMessage("粘贴局面功能待实现", 2000);
}

/**
 * @brief 设置局面
 * 打开局面设置对话框，允许用户自定义棋盘局面
 */
void Chess::onSetupPosition()
{
    // TODO: 实现局面设置功能
    statusBar()->showMessage("设置局面功能待实现", 2000);
}

/**
 * @brief 显示/隐藏坐标
 * 切换棋盘坐标显示状态
 */
void Chess::onShowCoordinates(bool checked)
{
    // TODO: 实现坐标显示切换功能
    statusBar()->showMessage(checked ? "显示坐标" : "隐藏坐标", 2000);
}

/**
 * @brief 显示/隐藏着法历史
 * 切换着法历史面板显示状态
 */
void Chess::onShowMoveHistory(bool checked)
{
    // TODO: 实现着法历史显示切换功能
    statusBar()->showMessage(checked ? "显示着法历史" : "隐藏着法历史", 2000);
}

/**
 * @brief 显示/隐藏局面评估
 * 切换局面评估信息显示状态
 */
void Chess::onShowEvaluation(bool checked)
{
    // TODO: 实现局面评估显示切换功能
    statusBar()->showMessage(checked ? "显示局面评估" : "隐藏局面评估", 2000);
}

/**
 * @brief 全屏模式切换
 * 在全屏和窗口模式之间切换
 */
void Chess::onFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        statusBar()->showMessage("退出全屏模式", 2000);
    } else {
        showFullScreen();
        statusBar()->showMessage("进入全屏模式", 2000);
    }
}

/**
 * @brief 引擎对弈
 * 启动引擎对弈模式
 */
void Chess::onEngineMatch()
{
    // TODO: 实现引擎对弈功能
    statusBar()->showMessage("引擎对弈功能待实现", 2000);
}

/**
 * @brief 分析棋谱
 * 使用引擎分析当前棋谱
 */
void Chess::onAnalyzeGame()
{
    // TODO: 实现棋谱分析功能
    statusBar()->showMessage("棋谱分析功能待实现", 2000);
}

/**
 * @brief 加载引擎
 * 打开文件对话框选择并加载象棋引擎
 */
void Chess::onLoadEngine()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "选择象棋引擎", "", "可执行文件 (*.exe);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: 实现引擎加载功能
        statusBar()->showMessage("引擎加载功能待实现: " + fileName, 3000);
    }
}

/**
 * @brief 启动引擎
 * 启动当前加载的象棋引擎
 */
void Chess::onStartEngine()
{
    // TODO: 实现引擎启动功能
    statusBar()->showMessage("启动引擎功能待实现", 2000);
}

/**
 * @brief 停止引擎
 * 停止当前运行的象棋引擎
 */
void Chess::onStopEngine()
{
    // TODO: 实现引擎停止功能
    statusBar()->showMessage("停止引擎功能待实现", 2000);
}

/**
 * @brief 引擎设置
 * 打开引擎配置对话框
 */
void Chess::onEngineSettings()
{
    // TODO: 实现引擎设置功能
    statusBar()->showMessage("引擎设置功能待实现", 2000);
}

/**
 * @brief 分析局面
 * 使用引擎分析当前局面
 */
void Chess::onAnalyzePosition()
{
    // TODO: 实现局面分析功能
    statusBar()->showMessage("局面分析功能待实现", 2000);
}

/**
 * @brief 打开数据库
 * 打开棋谱数据库文件
 */
void Chess::onOpenDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "打开棋谱数据库", "", "数据库文件 (*.db *.sqlite);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: 实现数据库打开功能
        statusBar()->showMessage("数据库打开功能待实现: " + fileName, 3000);
    }
}

/**
 * @brief 创建数据库
 * 创建新的棋谱数据库
 */
void Chess::onCreateDatabase()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "创建棋谱数据库", "", "数据库文件 (*.db *.sqlite);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: 实现数据库创建功能
        statusBar()->showMessage("数据库创建功能待实现: " + fileName, 3000);
    }
}

/**
 * @brief 搜索局面
 * 在棋谱数据库中搜索特定局面或棋谱
 */
void Chess::onSearchPosition()
{
    // TODO: 实现局面搜索功能
    statusBar()->showMessage("局面搜索功能待实现", 2000);
}

/**
 * @brief 程序选项
 * 打开程序设置对话框
 */
void Chess::onOptions()
{
    // TODO: 实现程序选项功能
    statusBar()->showMessage("程序选项功能待实现", 2000);
}

/**
 * @brief 统计信息
 * 显示游戏统计信息对话框
 */
void Chess::onStatistics()
{
    // TODO: 实现统计信息功能
    statusBar()->showMessage("统计信息功能待实现", 2000);
}

/**
 * @brief 帮助文档
 * 打开帮助文档或在线帮助
 */
void Chess::onHelp()
{
    QMessageBox::information(this, "帮助", 
        "鲨鱼象棋 V1.8.0\n\n"
        "这是一个功能丰富的中国象棋程序，支持：\n"
        "• 人机对弈和人人对弈\n"
        "• 棋谱导入导出\n"
        "• 引擎分析\n"
        "• 数据库管理\n"
        "• 多种界面风格\n\n"
        "更多功能正在开发中...");
}

/**
 * @brief 设置主工具栏
 * 初始化主工具栏的基本结构和样式，添加棋盘保存相关按钮
 */
void Chess::setupToolBar()
{
    // 获取主工具栏（已在UI文件中定义）
    mainToolBar = ui.mainToolBar;
    if (!mainToolBar) {
        mainToolBar = addToolBar("棋盘操作");
    }
    
    // 设置工具栏样式
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->setIconSize(QSize(16, 16));
    mainToolBar->setMovable(true);
    mainToolBar->setFloatable(true);
    
    // 设置工具栏位置为第一行
    addToolBarBreak(Qt::TopToolBarArea);
    
    // 添加新建游戏按钮
    QAction* newGameAction = new QAction(QIcon(":/images/icons/new_game.svg"), "新建", this);
    newGameAction->setToolTip("开始新游戏");
    mainToolBar->addAction(newGameAction);
    connect(newGameAction, &QAction::triggered, this, &Chess::onNewGame);
    
    // 添加打开文件按钮
    QAction* openAction = new QAction(QIcon(":/images/icons/open.svg"), "打开", this);
    openAction->setToolTip("打开棋谱文件");
    mainToolBar->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &Chess::onLoadGame);
    
    // 添加保存文件按钮
    QAction* saveAction = new QAction(QIcon(":/images/icons/save.svg"), "保存", this);
    saveAction->setToolTip("保存当前棋谱");
    mainToolBar->addAction(saveAction);
    connect(saveAction, &QAction::triggered, this, &Chess::onSaveGame);
    
    mainToolBar->addSeparator();
    
    // 添加撤销按钮
    QAction* undoAction = new QAction(QIcon(":/images/icons/undo.svg"), "撤销", this);
    undoAction->setToolTip("撤销上一步");
    mainToolBar->addAction(undoAction);
    connect(undoAction, &QAction::triggered, this, &Chess::onMovePrevious);
    
    // 添加重做按钮
    QAction* redoAction = new QAction(QIcon(":/images/icons/redo.svg"), "重做", this);
    redoAction->setToolTip("重做下一步");
    mainToolBar->addAction(redoAction);
    connect(redoAction, &QAction::triggered, this, &Chess::onMoveNext);
    
    mainToolBar->addSeparator();
    
    // 添加翻转棋盘按钮
    QAction* flipAction = new QAction(QIcon(":/images/icons/flip_board.svg"), "翻转", this);
    flipAction->setToolTip("翻转棋盘视角");
    mainToolBar->addAction(flipAction);
    connect(flipAction, &QAction::triggered, this, &Chess::onFlipBoard);
    
    // 添加分析位置按钮
    QAction* analyzeAction = new QAction(QIcon(":/images/icons/analyze_position.svg"), "分析", this);
    analyzeAction->setToolTip("分析当前局面");
    mainToolBar->addAction(analyzeAction);
    connect(analyzeAction, &QAction::triggered, this, &Chess::onAnalyzePosition);
}

/**
 * @brief 设置连线管理工具栏
 * 创建连线管理专用工具栏，包含连接、断开、方案选择等功能
 */
void Chess::setupConnectionToolBar()
{
    // 创建连线管理工具栏
    connectionToolBar = addToolBar("连线管理");
    connectionToolBar->setObjectName("connectionToolBar");
    connectionToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connectionToolBar->setIconSize(QSize(16, 16));
    
    // 将连线管理工具栏放在第二行
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, connectionToolBar);
    
    // 初始化连接状态
    isConnected = false;
    currentScheme = "默认方案";
    
    // 创建连接动作
    connectAction = new QAction(QIcon(":/images/icons/connection.svg"), "连接", this);
    connectAction->setToolTip("连接到象棋平台");
    connectAction->setEnabled(true);
    connectionToolBar->addAction(connectAction);
    
    // 创建断开连接动作
    disconnectAction = new QAction(QIcon(":/images/icons/disconnect.svg"), "断开", this);
    disconnectAction->setToolTip("断开连接");
    disconnectAction->setEnabled(false);
    connectionToolBar->addAction(disconnectAction);
    
    connectionToolBar->addSeparator();
    
    // 创建连接方案选择下拉框
    connectionSchemeCombo = new QComboBox(this);
    connectionSchemeCombo->addItem("默认方案");
    connectionSchemeCombo->addItem("天天象棋");
    connectionSchemeCombo->addItem("QQ象棋");
    connectionSchemeCombo->addItem("象棋巫师");
    connectionSchemeCombo->setToolTip("选择连接方案");
    connectionSchemeCombo->setMinimumWidth(120);
    connectionToolBar->addWidget(connectionSchemeCombo);
    
    connectionToolBar->addSeparator();
    
    // 创建连接状态标签
    connectionStatusLabel = new QLabel("未连接", this);
    connectionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 2px 8px; }");
    connectionStatusLabel->setToolTip("当前连接状态");
    connectionToolBar->addWidget(connectionStatusLabel);
    
    // 创建连接状态按钮
    connectionStatusBtn = new QPushButton(this);
    connectionStatusBtn->setIcon(QIcon(":/images/icons/status.svg"));
    connectionStatusBtn->setText("状态");
    connectionStatusBtn->setMaximumWidth(80);
    connectionStatusBtn->setToolTip("查看详细连接状态");
    connectionToolBar->addWidget(connectionStatusBtn);
    
    // 连接信号槽
    connect(connectAction, &QAction::triggered, this, &Chess::onConnect);
    connect(disconnectAction, &QAction::triggered, this, &Chess::onDisconnect);
    connect(connectionSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Chess::onConnectionSchemeChanged);
    connect(connectionStatusBtn, &QPushButton::clicked, this, &Chess::onConnectionStatusClicked);
    
    statusBar()->showMessage("显示帮助信息", 2000);
}

/**
 * @brief 处理连接按钮点击事件
 * 执行连接到象棋平台的操作
 */
void Chess::onConnect()
{
    if (isConnected) {
        statusBar()->showMessage("已经连接，请先断开连接", 2000);
        return;
    }
    
    statusBar()->showMessage("正在连接到 " + currentScheme + "...", 0);
    
    // 禁用连接按钮，启用断开按钮
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    connectionSchemeCombo->setEnabled(false);
    
    // 模拟连接过程（实际项目中这里会调用真实的连接逻辑）
    QTimer::singleShot(2000, this, [this]() {
        isConnected = true;
        connectionStatusLabel->setText("已连接");
        connectionStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; padding: 2px 8px; }");
        statusBar()->showMessage("成功连接到 " + currentScheme, 3000);
    });
}

/**
 * @brief 处理断开连接按钮点击事件
 * 执行断开连接的操作
 */
void Chess::onDisconnect()
{
    if (!isConnected) {
        statusBar()->showMessage("当前未连接", 2000);
        return;
    }
    
    statusBar()->showMessage("正在断开连接...", 0);
    
    // 模拟断开连接过程
    QTimer::singleShot(1000, this, [this]() {
        isConnected = false;
        connectionStatusLabel->setText("未连接");
        connectionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 2px 8px; }");
        
        // 恢复按钮状态
        connectAction->setEnabled(true);
        disconnectAction->setEnabled(false);
        connectionSchemeCombo->setEnabled(true);
        
        statusBar()->showMessage("已断开连接", 3000);
    });
}

/**
 * @brief 处理连接方案改变事件
 * @param index 选中的方案索引
 */
void Chess::onConnectionSchemeChanged(int index)
{
    if (isConnected) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "切换连接方案",
            "当前已连接，切换方案将断开现有连接。是否继续？",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            // 恢复到之前的选择
            connectionSchemeCombo->blockSignals(true);
            for (int i = 0; i < connectionSchemeCombo->count(); ++i) {
                if (connectionSchemeCombo->itemText(i) == currentScheme) {
                    connectionSchemeCombo->setCurrentIndex(i);
                    break;
                }
            }
            connectionSchemeCombo->blockSignals(false);
            return;
        }
        
        // 先断开连接
        onDisconnect();
    }
    
    currentScheme = connectionSchemeCombo->itemText(index);
    statusBar()->showMessage("已切换到连接方案: " + currentScheme, 2000);
}

/**
 * @brief 处理连接状态按钮点击事件
 * 显示详细的连接状态信息
 */
void Chess::onConnectionStatusClicked()
{
    QString statusInfo;
    if (isConnected) {
        statusInfo = QString("连接状态: 已连接\n")
                   + "连接方案: " + currentScheme + "\n"
                   + "连接时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n"
                   + "状态: 正常";
    } else {
        statusInfo = QString("连接状态: 未连接\n")
                   + "当前方案: " + currentScheme + "\n"
                   + "状态: 待连接";
    }
    
    QMessageBox::information(this, "连接状态详情", statusInfo);
}

/**
 * @brief 更新连接状态显示
 * 根据当前连接状态更新UI显示
 */
void Chess::updateConnectionStatus()
{
    if (isConnected) {
        connectionStatusLabel->setText("已连接");
        connectionStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; padding: 2px 8px; }");
        connectAction->setEnabled(false);
        disconnectAction->setEnabled(true);
        connectionSchemeCombo->setEnabled(false);
    } else {
        connectionStatusLabel->setText("未连接");
        connectionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 2px 8px; }");
        connectAction->setEnabled(true);
        disconnectAction->setEnabled(false);
        connectionSchemeCombo->setEnabled(true);
    }
}

