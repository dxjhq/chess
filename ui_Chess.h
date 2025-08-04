/********************************************************************************
** Form generated from reading UI file 'Chess.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHESS_H
#define UI_CHESS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChessClass
{
public:
    QAction *newGameAction;
    QAction *loadGameAction;
    QAction *saveGameAction;
    QAction *exitAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *settingsAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *showCoordinatesAction;
    QAction *showLastMoveAction;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QFrame *gameFrame;
    QVBoxLayout *gameLayout;
    QLabel *chessBoard;
    QHBoxLayout *playerInfoLayout;
    QFrame *player1Frame;
    QVBoxLayout *player1Layout;
    QLabel *player1Label;
    QLabel *player1Time;
    QFrame *player2Frame;
    QVBoxLayout *player2Layout;
    QLabel *player2Label;
    QLabel *player2Time;
    QFrame *controlFrame;
    QVBoxLayout *controlLayout;
    QGroupBox *gameControlGroup;
    QVBoxLayout *gameControlLayout;
    QPushButton *newGameButton;
    QPushButton *loadGameButton;
    QPushButton *saveGameButton;
    QPushButton *undoButton;
    QPushButton *redoButton;
    QGroupBox *gameInfoGroup;
    QVBoxLayout *gameInfoLayout;
    QLabel *currentPlayerLabel;
    QLabel *gameStatusLabel;
    QLabel *moveCountLabel;
    QGroupBox *moveHistoryGroup;
    QVBoxLayout *moveHistoryLayout;
    QListWidget *moveHistoryList;
    QSpacerItem *verticalSpacer;
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ChessClass)
    {
        if (ChessClass->objectName().isEmpty())
            ChessClass->setObjectName("ChessClass");
        ChessClass->resize(1024, 768);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/Chess/Resources/icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        ChessClass->setWindowIcon(icon);
        newGameAction = new QAction(ChessClass);
        newGameAction->setObjectName("newGameAction");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/Chess/Resources/new.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        newGameAction->setIcon(icon1);
        loadGameAction = new QAction(ChessClass);
        loadGameAction->setObjectName("loadGameAction");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/Chess/Resources/open.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        loadGameAction->setIcon(icon2);
        saveGameAction = new QAction(ChessClass);
        saveGameAction->setObjectName("saveGameAction");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/Chess/Resources/save.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        saveGameAction->setIcon(icon3);
        exitAction = new QAction(ChessClass);
        exitAction->setObjectName("exitAction");
        undoAction = new QAction(ChessClass);
        undoAction->setObjectName("undoAction");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/Chess/Resources/undo.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        undoAction->setIcon(icon4);
        redoAction = new QAction(ChessClass);
        redoAction->setObjectName("redoAction");
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/Chess/Resources/redo.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        redoAction->setIcon(icon5);
        settingsAction = new QAction(ChessClass);
        settingsAction->setObjectName("settingsAction");
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/Chess/Resources/settings.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        settingsAction->setIcon(icon6);
        aboutAction = new QAction(ChessClass);
        aboutAction->setObjectName("aboutAction");
        aboutQtAction = new QAction(ChessClass);
        aboutQtAction->setObjectName("aboutQtAction");
        showCoordinatesAction = new QAction(ChessClass);
        showCoordinatesAction->setObjectName("showCoordinatesAction");
        showCoordinatesAction->setCheckable(true);
        showLastMoveAction = new QAction(ChessClass);
        showLastMoveAction->setObjectName("showLastMoveAction");
        showLastMoveAction->setCheckable(true);
        centralWidget = new QWidget(ChessClass);
        centralWidget->setObjectName("centralWidget");
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName("horizontalLayout");
        gameFrame = new QFrame(centralWidget);
        gameFrame->setObjectName("gameFrame");
        gameFrame->setFrameShape(QFrame::Shape::StyledPanel);
        gameFrame->setFrameShadow(QFrame::Shadow::Raised);
        gameLayout = new QVBoxLayout(gameFrame);
        gameLayout->setSpacing(6);
        gameLayout->setContentsMargins(11, 11, 11, 11);
        gameLayout->setObjectName("gameLayout");
        chessBoard = new QLabel(gameFrame);
        chessBoard->setObjectName("chessBoard");
        chessBoard->setMinimumSize(QSize(480, 540));
        chessBoard->setMaximumSize(QSize(480, 540));
        chessBoard->setStyleSheet(QString::fromUtf8("background-color: rgb(139, 69, 19);\n"
"border: 2px solid black;"));
        chessBoard->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gameLayout->addWidget(chessBoard);

        playerInfoLayout = new QHBoxLayout();
        playerInfoLayout->setSpacing(6);
        playerInfoLayout->setObjectName("playerInfoLayout");
        player1Frame = new QFrame(gameFrame);
        player1Frame->setObjectName("player1Frame");
        player1Frame->setStyleSheet(QString::fromUtf8("background-color: rgb(100, 149, 237);"));
        player1Frame->setFrameShape(QFrame::Shape::StyledPanel);
        player1Frame->setFrameShadow(QFrame::Shadow::Raised);
        player1Layout = new QVBoxLayout(player1Frame);
        player1Layout->setSpacing(6);
        player1Layout->setContentsMargins(11, 11, 11, 11);
        player1Layout->setObjectName("player1Layout");
        player1Label = new QLabel(player1Frame);
        player1Label->setObjectName("player1Label");
        player1Label->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        player1Label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        player1Layout->addWidget(player1Label);

        player1Time = new QLabel(player1Frame);
        player1Time->setObjectName("player1Time");
        player1Time->setStyleSheet(QString::fromUtf8("color: white; font-size: 18px; font-weight: bold;"));
        player1Time->setAlignment(Qt::AlignmentFlag::AlignCenter);

        player1Layout->addWidget(player1Time);


        playerInfoLayout->addWidget(player1Frame);

        player2Frame = new QFrame(gameFrame);
        player2Frame->setObjectName("player2Frame");
        player2Frame->setStyleSheet(QString::fromUtf8("background-color: rgb(220, 20, 60);"));
        player2Frame->setFrameShape(QFrame::Shape::StyledPanel);
        player2Frame->setFrameShadow(QFrame::Shadow::Raised);
        player2Layout = new QVBoxLayout(player2Frame);
        player2Layout->setSpacing(6);
        player2Layout->setContentsMargins(11, 11, 11, 11);
        player2Layout->setObjectName("player2Layout");
        player2Label = new QLabel(player2Frame);
        player2Label->setObjectName("player2Label");
        player2Label->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        player2Label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        player2Layout->addWidget(player2Label);

        player2Time = new QLabel(player2Frame);
        player2Time->setObjectName("player2Time");
        player2Time->setStyleSheet(QString::fromUtf8("color: white; font-size: 18px; font-weight: bold;"));
        player2Time->setAlignment(Qt::AlignmentFlag::AlignCenter);

        player2Layout->addWidget(player2Time);


        playerInfoLayout->addWidget(player2Frame);


        gameLayout->addLayout(playerInfoLayout);


        horizontalLayout->addWidget(gameFrame);

        controlFrame = new QFrame(centralWidget);
        controlFrame->setObjectName("controlFrame");
        controlFrame->setMinimumSize(QSize(300, 0));
        controlFrame->setMaximumSize(QSize(300, 16777215));
        controlFrame->setFrameShape(QFrame::Shape::StyledPanel);
        controlFrame->setFrameShadow(QFrame::Shadow::Raised);
        controlLayout = new QVBoxLayout(controlFrame);
        controlLayout->setSpacing(6);
        controlLayout->setContentsMargins(11, 11, 11, 11);
        controlLayout->setObjectName("controlLayout");
        gameControlGroup = new QGroupBox(controlFrame);
        gameControlGroup->setObjectName("gameControlGroup");
        gameControlLayout = new QVBoxLayout(gameControlGroup);
        gameControlLayout->setSpacing(6);
        gameControlLayout->setContentsMargins(11, 11, 11, 11);
        gameControlLayout->setObjectName("gameControlLayout");
        newGameButton = new QPushButton(gameControlGroup);
        newGameButton->setObjectName("newGameButton");

        gameControlLayout->addWidget(newGameButton);

        loadGameButton = new QPushButton(gameControlGroup);
        loadGameButton->setObjectName("loadGameButton");

        gameControlLayout->addWidget(loadGameButton);

        saveGameButton = new QPushButton(gameControlGroup);
        saveGameButton->setObjectName("saveGameButton");

        gameControlLayout->addWidget(saveGameButton);

        undoButton = new QPushButton(gameControlGroup);
        undoButton->setObjectName("undoButton");

        gameControlLayout->addWidget(undoButton);

        redoButton = new QPushButton(gameControlGroup);
        redoButton->setObjectName("redoButton");

        gameControlLayout->addWidget(redoButton);


        controlLayout->addWidget(gameControlGroup);

        gameInfoGroup = new QGroupBox(controlFrame);
        gameInfoGroup->setObjectName("gameInfoGroup");
        gameInfoLayout = new QVBoxLayout(gameInfoGroup);
        gameInfoLayout->setSpacing(6);
        gameInfoLayout->setContentsMargins(11, 11, 11, 11);
        gameInfoLayout->setObjectName("gameInfoLayout");
        currentPlayerLabel = new QLabel(gameInfoGroup);
        currentPlayerLabel->setObjectName("currentPlayerLabel");

        gameInfoLayout->addWidget(currentPlayerLabel);

        gameStatusLabel = new QLabel(gameInfoGroup);
        gameStatusLabel->setObjectName("gameStatusLabel");

        gameInfoLayout->addWidget(gameStatusLabel);

        moveCountLabel = new QLabel(gameInfoGroup);
        moveCountLabel->setObjectName("moveCountLabel");

        gameInfoLayout->addWidget(moveCountLabel);


        controlLayout->addWidget(gameInfoGroup);

        moveHistoryGroup = new QGroupBox(controlFrame);
        moveHistoryGroup->setObjectName("moveHistoryGroup");
        moveHistoryLayout = new QVBoxLayout(moveHistoryGroup);
        moveHistoryLayout->setSpacing(6);
        moveHistoryLayout->setContentsMargins(11, 11, 11, 11);
        moveHistoryLayout->setObjectName("moveHistoryLayout");
        moveHistoryList = new QListWidget(moveHistoryGroup);
        moveHistoryList->setObjectName("moveHistoryList");
        moveHistoryList->setMaximumSize(QSize(16777215, 200));

        moveHistoryLayout->addWidget(moveHistoryList);


        controlLayout->addWidget(moveHistoryGroup);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        controlLayout->addItem(verticalSpacer);


        horizontalLayout->addWidget(controlFrame);

        ChessClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ChessClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1024, 33));
        fileMenu = new QMenu(menuBar);
        fileMenu->setObjectName("fileMenu");
        editMenu = new QMenu(menuBar);
        editMenu->setObjectName("editMenu");
        viewMenu = new QMenu(menuBar);
        viewMenu->setObjectName("viewMenu");
        helpMenu = new QMenu(menuBar);
        helpMenu->setObjectName("helpMenu");
        ChessClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ChessClass);
        mainToolBar->setObjectName("mainToolBar");
        ChessClass->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ChessClass);
        statusBar->setObjectName("statusBar");
        ChessClass->setStatusBar(statusBar);

        menuBar->addAction(fileMenu->menuAction());
        menuBar->addAction(editMenu->menuAction());
        menuBar->addAction(viewMenu->menuAction());
        menuBar->addAction(helpMenu->menuAction());
        fileMenu->addAction(newGameAction);
        fileMenu->addSeparator();
        fileMenu->addAction(loadGameAction);
        fileMenu->addAction(saveGameAction);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAction);
        editMenu->addAction(undoAction);
        editMenu->addAction(redoAction);
        editMenu->addSeparator();
        editMenu->addAction(settingsAction);
        viewMenu->addAction(showCoordinatesAction);
        viewMenu->addAction(showLastMoveAction);
        helpMenu->addAction(aboutAction);
        helpMenu->addAction(aboutQtAction);
        mainToolBar->addAction(newGameAction);
        mainToolBar->addAction(loadGameAction);
        mainToolBar->addAction(saveGameAction);
        mainToolBar->addSeparator();
        mainToolBar->addAction(undoAction);
        mainToolBar->addAction(redoAction);
        mainToolBar->addSeparator();
        mainToolBar->addAction(settingsAction);

        retranslateUi(ChessClass);
        QObject::connect(newGameAction, SIGNAL(triggered()), ChessClass, SLOT(onNewGame()));
        QObject::connect(loadGameAction, SIGNAL(triggered()), ChessClass, SLOT(onLoadGame()));
        QObject::connect(saveGameAction, SIGNAL(triggered()), ChessClass, SLOT(onSaveGame()));
        QObject::connect(settingsAction, SIGNAL(triggered()), ChessClass, SLOT(onSettings()));
        QObject::connect(aboutAction, SIGNAL(triggered()), ChessClass, SLOT(onAbout()));
        QObject::connect(newGameButton, SIGNAL(clicked()), ChessClass, SLOT(onNewGame()));
        QObject::connect(loadGameButton, SIGNAL(clicked()), ChessClass, SLOT(onLoadGame()));
        QObject::connect(saveGameButton, SIGNAL(clicked()), ChessClass, SLOT(onSaveGame()));

        QMetaObject::connectSlotsByName(ChessClass);
    } // setupUi

    void retranslateUi(QMainWindow *ChessClass)
    {
        ChessClass->setWindowTitle(QCoreApplication::translate("ChessClass", "\351\262\250\351\261\274\350\261\241\346\243\213 V1.8.0", nullptr));
        newGameAction->setText(QCoreApplication::translate("ChessClass", "\346\226\260\346\270\270\346\210\217", nullptr));
#if QT_CONFIG(tooltip)
        newGameAction->setToolTip(QCoreApplication::translate("ChessClass", "\345\274\200\345\247\213\346\226\260\346\270\270\346\210\217", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        newGameAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        loadGameAction->setText(QCoreApplication::translate("ChessClass", "\350\275\275\345\205\245\346\270\270\346\210\217", nullptr));
#if QT_CONFIG(tooltip)
        loadGameAction->setToolTip(QCoreApplication::translate("ChessClass", "\350\275\275\345\205\245\344\277\235\345\255\230\347\232\204\346\270\270\346\210\217", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        loadGameAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        saveGameAction->setText(QCoreApplication::translate("ChessClass", "\344\277\235\345\255\230\346\270\270\346\210\217", nullptr));
#if QT_CONFIG(tooltip)
        saveGameAction->setToolTip(QCoreApplication::translate("ChessClass", "\344\277\235\345\255\230\345\275\223\345\211\215\346\270\270\346\210\217", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        saveGameAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        exitAction->setText(QCoreApplication::translate("ChessClass", "\351\200\200\345\207\272", nullptr));
#if QT_CONFIG(shortcut)
        exitAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        undoAction->setText(QCoreApplication::translate("ChessClass", "\346\202\224\346\243\213", nullptr));
#if QT_CONFIG(tooltip)
        undoAction->setToolTip(QCoreApplication::translate("ChessClass", "\346\222\244\351\224\200\344\270\212\344\270\200\346\255\245", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        undoAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        redoAction->setText(QCoreApplication::translate("ChessClass", "\351\207\215\345\201\232", nullptr));
#if QT_CONFIG(tooltip)
        redoAction->setToolTip(QCoreApplication::translate("ChessClass", "\351\207\215\345\201\232\344\270\212\344\270\200\346\255\245", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        redoAction->setShortcut(QCoreApplication::translate("ChessClass", "Ctrl+Y", nullptr));
#endif // QT_CONFIG(shortcut)
        settingsAction->setText(QCoreApplication::translate("ChessClass", "\350\256\276\347\275\256", nullptr));
#if QT_CONFIG(tooltip)
        settingsAction->setToolTip(QCoreApplication::translate("ChessClass", "\346\270\270\346\210\217\350\256\276\347\275\256", nullptr));
#endif // QT_CONFIG(tooltip)
        aboutAction->setText(QCoreApplication::translate("ChessClass", "\345\205\263\344\272\216", nullptr));
        aboutQtAction->setText(QCoreApplication::translate("ChessClass", "\345\205\263\344\272\216Qt", nullptr));
        showCoordinatesAction->setText(QCoreApplication::translate("ChessClass", "\346\230\276\347\244\272\345\235\220\346\240\207", nullptr));
        showLastMoveAction->setText(QCoreApplication::translate("ChessClass", "\346\230\276\347\244\272\346\234\200\345\220\216\344\270\200\346\255\245", nullptr));
        chessBoard->setText(QString());
        player1Label->setText(QCoreApplication::translate("ChessClass", "\345\260\206 Player1", nullptr));
        player1Time->setText(QCoreApplication::translate("ChessClass", "00:00:00", nullptr));
        player2Label->setText(QCoreApplication::translate("ChessClass", "\345\270\205 Player2", nullptr));
        player2Time->setText(QCoreApplication::translate("ChessClass", "00:00:00", nullptr));
        gameControlGroup->setTitle(QCoreApplication::translate("ChessClass", "\346\270\270\346\210\217\346\216\247\345\210\266", nullptr));
        newGameButton->setText(QCoreApplication::translate("ChessClass", "\346\226\260\346\270\270\346\210\217", nullptr));
        loadGameButton->setText(QCoreApplication::translate("ChessClass", "\350\275\275\345\205\245\346\270\270\346\210\217", nullptr));
        saveGameButton->setText(QCoreApplication::translate("ChessClass", "\344\277\235\345\255\230\346\270\270\346\210\217", nullptr));
        undoButton->setText(QCoreApplication::translate("ChessClass", "\346\202\224\346\243\213", nullptr));
        redoButton->setText(QCoreApplication::translate("ChessClass", "\351\207\215\345\201\232", nullptr));
        gameInfoGroup->setTitle(QCoreApplication::translate("ChessClass", "\346\270\270\346\210\217\344\277\241\346\201\257", nullptr));
        currentPlayerLabel->setText(QCoreApplication::translate("ChessClass", "\345\275\223\345\211\215\347\216\251\345\256\266: \347\272\242\346\226\271", nullptr));
        gameStatusLabel->setText(QCoreApplication::translate("ChessClass", "\346\270\270\346\210\217\347\212\266\346\200\201: \350\277\233\350\241\214\344\270\255", nullptr));
        moveCountLabel->setText(QCoreApplication::translate("ChessClass", "\345\233\236\345\220\210\346\225\260: 1", nullptr));
        moveHistoryGroup->setTitle(QCoreApplication::translate("ChessClass", "\346\243\213\350\260\261", nullptr));
        fileMenu->setTitle(QCoreApplication::translate("ChessClass", "\346\226\207\344\273\266", nullptr));
        editMenu->setTitle(QCoreApplication::translate("ChessClass", "\347\274\226\350\276\221", nullptr));
        viewMenu->setTitle(QCoreApplication::translate("ChessClass", "\350\247\206\345\233\276", nullptr));
        helpMenu->setTitle(QCoreApplication::translate("ChessClass", "\345\270\256\345\212\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChessClass: public Ui_ChessClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHESS_H
