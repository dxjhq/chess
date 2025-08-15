#ifndef PLATFORMCONNECTOR_H
#define PLATFORMCONNECTOR_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>
#include <QPixmap>
#include <QWindow>
#include <QApplication>
#include <QScreen>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <memory>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#endif

/**
 * @brief 平台连接器 - 支持多平台象棋游戏连线
 * 
 * 支持的平台:
 * - 天天象棋
 * - QQ象棋  
 * - JJ象棋
 * - 中国象棋大师
 * - 自定义平台
 */
class PlatformConnector : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 支持的平台类型
     */
    enum PlatformType {
        PLATFORM_UNKNOWN = 0,
        PLATFORM_TIANTIAN,      // 天天象棋
        PLATFORM_QQ,            // QQ象棋
        PLATFORM_JJ,            // JJ象棋
        PLATFORM_MASTER,        // 中国象棋大师
        PLATFORM_CUSTOM         // 自定义平台
    };
    Q_ENUM(PlatformType)

    /**
     * @brief 连接状态
     */
    enum ConnectionStatus {
        STATUS_DISCONNECTED = 0,
        STATUS_CONNECTING,
        STATUS_CONNECTED,
        STATUS_ERROR
    };
    Q_ENUM(ConnectionStatus)

    /**
     * @brief 平台配置信息
     */
    struct PlatformConfig {
        QString name;               // 平台名称
        QString windowClass;        // 窗口类名
        QString windowTitle;        // 窗口标题
        QString clickClass;         // 点击目标类名
        QString clickTitle;         // 点击目标标题
        QRect boardArea;           // 棋盘区域
        QPoint boardOffset;        // 棋盘偏移
        QSize cellSize;            // 格子大小
        int animationDelay;        // 动画延迟(ms)
        bool useChildWindow;       // 是否使用子窗口
        
        PlatformConfig() : 
            boardArea(0, 0, 360, 400),
            boardOffset(20, 20),
            cellSize(40, 40),
            animationDelay(500),
            useChildWindow(false) {}
    };

    explicit PlatformConnector(QObject *parent = nullptr);
    ~PlatformConnector();

    // 平台管理
    /**
     * @brief 获取支持的平台列表
     */
    QStringList getSupportedPlatforms() const;
    
    /**
     * @brief 设置当前平台
     */
    bool setPlatform(PlatformType platform);
    bool setPlatform(const QString& platformName);
    
    /**
     * @brief 获取当前平台
     */
    PlatformType getCurrentPlatform() const { return currentPlatform; }
    QString getCurrentPlatformName() const;
    
    /**
     * @brief 添加自定义平台配置
     */
    bool addCustomPlatform(const QString& name, const PlatformConfig& config);
    
    /**
     * @brief 获取平台配置
     */
    PlatformConfig getPlatformConfig(PlatformType platform) const;
    PlatformConfig getPlatformConfig(const QString& platformName) const;

    // 连接管理
    /**
     * @brief 连接到游戏窗口
     */
    bool connectToGame();
    
    /**
     * @brief 断开连接
     */
    void disconnect();
    
    /**
     * @brief 获取连接状态
     */
    ConnectionStatus getConnectionStatus() const { return connectionStatus; }
    
    /**
     * @brief 检查连接是否有效
     */
    bool isConnected() const { return connectionStatus == STATUS_CONNECTED; }

    // 窗口操作
    /**
     * @brief 查找游戏窗口
     */
    bool findGameWindow();
    
    /**
     * @brief 获取窗口截图
     */
    QPixmap captureWindow();
    
    /**
     * @brief 获取棋盘区域截图
     */
    QPixmap captureBoardArea();

    // 鼠标操作
    /**
     * @brief 在指定棋盘位置点击
     * @param row 行 (0-9)
     * @param col 列 (0-8)
     */
    bool clickBoardPosition(int row, int col);
    
    /**
     * @brief 执行走法
     * @param fromRow 起始行
     * @param fromCol 起始列
     * @param toRow 目标行
     * @param toCol 目标列
     */
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol);
    
    /**
     * @brief 在窗口坐标点击
     */
    bool clickWindowPosition(const QPoint& pos);

    // 配置管理
    /**
     * @brief 保存配置
     */
    void saveSettings();
    
    /**
     * @brief 加载配置
     */
    void loadSettings();
    
    /**
     * @brief 导出平台配置
     */
    bool exportPlatformConfig(const QString& filePath) const;
    
    /**
     * @brief 导入平台配置
     */
    bool importPlatformConfig(const QString& filePath);

    // 调试功能
    /**
     * @brief 设置调试模式
     */
    void setDebugMode(bool enabled) { debugMode = enabled; }
    
    /**
     * @brief 获取调试信息
     */
    QString getDebugInfo() const;

signals:
    /**
     * @brief 连接状态改变
     */
    void connectionStatusChanged(ConnectionStatus status);
    
    /**
     * @brief 平台改变
     */
    void platformChanged(PlatformType platform);
    
    /**
     * @brief 错误信息
     */
    void errorOccurred(const QString& error);
    
    /**
     * @brief 调试信息
     */
    void debugMessage(const QString& message);
    
    /**
     * @brief 窗口找到
     */
    void gameWindowFound(const QString& windowTitle);
    
    /**
     * @brief 鼠标点击完成
     */
    void clickCompleted(int row, int col);

private slots:
    /**
     * @brief 检查窗口状态
     */
    void checkWindowStatus();

private:
    // 初始化
    void initializePlatforms();
    void setupDefaultConfigs();
    
    // 窗口操作 (平台相关)
#ifdef Q_OS_WIN
    HWND findWindowByClass(const QString& className, const QString& title = QString());
    HWND findChildWindow(HWND parent, const QString& className, const QString& title = QString());
    QRect getWindowRect(HWND hwnd);
    QPixmap captureWindow(HWND hwnd);
    bool sendMouseClick(HWND hwnd, const QPoint& pos);
    QString getWindowTitle(HWND hwnd);
    QString getWindowClass(HWND hwnd);
#endif
    
    // 坐标转换
    QPoint boardToWindow(int row, int col) const;
    QPoint windowToBoard(const QPoint& pos) const;
    
    // 配置转换
    QJsonObject configToJson(const PlatformConfig& config) const;
    PlatformConfig configFromJson(const QJsonObject& json) const;
    
    // 调试输出
    void debugLog(const QString& message) const;

private:
    // 当前状态
    PlatformType currentPlatform;
    ConnectionStatus connectionStatus;
    
    // 平台配置
    QMap<PlatformType, PlatformConfig> platformConfigs;
    QMap<QString, PlatformConfig> customPlatforms;
    
    // 窗口句柄
#ifdef Q_OS_WIN
    HWND gameWindow;
    HWND clickWindow;
#else
    void* gameWindow;
    void* clickWindow;
#endif
    
    // 定时器
    QTimer* statusTimer;
    
    // 设置
    QSettings* settings;
    
    // 调试
    bool debugMode;
    
    // 线程安全
    mutable QMutex mutex;
    
    // 常量
    static const QString SETTINGS_GROUP;
    static const QString CONFIG_DIR;
    static const int STATUS_CHECK_INTERVAL;
};

#endif // PLATFORMCONNECTOR_H