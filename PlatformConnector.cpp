#include "PlatformConnector.h"
#include <QApplication>
#include <QScreen>
#include <QStandardPaths>
#include <QMessageBox>
#include <QThread>

// 静态常量定义
const QString PlatformConnector::SETTINGS_GROUP = "PlatformConnector";
const QString PlatformConnector::CONFIG_DIR = "platforms";
const int PlatformConnector::STATUS_CHECK_INTERVAL = 1000; // 1秒

/**
 * @brief 构造函数
 */
PlatformConnector::PlatformConnector(QObject *parent)
    : QObject(parent)
    , currentPlatform(PLATFORM_UNKNOWN)
    , connectionStatus(STATUS_DISCONNECTED)
    , gameWindow(nullptr)
    , clickWindow(nullptr)
    , statusTimer(new QTimer(this))
    , settings(new QSettings(this))
    , debugMode(false)
{
    // 初始化平台配置
    initializePlatforms();
    
    // 设置状态检查定时器
    statusTimer->setInterval(STATUS_CHECK_INTERVAL);
    connect(statusTimer, &QTimer::timeout, this, &PlatformConnector::checkWindowStatus);
    
    // 加载设置
    loadSettings();
    
    debugLog("PlatformConnector initialized");
}

/**
 * @brief 析构函数
 */
PlatformConnector::~PlatformConnector()
{
    disconnect();
    saveSettings();
    debugLog("PlatformConnector destroyed");
}

/**
 * @brief 初始化平台配置
 */
void PlatformConnector::initializePlatforms()
{
    setupDefaultConfigs();
}

/**
 * @brief 设置默认平台配置
 */
void PlatformConnector::setupDefaultConfigs()
{
    // 天天象棋配置
    PlatformConfig ttConfig;
    ttConfig.name = "天天象棋";
    ttConfig.windowClass = "";
    ttConfig.windowTitle = "天天象棋";
    ttConfig.clickClass = "";
    ttConfig.clickTitle = "";
    ttConfig.boardArea = QRect(50, 80, 360, 400);
    ttConfig.boardOffset = QPoint(20, 20);
    ttConfig.cellSize = QSize(40, 44);
    ttConfig.animationDelay = 800;
    ttConfig.useChildWindow = false;
    platformConfigs[PLATFORM_TIANTIAN] = ttConfig;
    
    // QQ象棋配置
    PlatformConfig qqConfig;
    qqConfig.name = "QQ象棋";
    qqConfig.windowClass = "";
    qqConfig.windowTitle = "QQ游戏";
    qqConfig.clickClass = "";
    qqConfig.clickTitle = "";
    qqConfig.boardArea = QRect(60, 100, 360, 400);
    qqConfig.boardOffset = QPoint(25, 25);
    qqConfig.cellSize = QSize(40, 44);
    qqConfig.animationDelay = 600;
    qqConfig.useChildWindow = true;
    platformConfigs[PLATFORM_QQ] = qqConfig;
    
    // JJ象棋配置
    PlatformConfig jjConfig;
    jjConfig.name = "JJ象棋";
    jjConfig.windowClass = "";
    jjConfig.windowTitle = "JJ象棋";
    jjConfig.clickClass = "";
    jjConfig.clickTitle = "";
    jjConfig.boardArea = QRect(40, 70, 360, 400);
    jjConfig.boardOffset = QPoint(18, 18);
    jjConfig.cellSize = QSize(40, 44);
    jjConfig.animationDelay = 500;
    jjConfig.useChildWindow = false;
    platformConfigs[PLATFORM_JJ] = jjConfig;
    
    // 中国象棋大师配置
    PlatformConfig masterConfig;
    masterConfig.name = "中国象棋大师";
    masterConfig.windowClass = "";
    masterConfig.windowTitle = "中国象棋大师";
    masterConfig.clickClass = "";
    masterConfig.clickTitle = "";
    masterConfig.boardArea = QRect(30, 60, 360, 400);
    masterConfig.boardOffset = QPoint(15, 15);
    masterConfig.cellSize = QSize(40, 44);
    masterConfig.animationDelay = 400;
    masterConfig.useChildWindow = false;
    platformConfigs[PLATFORM_MASTER] = masterConfig;
}

/**
 * @brief 获取支持的平台列表
 */
QStringList PlatformConnector::getSupportedPlatforms() const
{
    QStringList platforms;
    
    for (auto it = platformConfigs.begin(); it != platformConfigs.end(); ++it) {
        if (it.key() != PLATFORM_UNKNOWN) {
            platforms << it.value().name;
        }
    }
    
    // 添加自定义平台
    for (auto it = customPlatforms.begin(); it != customPlatforms.end(); ++it) {
        platforms << it.key();
    }
    
    return platforms;
}

/**
 * @brief 设置当前平台
 */
bool PlatformConnector::setPlatform(PlatformType platform)
{
    QMutexLocker locker(&mutex);
    
    if (platform == currentPlatform) {
        return true;
    }
    
    if (!platformConfigs.contains(platform)) {
        debugLog(QString("Unknown platform type: %1").arg(platform));
        return false;
    }
    
    // 断开当前连接
    if (connectionStatus != STATUS_DISCONNECTED) {
        disconnect();
    }
    
    currentPlatform = platform;
    debugLog(QString("Platform changed to: %1").arg(getCurrentPlatformName()));
    
    emit platformChanged(platform);
    return true;
}

/**
 * @brief 设置当前平台（通过名称）
 */
bool PlatformConnector::setPlatform(const QString& platformName)
{
    // 查找内置平台
    for (auto it = platformConfigs.begin(); it != platformConfigs.end(); ++it) {
        if (it.value().name == platformName) {
            return setPlatform(it.key());
        }
    }
    
    // 查找自定义平台
    if (customPlatforms.contains(platformName)) {
        QMutexLocker locker(&mutex);
        
        if (connectionStatus != STATUS_DISCONNECTED) {
            disconnect();
        }
        
        currentPlatform = PLATFORM_CUSTOM;
        debugLog(QString("Custom platform set: %1").arg(platformName));
        
        emit platformChanged(PLATFORM_CUSTOM);
        return true;
    }
    
    debugLog(QString("Platform not found: %1").arg(platformName));
    return false;
}

/**
 * @brief 获取当前平台名称
 */
QString PlatformConnector::getCurrentPlatformName() const
{
    if (currentPlatform == PLATFORM_CUSTOM) {
        // 返回第一个自定义平台名称（简化处理）
        if (!customPlatforms.isEmpty()) {
            return customPlatforms.firstKey();
        }
        return "自定义平台";
    }
    
    if (platformConfigs.contains(currentPlatform)) {
        return platformConfigs[currentPlatform].name;
    }
    
    return "未知平台";
}

/**
 * @brief 添加自定义平台配置
 */
bool PlatformConnector::addCustomPlatform(const QString& name, const PlatformConfig& config)
{
    QMutexLocker locker(&mutex);
    
    if (name.isEmpty()) {
        debugLog("Custom platform name cannot be empty");
        return false;
    }
    
    customPlatforms[name] = config;
    debugLog(QString("Custom platform added: %1").arg(name));
    
    return true;
}

/**
 * @brief 获取平台配置
 */
PlatformConnector::PlatformConfig PlatformConnector::getPlatformConfig(PlatformType platform) const
{
    if (platformConfigs.contains(platform)) {
        return platformConfigs[platform];
    }
    return PlatformConfig();
}

/**
 * @brief 获取平台配置（通过名称）
 */
PlatformConnector::PlatformConfig PlatformConnector::getPlatformConfig(const QString& platformName) const
{
    // 查找内置平台
    for (auto it = platformConfigs.begin(); it != platformConfigs.end(); ++it) {
        if (it.value().name == platformName) {
            return it.value();
        }
    }
    
    // 查找自定义平台
    if (customPlatforms.contains(platformName)) {
        return customPlatforms[platformName];
    }
    
    return PlatformConfig();
}

/**
 * @brief 连接到游戏窗口
 */
bool PlatformConnector::connectToGame()
{
    QMutexLocker locker(&mutex);
    
    if (currentPlatform == PLATFORM_UNKNOWN) {
        emit errorOccurred("请先选择平台");
        return false;
    }
    
    connectionStatus = STATUS_CONNECTING;
    emit connectionStatusChanged(connectionStatus);
    
    debugLog("Attempting to connect to game...");
    
    // 查找游戏窗口
    if (!findGameWindow()) {
        connectionStatus = STATUS_ERROR;
        emit connectionStatusChanged(connectionStatus);
        emit errorOccurred("未找到游戏窗口");
        return false;
    }
    
    connectionStatus = STATUS_CONNECTED;
    emit connectionStatusChanged(connectionStatus);
    
    // 启动状态检查定时器
    statusTimer->start();
    
    debugLog("Successfully connected to game");
    return true;
}

/**
 * @brief 断开连接
 */
void PlatformConnector::disconnect()
{
    QMutexLocker locker(&mutex);
    
    if (connectionStatus == STATUS_DISCONNECTED) {
        return;
    }
    
    statusTimer->stop();
    
    gameWindow = nullptr;
    clickWindow = nullptr;
    
    connectionStatus = STATUS_DISCONNECTED;
    emit connectionStatusChanged(connectionStatus);
    
    debugLog("Disconnected from game");
}

/**
 * @brief 查找游戏窗口
 */
bool PlatformConnector::findGameWindow()
{
#ifdef Q_OS_WIN
    PlatformConfig config;
    
    if (currentPlatform == PLATFORM_CUSTOM) {
        if (customPlatforms.isEmpty()) {
            return false;
        }
        config = customPlatforms.first();
    } else {
        config = platformConfigs[currentPlatform];
    }
    
    // 查找主窗口
    HWND hwnd = findWindowByClass(config.windowClass, config.windowTitle);
    if (!hwnd) {
        debugLog(QString("Main window not found: %1").arg(config.windowTitle));
        return false;
    }
    
    gameWindow = hwnd;
    
    // 查找点击目标窗口
    if (config.useChildWindow && !config.clickClass.isEmpty()) {
        HWND clickHwnd = findChildWindow(hwnd, config.clickClass, config.clickTitle);
        if (clickHwnd) {
            clickWindow = clickHwnd;
        } else {
            clickWindow = hwnd; // 回退到主窗口
        }
    } else {
        clickWindow = hwnd;
    }
    
    QString windowTitle = getWindowTitle(hwnd);
    debugLog(QString("Game window found: %1").arg(windowTitle));
    emit gameWindowFound(windowTitle);
    
    return true;
#else
    // 非Windows平台暂不支持
    debugLog("Platform not supported on non-Windows systems");
    return false;
#endif
}

/**
 * @brief 获取窗口截图
 */
QPixmap PlatformConnector::captureWindow()
{
#ifdef Q_OS_WIN
    if (!gameWindow) {
        return QPixmap();
    }
    
    return captureWindow(gameWindow);
#else
    return QPixmap();
#endif
}

/**
 * @brief 获取棋盘区域截图
 */
QPixmap PlatformConnector::captureBoardArea()
{
    QPixmap fullCapture = captureWindow();
    if (fullCapture.isNull()) {
        return QPixmap();
    }
    
    PlatformConfig config;
    if (currentPlatform == PLATFORM_CUSTOM) {
        if (customPlatforms.isEmpty()) {
            return QPixmap();
        }
        config = customPlatforms.first();
    } else {
        config = platformConfigs[currentPlatform];
    }
    
    // 裁剪棋盘区域
    QRect boardRect = config.boardArea;
    if (boardRect.isValid() && fullCapture.rect().contains(boardRect)) {
        return fullCapture.copy(boardRect);
    }
    
    return fullCapture;
}

/**
 * @brief 在指定棋盘位置点击
 */
bool PlatformConnector::clickBoardPosition(int row, int col)
{
    if (!isConnected()) {
        debugLog("Not connected to game");
        return false;
    }
    
    if (row < 0 || row > 9 || col < 0 || col > 8) {
        debugLog(QString("Invalid board position: (%1, %2)").arg(row).arg(col));
        return false;
    }
    
    QPoint windowPos = boardToWindow(row, col);
    bool success = clickWindowPosition(windowPos);
    
    if (success) {
        debugLog(QString("Clicked board position: (%1, %2) -> (%3, %4)")
                .arg(row).arg(col).arg(windowPos.x()).arg(windowPos.y()));
        emit clickCompleted(row, col);
    }
    
    return success;
}

/**
 * @brief 执行走法
 */
bool PlatformConnector::makeMove(int fromRow, int fromCol, int toRow, int toCol)
{
    if (!isConnected()) {
        debugLog("Not connected to game");
        return false;
    }
    
    debugLog(QString("Making move: (%1,%2) -> (%3,%4)")
            .arg(fromRow).arg(fromCol).arg(toRow).arg(toCol));
    
    // 点击起始位置
    if (!clickBoardPosition(fromRow, fromCol)) {
        return false;
    }
    
    // 等待动画
    PlatformConfig config;
    if (currentPlatform == PLATFORM_CUSTOM) {
        if (!customPlatforms.isEmpty()) {
            config = customPlatforms.first();
        }
    } else {
        config = platformConfigs[currentPlatform];
    }
    
    QThread::msleep(config.animationDelay);
    
    // 点击目标位置
    return clickBoardPosition(toRow, toCol);
}

/**
 * @brief 在窗口坐标点击
 */
bool PlatformConnector::clickWindowPosition(const QPoint& pos)
{
#ifdef Q_OS_WIN
    if (!clickWindow) {
        return false;
    }
    
    return sendMouseClick(clickWindow, pos);
#else
    Q_UNUSED(pos)
    return false;
#endif
}

/**
 * @brief 检查窗口状态
 */
void PlatformConnector::checkWindowStatus()
{
#ifdef Q_OS_WIN
    if (!gameWindow) {
        return;
    }
    
    // 检查窗口是否仍然存在
    if (!IsWindow(gameWindow)) {
        debugLog("Game window closed");
        disconnect();
        emit errorOccurred("游戏窗口已关闭");
        return;
    }
    
    // 检查窗口是否可见
    if (!IsWindowVisible(gameWindow)) {
        debugLog("Game window is not visible");
        // 可以选择是否断开连接
    }
#endif
}

/**
 * @brief 棋盘坐标转窗口坐标
 */
QPoint PlatformConnector::boardToWindow(int row, int col) const
{
    PlatformConfig config;
    if (currentPlatform == PLATFORM_CUSTOM) {
        if (!customPlatforms.isEmpty()) {
            config = customPlatforms.first();
        }
    } else {
        config = platformConfigs[currentPlatform];
    }
    
    int x = config.boardArea.x() + config.boardOffset.x() + col * config.cellSize.width();
    int y = config.boardArea.y() + config.boardOffset.y() + row * config.cellSize.height();
    
    return QPoint(x, y);
}

/**
 * @brief 窗口坐标转棋盘坐标
 */
QPoint PlatformConnector::windowToBoard(const QPoint& pos) const
{
    PlatformConfig config;
    if (currentPlatform == PLATFORM_CUSTOM) {
        if (!customPlatforms.isEmpty()) {
            config = customPlatforms.first();
        }
    } else {
        config = platformConfigs[currentPlatform];
    }
    
    int col = (pos.x() - config.boardArea.x() - config.boardOffset.x()) / config.cellSize.width();
    int row = (pos.y() - config.boardArea.y() - config.boardOffset.y()) / config.cellSize.height();
    
    return QPoint(col, row); // 注意：返回的是(col, row)
}

/**
 * @brief 调试输出
 */
void PlatformConnector::debugLog(const QString& message) const
{
    if (debugMode) {
        qDebug() << "[PlatformConnector]" << message;
        emit const_cast<PlatformConnector*>(this)->debugMessage(message);
    }
}

/**
 * @brief 获取调试信息
 */
QString PlatformConnector::getDebugInfo() const
{
    QStringList info;
    info << QString("Current Platform: %1").arg(getCurrentPlatformName());
    info << QString("Connection Status: %1").arg(connectionStatus);
    info << QString("Game Window: %1").arg(gameWindow ? "Found" : "Not Found");
    info << QString("Click Window: %1").arg(clickWindow ? "Found" : "Not Found");
    info << QString("Debug Mode: %1").arg(debugMode ? "Enabled" : "Disabled");
    
    return info.join("\n");
}

/**
 * @brief 保存配置
 */
void PlatformConnector::saveSettings()
{
    settings->beginGroup(SETTINGS_GROUP);
    settings->setValue("currentPlatform", static_cast<int>(currentPlatform));
    settings->setValue("debugMode", debugMode);
    
    // 保存自定义平台
    settings->beginWriteArray("customPlatforms");
    int index = 0;
    for (auto it = customPlatforms.begin(); it != customPlatforms.end(); ++it, ++index) {
        settings->setArrayIndex(index);
        settings->setValue("name", it.key());
        settings->setValue("config", configToJson(it.value()).toVariantMap());
    }
    settings->endArray();
    
    settings->endGroup();
    
    debugLog("Settings saved");
}

/**
 * @brief 加载配置
 */
void PlatformConnector::loadSettings()
{
    settings->beginGroup(SETTINGS_GROUP);
    
    currentPlatform = static_cast<PlatformType>(settings->value("currentPlatform", PLATFORM_UNKNOWN).toInt());
    debugMode = settings->value("debugMode", false).toBool();
    
    // 加载自定义平台
    int size = settings->beginReadArray("customPlatforms");
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString name = settings->value("name").toString();
        QVariantMap configMap = settings->value("config").toMap();
        QJsonObject configJson = QJsonObject::fromVariantMap(configMap);
        customPlatforms[name] = configFromJson(configJson);
    }
    settings->endArray();
    
    settings->endGroup();
    
    debugLog("Settings loaded");
}

/**
 * @brief 配置转JSON
 */
QJsonObject PlatformConnector::configToJson(const PlatformConfig& config) const
{
    QJsonObject json;
    json["name"] = config.name;
    json["windowClass"] = config.windowClass;
    json["windowTitle"] = config.windowTitle;
    json["clickClass"] = config.clickClass;
    json["clickTitle"] = config.clickTitle;
    
    QJsonObject boardArea;
    boardArea["x"] = config.boardArea.x();
    boardArea["y"] = config.boardArea.y();
    boardArea["width"] = config.boardArea.width();
    boardArea["height"] = config.boardArea.height();
    json["boardArea"] = boardArea;
    
    QJsonObject boardOffset;
    boardOffset["x"] = config.boardOffset.x();
    boardOffset["y"] = config.boardOffset.y();
    json["boardOffset"] = boardOffset;
    
    QJsonObject cellSize;
    cellSize["width"] = config.cellSize.width();
    cellSize["height"] = config.cellSize.height();
    json["cellSize"] = cellSize;
    
    json["animationDelay"] = config.animationDelay;
    json["useChildWindow"] = config.useChildWindow;
    
    return json;
}

/**
 * @brief JSON转配置
 */
PlatformConnector::PlatformConfig PlatformConnector::configFromJson(const QJsonObject& json) const
{
    PlatformConfig config;
    config.name = json["name"].toString();
    config.windowClass = json["windowClass"].toString();
    config.windowTitle = json["windowTitle"].toString();
    config.clickClass = json["clickClass"].toString();
    config.clickTitle = json["clickTitle"].toString();
    
    QJsonObject boardArea = json["boardArea"].toObject();
    config.boardArea = QRect(
        boardArea["x"].toInt(),
        boardArea["y"].toInt(),
        boardArea["width"].toInt(),
        boardArea["height"].toInt()
    );
    
    QJsonObject boardOffset = json["boardOffset"].toObject();
    config.boardOffset = QPoint(
        boardOffset["x"].toInt(),
        boardOffset["y"].toInt()
    );
    
    QJsonObject cellSize = json["cellSize"].toObject();
    config.cellSize = QSize(
        cellSize["width"].toInt(),
        cellSize["height"].toInt()
    );
    
    config.animationDelay = json["animationDelay"].toInt(500);
    config.useChildWindow = json["useChildWindow"].toBool(false);
    
    return config;
}

/**
 * @brief 导出平台配置
 */
bool PlatformConnector::exportPlatformConfig(const QString& filePath) const
{
    QJsonObject root;
    QJsonArray platforms;
    
    // 导出内置平台
    for (auto it = platformConfigs.begin(); it != platformConfigs.end(); ++it) {
        if (it.key() != PLATFORM_UNKNOWN) {
            QJsonObject platform = configToJson(it.value());
            platform["type"] = "builtin";
            platform["id"] = static_cast<int>(it.key());
            platforms.append(platform);
        }
    }
    
    // 导出自定义平台
    for (auto it = customPlatforms.begin(); it != customPlatforms.end(); ++it) {
        QJsonObject platform = configToJson(it.value());
        platform["type"] = "custom";
        platforms.append(platform);
    }
    
    root["platforms"] = platforms;
    root["version"] = "1.0";
    root["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        debugLog(QString("Failed to open file for writing: %1").arg(filePath));
        return false;
    }
    
    file.write(doc.toJson());
    debugLog(QString("Platform config exported to: %1").arg(filePath));
    return true;
}

/**
 * @brief 导入平台配置
 */
bool PlatformConnector::importPlatformConfig(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        debugLog(QString("Failed to open file for reading: %1").arg(filePath));
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        debugLog(QString("JSON parse error: %1").arg(error.errorString()));
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray platforms = root["platforms"].toArray();
    
    int importedCount = 0;
    for (const QJsonValue& value : platforms) {
        QJsonObject platform = value.toObject();
        QString type = platform["type"].toString();
        
        if (type == "custom") {
            QString name = platform["name"].toString();
            if (!name.isEmpty()) {
                PlatformConfig config = configFromJson(platform);
                customPlatforms[name] = config;
                importedCount++;
            }
        }
        // 内置平台配置暂不支持导入覆盖
    }
    
    debugLog(QString("Imported %1 custom platforms from: %2").arg(importedCount).arg(filePath));
    return importedCount > 0;
}

#ifdef Q_OS_WIN
/**
 * @brief 通过类名查找窗口
 */
HWND PlatformConnector::findWindowByClass(const QString& className, const QString& title)
{
    LPCWSTR lpClassName = className.isEmpty() ? nullptr : reinterpret_cast<LPCWSTR>(className.utf16());
    LPCWSTR lpWindowName = title.isEmpty() ? nullptr : reinterpret_cast<LPCWSTR>(title.utf16());
    
    return FindWindowW(lpClassName, lpWindowName);
}

/**
 * @brief 查找子窗口
 */
HWND PlatformConnector::findChildWindow(HWND parent, const QString& className, const QString& title)
{
    LPCWSTR lpClassName = className.isEmpty() ? nullptr : reinterpret_cast<LPCWSTR>(className.utf16());
    LPCWSTR lpWindowName = title.isEmpty() ? nullptr : reinterpret_cast<LPCWSTR>(title.utf16());
    
    return FindWindowExW(parent, nullptr, lpClassName, lpWindowName);
}

/**
 * @brief 获取窗口矩形
 */
QRect PlatformConnector::getWindowRect(HWND hwnd)
{
    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
        return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    }
    return QRect();
}

/**
 * @brief 截取窗口
 */
QPixmap PlatformConnector::captureWindow(HWND hwnd)
{
    if (!hwnd) {
        return QPixmap();
    }
    
    RECT rect;
    if (!GetClientRect(hwnd, &rect)) {
        return QPixmap();
    }
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    if (width <= 0 || height <= 0) {
        return QPixmap();
    }
    
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMemDC, hbmScreen);
    
    // 使用PrintWindow进行截图
    PrintWindow(hwnd, hdcMemDC, PW_CLIENTONLY);
    
    // 转换为QPixmap (Qt 6兼容实现)
    QPixmap pixmap;
#ifdef Q_OS_WIN
    // 获取位图信息
    BITMAP bmp;
    GetObject(hbmScreen, sizeof(BITMAP), &bmp);
    
    // 创建QImage
    QImage image(bmp.bmWidth, bmp.bmHeight, QImage::Format_RGB32);
    
    // 获取位图数据
    HDC hdc = GetDC(nullptr);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = -bmp.bmHeight; // 负值表示从上到下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    GetDIBits(hdc, hbmScreen, 0, bmp.bmHeight, image.bits(), &bmi, DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);
    
    pixmap = QPixmap::fromImage(image);
#endif
    
    // 清理资源
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    
    return pixmap;
}

/**
 * @brief 发送鼠标点击
 */
bool PlatformConnector::sendMouseClick(HWND hwnd, const QPoint& pos)
{
    if (!hwnd) {
        return false;
    }
    
    // 将窗口置于前台
    SetForegroundWindow(hwnd);
    
    // 计算屏幕坐标
    POINT screenPos = {pos.x(), pos.y()};
    ClientToScreen(hwnd, &screenPos);
    
    // 设置鼠标位置
    SetCursorPos(screenPos.x, screenPos.y);
    
    // 发送鼠标点击事件
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(50); // 短暂延迟
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    
    return true;
}

/**
 * @brief 获取窗口标题
 */
QString PlatformConnector::getWindowTitle(HWND hwnd)
{
    if (!hwnd) {
        return QString();
    }
    
    wchar_t title[256];
    int length = GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
    
    if (length > 0) {
        return QString::fromWCharArray(title, length);
    }
    
    return QString();
}

/**
 * @brief 获取窗口类名
 */
QString PlatformConnector::getWindowClass(HWND hwnd)
{
    if (!hwnd) {
        return QString();
    }
    
    wchar_t className[256];
    int length = GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
    
    if (length > 0) {
        return QString::fromWCharArray(className, length);
    }
    
    return QString();
}
#endif