#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QToolBar>
#include <QAction>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QVector>
#include <QClipboard>
#include <QApplication>
#include <QTimer>
#include "ui_Chess.h"
#include "ChessEngine.h"
#include "MoveHistory.h"
#include "AIEngine.h"

// 前向声明
class ConnectionDialog;

// 棋子显示样式枚举
enum PieceStyle {
    STYLE_FONT_TRADITIONAL,  // 传统字体样式
    STYLE_FONT_MODERN,       // 现代字体样式
    STYLE_IMAGE_CLASSIC,     // 经典图片样式
    STYLE_IMAGE_MODERN       // 现代图片样式
};

// 棋子样式管理器
class PieceStyleManager {
public:
    static PieceStyleManager& getInstance();
    
    void setPieceStyle(PieceStyle style);
    PieceStyle getPieceStyle() const;
    
    QString getPieceText(PieceType piece) const;
    QString getPieceImagePath(PieceType piece) const;
    QFont getPieceFont() const;
    QColor getPieceTextColor(PieceType piece) const;
    QColor getPieceBackgroundColor(PieceType piece) const;
    
private:
    PieceStyleManager();
    PieceStyle currentStyle;
    
    QString getTraditionalText(PieceType piece) const;
    QString getModernText(PieceType piece) const;
};

// 棋盘绘制类
class ChessBoard : public QWidget
{
    Q_OBJECT

public:
    ChessBoard(QWidget *parent = nullptr);
    ~ChessBoard();
    void initializeBoard();
    
    // 游戏控制
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol);
    void undoMove();
    void redoMove();
    void goToMove(int index);
    
    // 状态查询
    bool isGameOver() const;
    bool isRedTurn() const;
    QString getGameStatus() const;
    
    // 棋盘状态
    void setBoard(const PieceType board[10][9]);
    void copyBoard(PieceType board[10][9]) const;
    
    // 走法历史
    const MoveHistory& getMoveHistory() const { return moveHistory; }
    void setMoveHistory(const MoveHistory& history);
    
    // 引擎访问
    ChessEngine& getEngine() { return engine; }
    const ChessEngine& getEngine() const { return engine; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void setPieceStyle(PieceStyle style);
    
signals:
    void moveExecuted(const Move& move);
    void gameStatusChanged(const QString& status);
    void boardChanged();
    
private:
    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawPiece(QPainter &painter, int x, int y, PieceType piece);
    void drawPieceWithFont(QPainter &painter, int x, int y, PieceType piece);
    void drawPieceWithImage(QPainter &painter, int x, int y, PieceType piece);
    void drawSelection(QPainter &painter);
    void drawValidMoves(QPainter &painter);
    QPoint boardToPixel(int row, int col);
    QPoint pixelToBoard(QPoint pixel);
    QColor getPieceColor(PieceType piece);
    
    void updateGameStatus();
    void highlightValidMoves();
    
    PieceStyleManager& styleManager;

private:
    static const int BOARD_WIDTH = 9;   // 棋盘宽度（列数）
    static const int BOARD_HEIGHT = 10; // 棋盘高度（行数）
    static const int CELL_SIZE = 50;    // 每个格子的大小
    static const int BOARD_MARGIN = 50; // 棋盘边距
    
    ChessEngine engine;         // 象棋引擎
    MoveHistory moveHistory;    // 走法历史
    
    QPoint selectedPos;         // 选中的位置
    bool pieceSelected;         // 是否有棋子被选中
    QPoint mousePos;            // 鼠标位置
    std::vector<Move> validMoves; // 当前选中棋子的合法走法
    QString gameStatus;         // 游戏状态信息
};

class Chess : public QMainWindow
{
    Q_OBJECT

public:
    Chess(QWidget *parent = nullptr);
    ~Chess();

private slots:
    void onNewGame();
    void onLoadGame();
    void onSaveGame();
    void onSettings();
    void onAbout();
    void onStyleChanged();
    
    // 棋盘事件处理
    void onMoveExecuted(const Move& move);
    void onGameStatusChanged(const QString& status);
    void onBoardChanged();
    
    // 走法导航
    void onMoveFirst();
    void onMovePrevious();
    void onMoveNext();
    void onMoveLast();
    void onFlipBoard();
    void onCopyMoves();
    
    // AI相关槽函数
    void onAIEnabled(bool enabled);
    void onAIDifficultyChanged();
    void onAIDepthChanged(int depth);
    void onAITimeChanged(int timeMs);
    void onAIMove();
    void onStopAI();
    
    // 连线功能槽函数
    void onConnection();
    
    // 连线管理相关槽函数
    void onConnect();                    // 连接槽函数
    void onDisconnect();                 // 断开连接槽函数
    void onConnectionSchemeChanged(int index);    // 连接方案改变槽函数
    void onConnectionStatusClicked();    // 连接状态按钮点击槽函数
    void updateConnectionStatus(); // 更新连接状态
    
    // 文件菜单槽函数
    void onImportPGN();
    void onExportPGN();
    
    // 编辑菜单槽函数
    void onCopyPosition();
    void onPastePosition();
    void onSetupPosition();
    
    // 查看菜单槽函数
    void onShowCoordinates(bool show);
    void onShowMoveHistory(bool show);
    void onShowEvaluation(bool show);
    void onFullScreen();
    
    // 游戏菜单槽函数
    void onEngineMatch();
    void onAnalyzePosition();
    void onAnalyzeGame();
    
    // 引擎菜单槽函数
    void onEngineSettings();
    void onLoadEngine();
    void onStartEngine();
    void onStopEngine();
    
    // 开局库菜单槽函数
    void onOpenDatabase();
    void onCreateDatabase();
    void onSearchPosition();
    
    // 工具菜单槽函数
    void onOptions();
    void onStatistics();
    
    // 帮助菜单槽函数
    void onHelp();

private:
    void updateGameInfo();
    void setupStyleMenu();
    void initializeNewControls();
    void setupAIEngine();
    void updateAIControls();
    void makeAIMove();
    // void setupUI();
    // void setupMenuBar();
    void setupToolBar();  // 设置工具栏
    void setupConnectionToolBar();  // 设置连线管理工具栏
    // void setupStatusBar();
    // void setupMainLayout();
    
    QComboBox* styleComboBox;
    ChessBoard* chessBoard;

private:
    Ui::ChessClass ui;
    
    // 工具栏相关成员变量
    QToolBar* mainToolBar;           // 主工具栏
    QToolBar* connectionToolBar;     // 连线管理工具栏
    
    // 连线管理工具栏按钮
    QAction* connectAction;          // 连接动作
    QAction* disconnectAction;       // 断开连接动作
    QAction* connectionSchemeAction; // 连接方案动作
    QComboBox* connectionSchemeCombo; // 连接方案选择框
    QPushButton* connectionStatusBtn; // 连接状态按钮
    QLabel* connectionStatusLabel;   // 连接状态标签
    
    // 连线管理相关状态
    bool isConnected;                // 连接状态
    QString currentScheme;           // 当前连接方案
    
    // UI控件指针 - 4个独立的dock窗口
    QDockWidget *gameControlDockWidget;
    QDockWidget *timeControlDockWidget;
    QDockWidget *engineControlDockWidget;
    QDockWidget *moveHistoryDockWidget;
    
    QGroupBox *gameControlGroup;
    QGroupBox *timeControlGroup;
    QGroupBox *engineGroup;
    QGroupBox *navigationGroup;
    QGroupBox *moveHistoryGroup;
    
    // 游戏控制按钮
    QPushButton *newGameBtn;
    QPushButton *loadGameBtn;
    QPushButton *saveGameBtn;
    QPushButton *undoBtn;
    QPushButton *redoBtn;
    
    // 时间控制标签
    QLabel *blackTimeLabel;
    QLabel *blackUsedTimeLabel;
    QLabel *blackLastMoveLabel;
    QLabel *blackRemainingLabel;
    QLabel *redTimeLabel;
    QLabel *redUsedTimeLabel;
    QLabel *redLastMoveLabel;
    QLabel *redRemainingLabel;
    
    // 引擎控制组件
    QPushButton *addEngineButton;
    QPushButton *engineManageButton;
    QPushButton *multiEngineButton;
    QPushButton *deleteEngineButton;
    QCheckBox *engineEnabledCheck;
    QComboBox *engineComboBox;
    QSpinBox *engineDepthSpinBox;
    QSpinBox *engineTimeSpinBox;
    
    // 导航按钮
    QPushButton *firstMoveButton;
    QPushButton *prevMoveButton;
    QPushButton *nextMoveButton;
    QPushButton *lastMoveButton;
    QPushButton *openBookButton;
    
    // 棋谱导航组件
    QTableWidget *moveHistoryTable;
    QLabel *moveAnalysisLabel;
    QPushButton *moveFirstButton;
    QPushButton *movePrevButton;
    QPushButton *moveNextButton;
    QPushButton *moveLastButton;
    QPushButton *flipBoardButton;
    QPushButton *copyButton;
    
    QLabel *statusLabel;
    QProgressBar *thinkingProgress;
    
    // AI引擎相关
    AIEngine *aiEngine;
    bool aiEnabled;
    bool aiThinking;
    QTimer *aiTimer;
    
    // AI难度设置
    QComboBox *aiDifficultyCombo;
    QPushButton *aiMoveButton;
    QPushButton *stopAIButton;
    QLabel *aiStatusLabel;
    
    // 连线功能相关
    ConnectionDialog *connectionDialog;
};

