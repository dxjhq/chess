#include "Chess.h"
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
    // 初始化棋盘为空
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = NONE;
        }
    }
    
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
}

void ChessBoard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置背景色
    painter.fillRect(rect(), QColor(245, 222, 179)); // 木色背景
    
    drawBoard(painter);
    drawPieces(painter);
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
            if (board[i][j] != NONE) {
                drawPiece(painter, i, j, board[i][j]);
            }
        }
    }
    
    // 绘制选中效果
    if (pieceSelected && selectedPos.x() >= 0 && selectedPos.y() >= 0) {
        painter.setPen(QPen(Qt::yellow, 3));
        painter.setBrush(Qt::NoBrush);
        QPoint center = boardToPixel(selectedPos.x(), selectedPos.y());
        painter.drawEllipse(center.x() - 22, center.y() - 22, 44, 44);
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
        if (boardPos.x() >= 0 && boardPos.y() >= 0) {
            if (pieceSelected && selectedPos == boardPos) {
                // 取消选择
                pieceSelected = false;
                selectedPos = QPoint(-1, -1);
            } else if (board[boardPos.x()][boardPos.y()] != NONE) {
                // 选择棋子
                selectedPos = boardPos;
                pieceSelected = true;
            } else if (pieceSelected) {
                // 移动棋子（简单实现，不检查规则）
                board[boardPos.x()][boardPos.y()] = board[selectedPos.x()][selectedPos.y()];
                board[selectedPos.x()][selectedPos.y()] = NONE;
                pieceSelected = false;
                selectedPos = QPoint(-1, -1);
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
    : QMainWindow(parent), chessBoard(nullptr), styleComboBox(nullptr)
{
    ui.setupUi(this);
    
    // 创建棋盘并设置到UI中的chessBoard标签位置
    chessBoard = new ChessBoard(this);
    
    // 将棋盘设置到UI设计中的chessBoard标签位置
    QVBoxLayout *boardLayout = new QVBoxLayout(ui.chessBoard);
    boardLayout->setContentsMargins(0, 0, 0, 0);
    boardLayout->addWidget(chessBoard);
    ui.chessBoard->setLayout(boardLayout);
    
    // 设置字体以支持中文显示
    QFont font("Microsoft YaHei", 10);
    this->setFont(font);
    
    // 设置样式选择菜单
    setupStyleMenu();
    
    // 显示状态栏消息
    statusBar()->showMessage("欢迎使用鲨鱼象棋 V1.8.0", 3000);
    
    // 初始化游戏信息显示
    updateGameInfo();
}

Chess::~Chess()
{
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
    // 更新游戏信息显示
    ui.currentPlayerLabel->setText("当前玩家: 红方");
    ui.gameStatusLabel->setText("游戏状态: 进行中");
    ui.moveCountLabel->setText("回合数: 1");
    
    // 更新玩家时间显示
    ui.player1Time->setText("00:00:00");
    ui.player2Time->setText("00:00:00");
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
    chessBoard->update();
    moveHistory->clear();
    engineOutput->clear();
    statusLabel->setText("新游戏开始");
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

