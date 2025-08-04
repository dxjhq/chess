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
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QVector>
#include "ui_Chess.h"

// 棋子类型枚举
enum PieceType {
    NONE = 0,
    RED_KING,    // 红帅
    RED_ADVISOR, // 红仕
    RED_BISHOP,  // 红相
    RED_KNIGHT,  // 红马
    RED_ROOK,    // 红车
    RED_CANNON,  // 红炮
    RED_PAWN,    // 红兵
    BLACK_KING,  // 黑将
    BLACK_ADVISOR, // 黑士
    BLACK_BISHOP,  // 黑象
    BLACK_KNIGHT,  // 黑马
    BLACK_ROOK,    // 黑车
    BLACK_CANNON,  // 黑炮
    BLACK_PAWN     // 黑卒
};

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

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void setPieceStyle(PieceStyle style);
    
private:
    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawPiece(QPainter &painter, int x, int y, PieceType piece);
    void drawPieceWithFont(QPainter &painter, int x, int y, PieceType piece);
    void drawPieceWithImage(QPainter &painter, int x, int y, PieceType piece);
    QPoint boardToPixel(int row, int col);
    QPoint pixelToBoard(QPoint pixel);
    QColor getPieceColor(PieceType piece);
    
    PieceStyleManager& styleManager;

private:
    static const int BOARD_WIDTH = 9;   // 棋盘宽度（列数）
    static const int BOARD_HEIGHT = 10; // 棋盘高度（行数）
    static const int CELL_SIZE = 50;    // 每个格子的大小
    static const int BOARD_MARGIN = 50; // 棋盘边距
    
    PieceType board[BOARD_HEIGHT][BOARD_WIDTH]; // 棋盘状态
    QPoint selectedPos; // 选中的位置
    bool pieceSelected; // 是否有棋子被选中
    QPoint mousePos;    // 鼠标位置
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

private:
    void updateGameInfo();
    void setupStyleMenu();
    // void setupUI();
    // void setupMenuBar();
    // void setupToolBar();
    // void setupStatusBar();
    // void setupMainLayout();
    
    QComboBox* styleComboBox;
    ChessBoard* chessBoard;

private:
    Ui::ChessClass ui;
    
    // 右侧控制面板组件
    QWidget *controlPanel;
    QGroupBox *gameControlGroup;
    QGroupBox *engineGroup;
    QGroupBox *analysisGroup;
    QGroupBox *settingsGroup;
    
    QPushButton *newGameBtn;
    QPushButton *loadGameBtn;
    QPushButton *saveGameBtn;
    QPushButton *undoBtn;
    QPushButton *redoBtn;
    QPushButton *analyzeBtn;
    QPushButton *hintBtn;
    
    QTextEdit *moveHistory;
    QTextEdit *engineOutput;
    QLabel *statusLabel;
    QProgressBar *thinkingProgress;
    
    QCheckBox *autoPlayCheck;
    QCheckBox *showHintsCheck;
    QComboBox *difficultyCombo;
    QSpinBox *thinkTimeSpinBox;
};

