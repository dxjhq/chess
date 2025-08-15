#include "ConnectionDialog.h"
#include "ConnectionSchemeDialog.h"
#include <QApplication>
#include <QScreen>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

// 静态常量定义
const int ConnectionDialog::PREVIEW_UPDATE_INTERVAL = 1000; // 1秒
const int ConnectionDialog::STATUS_UPDATE_INTERVAL = 500;   // 0.5秒
const int ConnectionDialog::MAX_PREVIEW_WIDTH = 800;
const int ConnectionDialog::MAX_PREVIEW_HEIGHT = 600;

/**
 * @brief 构造函数
 */
ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
    , connector(nullptr)
    , ownConnector(true)
    , tabWidget(nullptr)
    , isCalibrating(false)
    , lastClickPos(-1, -1)
{
    // 创建连接器
    connector = new PlatformConnector(this);
    
    // 创建定时器
    previewTimer = new QTimer(this);
    previewTimer->setInterval(PREVIEW_UPDATE_INTERVAL);
    connect(previewTimer, &QTimer::timeout, this, &ConnectionDialog::onPreviewTimer);
    
    statusTimer = new QTimer(this);
    statusTimer->setInterval(STATUS_UPDATE_INTERVAL);
    connect(statusTimer, &QTimer::timeout, this, &ConnectionDialog::onStatusTimer);
    
    // 设置窗口属性
    setWindowTitle("平台连线配置");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(900, 700);
    
    // 初始化界面
    setupUI();
    
    // 连接信号
    connect(connector, &PlatformConnector::connectionStatusChanged,
            this, &ConnectionDialog::onConnectionStatusChanged);
    connect(connector, &PlatformConnector::platformChanged,
            this, &ConnectionDialog::onPlatformChangedFromConnector);
    connect(connector, &PlatformConnector::errorOccurred,
            this, &ConnectionDialog::onErrorOccurred);
    connect(connector, &PlatformConnector::debugMessage,
            this, &ConnectionDialog::onDebugMessage);
    connect(connector, &PlatformConnector::gameWindowFound,
            this, &ConnectionDialog::onGameWindowFound);
    connect(connector, &PlatformConnector::clickCompleted,
            this, &ConnectionDialog::onClickCompleted);
    
    // 注意：不在构造函数中调用updateUI()，避免窗口枚举导致卡死
    // updateUI()将在showEvent()中调用
}

/**
 * @brief 析构函数
 */
ConnectionDialog::~ConnectionDialog()
{
    if (ownConnector && connector) {
        delete connector;
    }
}

/**
 * @brief 设置外部连接器
 */
void ConnectionDialog::setPlatformConnector(PlatformConnector* platformConnector)
{
    if (ownConnector && connector) {
        delete connector;
    }
    
    connector = platformConnector;
    ownConnector = false;
    
    if (connector) {
        // 重新连接信号
        connect(connector, &PlatformConnector::connectionStatusChanged,
                this, &ConnectionDialog::onConnectionStatusChanged);
        connect(connector, &PlatformConnector::platformChanged,
                this, &ConnectionDialog::onPlatformChangedFromConnector);
        connect(connector, &PlatformConnector::errorOccurred,
                this, &ConnectionDialog::onErrorOccurred);
        connect(connector, &PlatformConnector::debugMessage,
                this, &ConnectionDialog::onDebugMessage);
        connect(connector, &PlatformConnector::gameWindowFound,
                this, &ConnectionDialog::onGameWindowFound);
        connect(connector, &PlatformConnector::clickCompleted,
                this, &ConnectionDialog::onClickCompleted);
        
        updateUI();
    }
}

/**
 * @brief 设置界面
 */
void ConnectionDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建标签页
    tabWidget = new QTabWidget;
    
    setupPlatformTab();
    setupConnectionTab();
    setupCalibrationTab();
    setupDebugTab();
    
    // 连接标签页切换事件，实现按需更新窗口列表
    connect(tabWidget, &QTabWidget::currentChanged, this, &ConnectionDialog::onTabChanged);
    
    mainLayout->addWidget(tabWidget);
    
    // 底部按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");
    QPushButton* applyBtn = new QPushButton("应用");
    
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyBtn, &QPushButton::clicked, this, &ConnectionDialog::syncUIToConfig);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(applyBtn);
    
    mainLayout->addLayout(buttonLayout);
}

/**
 * @brief 设置平台配置标签页
 */
void ConnectionDialog::setupPlatformTab()
{
    platformTab = new QWidget;
    
    QHBoxLayout* mainLayout = new QHBoxLayout(platformTab);
    
    // 左侧配置区域
    QWidget* configWidget = createPlatformConfigWidget();
    mainLayout->addWidget(configWidget, 1);
    
    // 右侧预览区域
    QWidget* previewWidget = createPreviewWidget();
    mainLayout->addWidget(previewWidget, 1);
    
    tabWidget->addTab(platformTab, "平台配置");
}

/**
 * @brief 设置连接配置标签页
 */
void ConnectionDialog::setupConnectionTab()
{
    connectionTab = new QWidget;
    
    QHBoxLayout* mainLayout = new QHBoxLayout(connectionTab);
    
    // 左侧窗口选择
    QWidget* windowWidget = createWindowSelectionWidget();
    mainLayout->addWidget(windowWidget, 1);
    
    // 右侧连接状态
    QGroupBox* statusGroup = new QGroupBox("连接状态");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    connectionStatusLabel = new QLabel("未连接");
    connectionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    statusLayout->addWidget(connectionStatusLabel);
    
    connectionProgress = new QProgressBar;
    connectionProgress->setVisible(false);
    statusLayout->addWidget(connectionProgress);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    connectBtn = new QPushButton("连接");
    disconnectBtn = new QPushButton("断开");
    testConnectionBtn = new QPushButton("测试连接");
    
    connect(connectBtn, &QPushButton::clicked, this, &ConnectionDialog::onConnectToGame);
    connect(disconnectBtn, &QPushButton::clicked, this, &ConnectionDialog::onDisconnectFromGame);
    connect(testConnectionBtn, &QPushButton::clicked, this, &ConnectionDialog::onTestConnection);
    
    buttonLayout->addWidget(connectBtn);
    buttonLayout->addWidget(disconnectBtn);
    buttonLayout->addWidget(testConnectionBtn);
    statusLayout->addLayout(buttonLayout);
    
    statusLayout->addStretch();
    
    mainLayout->addWidget(statusGroup, 1);
    
    tabWidget->addTab(connectionTab, "连接管理");
}

/**
 * @brief 设置坐标校准标签页
 */
void ConnectionDialog::setupCalibrationTab()
{
    calibrationTab = new QWidget;
    
    QHBoxLayout* mainLayout = new QHBoxLayout(calibrationTab);
    
    // 左侧校准控件
    QWidget* calibrationWidget = createCalibrationWidget();
    mainLayout->addWidget(calibrationWidget, 1);
    
    // 右侧预览（共享）
    mainLayout->addWidget(createPreviewWidget(), 1);
    
    tabWidget->addTab(calibrationTab, "坐标校准");
}

/**
 * @brief 设置调试标签页
 */
void ConnectionDialog::setupDebugTab()
{
    debugTab = new QWidget;
    
    QVBoxLayout* mainLayout = new QVBoxLayout(debugTab);
    
    // 调试控制
    QHBoxLayout* controlLayout = new QHBoxLayout;
    
    debugModeCheck = new QCheckBox("启用调试模式");
    connect(debugModeCheck, &QCheckBox::toggled, [this](bool enabled) {
        if (connector) {
            connector->setDebugMode(enabled);
        }
    });
    
    clearDebugBtn = new QPushButton("清空日志");
    connect(clearDebugBtn, &QPushButton::clicked, [this]() {
        debugOutput->clear();
    });
    
    saveDebugBtn = new QPushButton("保存日志");
    connect(saveDebugBtn, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "保存调试日志", 
            QString("debug_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
            "文本文件 (*.txt)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << debugOutput->toPlainText();
                showMessage("调试日志已保存");
            }
        }
    });
    
    controlLayout->addWidget(debugModeCheck);
    controlLayout->addStretch();
    controlLayout->addWidget(clearDebugBtn);
    controlLayout->addWidget(saveDebugBtn);
    
    mainLayout->addLayout(controlLayout);
    
    // 调试输出
    debugOutput = new QTextEdit;
    debugOutput->setReadOnly(true);
    debugOutput->setFont(QFont("Consolas", 9));
    mainLayout->addWidget(debugOutput);
    
    // 调试信息
    debugInfoLabel = new QLabel;
    debugInfoLabel->setWordWrap(true);
    debugInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
    mainLayout->addWidget(debugInfoLabel);
    
    tabWidget->addTab(debugTab, "调试信息");
}

/**
 * @brief 创建平台配置控件
 */
QWidget* ConnectionDialog::createPlatformConfigWidget()
{
    QGroupBox* configGroup = new QGroupBox("平台配置");
    QVBoxLayout* mainLayout = new QVBoxLayout(configGroup);
    
    // 连线方案选择
    QHBoxLayout* schemeSelectLayout = new QHBoxLayout;
    schemeSelectLayout->addWidget(new QLabel("连线方案:"));
    
    connectionSchemeCombo = new QComboBox;
    connect(connectionSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionDialog::onConnectionSchemeSelected);
    schemeSelectLayout->addWidget(connectionSchemeCombo);
    
    refreshSchemesBtn = new QPushButton("刷新");
    connect(refreshSchemesBtn, &QPushButton::clicked, this, &ConnectionDialog::onRefreshConnectionSchemes);
    schemeSelectLayout->addWidget(refreshSchemesBtn);
    
    schemeSelectLayout->addStretch();
    mainLayout->addLayout(schemeSelectLayout);
    
    // 平台选择
    QHBoxLayout* platformLayout = new QHBoxLayout;
    platformLayout->addWidget(new QLabel("平台:"));
    
    platformCombo = new QComboBox;
    connect(platformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionDialog::onPlatformChanged);
    platformLayout->addWidget(platformCombo);
    
    addPlatformBtn = new QPushButton("添加");
    editPlatformBtn = new QPushButton("编辑");
    deletePlatformBtn = new QPushButton("删除");
    
    connect(addPlatformBtn, &QPushButton::clicked, this, &ConnectionDialog::onAddCustomPlatform);
    connect(editPlatformBtn, &QPushButton::clicked, this, &ConnectionDialog::onEditCustomPlatform);
    connect(deletePlatformBtn, &QPushButton::clicked, this, &ConnectionDialog::onDeleteCustomPlatform);
    
    platformLayout->addWidget(addPlatformBtn);
    platformLayout->addWidget(editPlatformBtn);
    platformLayout->addWidget(deletePlatformBtn);
    
    mainLayout->addLayout(platformLayout);
    
    // 配置表单
    QFormLayout* formLayout = new QFormLayout;
    
    platformNameEdit = new QLineEdit;
    platformNameEdit->setReadOnly(true);
    formLayout->addRow("名称:", platformNameEdit);
    
    windowClassEdit = new QLineEdit;
    connect(windowClassEdit, &QLineEdit::textChanged, this, &ConnectionDialog::updatePlatformConfig);
    formLayout->addRow("窗口类名:", windowClassEdit);
    
    windowTitleEdit = new QLineEdit;
    connect(windowTitleEdit, &QLineEdit::textChanged, this, &ConnectionDialog::updatePlatformConfig);
    formLayout->addRow("窗口标题:", windowTitleEdit);
    
    clickClassEdit = new QLineEdit;
    connect(clickClassEdit, &QLineEdit::textChanged, this, &ConnectionDialog::updatePlatformConfig);
    formLayout->addRow("点击类名:", clickClassEdit);
    
    clickTitleEdit = new QLineEdit;
    connect(clickTitleEdit, &QLineEdit::textChanged, this, &ConnectionDialog::updatePlatformConfig);
    formLayout->addRow("点击标题:", clickTitleEdit);
    
    useChildWindowCheck = new QCheckBox("使用子窗口");
    connect(useChildWindowCheck, &QCheckBox::toggled, this, &ConnectionDialog::updatePlatformConfig);
    formLayout->addRow("", useChildWindowCheck);
    
    mainLayout->addLayout(formLayout);
    
    // 连线方案管理按钮
    QHBoxLayout* schemeLayout = new QHBoxLayout;
    
    manageSchemesBtn = new QPushButton("管理连线方案");
    applySchemesBtn = new QPushButton("应用方案");
    
    connect(manageSchemesBtn, &QPushButton::clicked, this, &ConnectionDialog::onManageConnectionSchemes);
    connect(applySchemesBtn, &QPushButton::clicked, this, &ConnectionDialog::onApplyConnectionScheme);
    
    schemeLayout->addWidget(manageSchemesBtn);
    schemeLayout->addWidget(applySchemesBtn);
    schemeLayout->addStretch();
    
    mainLayout->addLayout(schemeLayout);
    
    // 导入导出按钮
    QHBoxLayout* ioLayout = new QHBoxLayout;
    
    importBtn = new QPushButton("导入配置");
    exportBtn = new QPushButton("导出配置");
    
    connect(importBtn, &QPushButton::clicked, this, &ConnectionDialog::onImportPlatforms);
    connect(exportBtn, &QPushButton::clicked, this, &ConnectionDialog::onExportPlatforms);
    
    ioLayout->addWidget(importBtn);
    ioLayout->addWidget(exportBtn);
    ioLayout->addStretch();
    
    mainLayout->addLayout(ioLayout);
    
    return configGroup;
}

/**
 * @brief 创建窗口选择控件
 */
QWidget* ConnectionDialog::createWindowSelectionWidget()
{
    QGroupBox* windowGroup = new QGroupBox("窗口选择");
    QVBoxLayout* mainLayout = new QVBoxLayout(windowGroup);
    
    // 刷新按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    refreshWindowsBtn = new QPushButton("刷新窗口列表");
    connect(refreshWindowsBtn, &QPushButton::clicked, this, &ConnectionDialog::onRefreshWindows);
    
    buttonLayout->addWidget(refreshWindowsBtn);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // 窗口列表
    windowList = new QListWidget;
    connect(windowList, &QListWidget::itemSelectionChanged, this, &ConnectionDialog::onWindowSelected);
    mainLayout->addWidget(windowList);
    
    return windowGroup;
}

/**
 * @brief 创建坐标校准控件
 */
QWidget* ConnectionDialog::createCalibrationWidget()
{
    QGroupBox* calibrationGroup = new QGroupBox("坐标校准");
    QVBoxLayout* mainLayout = new QVBoxLayout(calibrationGroup);
    
    // 棋盘区域
    QGroupBox* boardGroup = new QGroupBox("棋盘区域");
    QFormLayout* boardLayout = new QFormLayout(boardGroup);
    
    boardXSpin = new QSpinBox;
    boardXSpin->setRange(0, 2000);
    connect(boardXSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onBoardAreaChanged);
    boardLayout->addRow("X坐标:", boardXSpin);
    
    boardYSpin = new QSpinBox;
    boardYSpin->setRange(0, 2000);
    connect(boardYSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onBoardAreaChanged);
    boardLayout->addRow("Y坐标:", boardYSpin);
    
    boardWidthSpin = new QSpinBox;
    boardWidthSpin->setRange(100, 1000);
    connect(boardWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onBoardAreaChanged);
    boardLayout->addRow("宽度:", boardWidthSpin);
    
    boardHeightSpin = new QSpinBox;
    boardHeightSpin->setRange(100, 1000);
    connect(boardHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onBoardAreaChanged);
    boardLayout->addRow("高度:", boardHeightSpin);
    
    mainLayout->addWidget(boardGroup);
    
    // 偏移和格子大小
    QGroupBox* offsetGroup = new QGroupBox("偏移和格子");
    QFormLayout* offsetLayout = new QFormLayout(offsetGroup);
    
    offsetXSpin = new QSpinBox;
    offsetXSpin->setRange(0, 100);
    connect(offsetXSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onOffsetChanged);
    offsetLayout->addRow("X偏移:", offsetXSpin);
    
    offsetYSpin = new QSpinBox;
    offsetYSpin->setRange(0, 100);
    connect(offsetYSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onOffsetChanged);
    offsetLayout->addRow("Y偏移:", offsetYSpin);
    
    cellWidthSpin = new QSpinBox;
    cellWidthSpin->setRange(10, 100);
    connect(cellWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onCellSizeChanged);
    offsetLayout->addRow("格子宽度:", cellWidthSpin);
    
    cellHeightSpin = new QSpinBox;
    cellHeightSpin->setRange(10, 100);
    connect(cellHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionDialog::onCellSizeChanged);
    offsetLayout->addRow("格子高度:", cellHeightSpin);
    
    animationDelaySpin = new QSpinBox;
    animationDelaySpin->setRange(0, 2000);
    animationDelaySpin->setSuffix(" ms");
    offsetLayout->addRow("动画延迟:", animationDelaySpin);
    
    mainLayout->addWidget(offsetGroup);
    
    // 测试区域
    QGroupBox* testGroup = new QGroupBox("点击测试");
    QVBoxLayout* testLayout = new QVBoxLayout(testGroup);
    
    QHBoxLayout* testPosLayout = new QHBoxLayout;
    testPosLayout->addWidget(new QLabel("测试位置:"));
    
    testRowSpin = new QSpinBox;
    testRowSpin->setRange(0, 9);
    testRowSpin->setValue(4);
    testPosLayout->addWidget(new QLabel("行:"));
    testPosLayout->addWidget(testRowSpin);
    
    testColSpin = new QSpinBox;
    testColSpin->setRange(0, 8);
    testColSpin->setValue(4);
    testPosLayout->addWidget(new QLabel("列:"));
    testPosLayout->addWidget(testColSpin);
    
    testPosLayout->addStretch();
    testLayout->addLayout(testPosLayout);
    
    QHBoxLayout* testBtnLayout = new QHBoxLayout;
    
    calibrateBoardBtn = new QPushButton("自动校准");
    connect(calibrateBoardBtn, &QPushButton::clicked, this, &ConnectionDialog::onCalibrateBoard);
    testBtnLayout->addWidget(calibrateBoardBtn);
    
    testClickBtn = new QPushButton("测试点击");
    connect(testClickBtn, &QPushButton::clicked, this, &ConnectionDialog::onTestClick);
    testBtnLayout->addWidget(testClickBtn);
    
    testLayout->addLayout(testBtnLayout);
    
    mainLayout->addWidget(testGroup);
    
    mainLayout->addStretch();
    
    return calibrationGroup;
}

/**
 * @brief 创建预览控件
 */
QWidget* ConnectionDialog::createPreviewWidget()
{
    QGroupBox* previewGroup = new QGroupBox("预览");
    QVBoxLayout* mainLayout = new QVBoxLayout(previewGroup);
    
    // 控制按钮
    QHBoxLayout* controlLayout = new QHBoxLayout;
    
    captureWindowBtn = new QPushButton("截取窗口");
    connect(captureWindowBtn, &QPushButton::clicked, this, &ConnectionDialog::onCaptureWindow);
    controlLayout->addWidget(captureWindowBtn);
    
    captureBoardBtn = new QPushButton("截取棋盘");
    connect(captureBoardBtn, &QPushButton::clicked, this, &ConnectionDialog::onCaptureBoardArea);
    controlLayout->addWidget(captureBoardBtn);
    
    controlLayout->addStretch();
    
    showGridCheck = new QCheckBox("显示网格");
    showGridCheck->setChecked(true);
    connect(showGridCheck, &QCheckBox::toggled, this, &ConnectionDialog::updatePreview);
    controlLayout->addWidget(showGridCheck);
    
    autoUpdateCheck = new QCheckBox("自动更新");
    connect(autoUpdateCheck, &QCheckBox::toggled, [this](bool enabled) {
        if (enabled) {
            previewTimer->start();
        } else {
            previewTimer->stop();
        }
    });
    controlLayout->addWidget(autoUpdateCheck);
    
    mainLayout->addLayout(controlLayout);
    
    // 缩放控制
    QHBoxLayout* scaleLayout = new QHBoxLayout;
    scaleLayout->addWidget(new QLabel("缩放:"));
    
    previewScaleSlider = new QSlider(Qt::Horizontal);
    previewScaleSlider->setRange(25, 200);
    previewScaleSlider->setValue(100);
    connect(previewScaleSlider, &QSlider::valueChanged, this, &ConnectionDialog::updatePreview);
    scaleLayout->addWidget(previewScaleSlider);
    
    QLabel* scaleLabel = new QLabel("100%");
    connect(previewScaleSlider, &QSlider::valueChanged, [scaleLabel](int value) {
        scaleLabel->setText(QString("%1%").arg(value));
    });
    scaleLayout->addWidget(scaleLabel);
    
    mainLayout->addLayout(scaleLayout);
    
    // 预览区域
    previewArea = new QScrollArea;
    previewArea->setWidgetResizable(true);
    previewArea->setAlignment(Qt::AlignCenter);
    
    previewLabel = new QLabel;
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid gray; }");
    previewLabel->setText("暂无预览图像");
    previewLabel->setMinimumSize(200, 150);
    
    previewArea->setWidget(previewLabel);
    mainLayout->addWidget(previewArea);
    
    return previewGroup;
}

/**
 * @brief 平台改变事件
 */
void ConnectionDialog::onPlatformChanged()
{
    if (!connector) return;
    
    QString platformName = platformCombo->currentText();
    if (!platformName.isEmpty()) {
        connector->setPlatform(platformName);
        syncConfigToUI();
    }
}

/**
 * @brief 添加自定义平台
 */
void ConnectionDialog::onAddCustomPlatform()
{
    CustomPlatformDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getPlatformName();
        PlatformConnector::PlatformConfig config = dialog.getPlatformConfig();
        
        if (connector && connector->addCustomPlatform(name, config)) {
            // 更新平台列表
            updateUI();
            
            // 选择新添加的平台
            int index = platformCombo->findText(name);
            if (index >= 0) {
                platformCombo->setCurrentIndex(index);
            }
            
            showMessage(QString("自定义平台 '%1' 添加成功").arg(name));
        }
    }
}

/**
 * @brief 编辑自定义平台
 */
void ConnectionDialog::onEditCustomPlatform()
{
    QString currentPlatform = platformCombo->currentText();
    if (currentPlatform.isEmpty()) {
        showMessage("请先选择要编辑的平台", true);
        return;
    }
    
    // 检查是否为自定义平台
    if (!connector) return;
    
    PlatformConnector::PlatformConfig config = connector->getPlatformConfig(currentPlatform);
    if (config.name.isEmpty()) {
        showMessage("无法编辑内置平台", true);
        return;
    }
    
    CustomPlatformDialog dialog(this);
    dialog.setPlatformConfig(currentPlatform, config);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString newName = dialog.getPlatformName();
        PlatformConnector::PlatformConfig newConfig = dialog.getPlatformConfig();
        
        if (connector->addCustomPlatform(newName, newConfig)) {
            updateUI();
            
            // 选择编辑后的平台
            int index = platformCombo->findText(newName);
            if (index >= 0) {
                platformCombo->setCurrentIndex(index);
            }
            
            showMessage(QString("平台 '%1' 编辑成功").arg(newName));
        }
    }
}

/**
 * @brief 删除自定义平台
 */
void ConnectionDialog::onDeleteCustomPlatform()
{
    QString currentPlatform = platformCombo->currentText();
    if (currentPlatform.isEmpty()) {
        showMessage("请先选择要删除的平台", true);
        return;
    }
    
    // 确认删除
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除平台 '%1' 吗？").arg(currentPlatform),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // TODO: 实现删除自定义平台功能
        showMessage("删除功能待实现");
    }
}

/**
 * @brief 导入平台配置
 */
void ConnectionDialog::onImportPlatforms()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入平台配置", "", 
        "JSON文件 (*.json);;所有文件 (*.*)");
    
    if (!fileName.isEmpty() && connector) {
        if (connector->importPlatformConfig(fileName)) {
            updateUI();
            showMessage("平台配置导入成功");
        } else {
            showMessage("平台配置导入失败", true);
        }
    }
}

/**
 * @brief 导出平台配置
 */
void ConnectionDialog::onExportPlatforms()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出平台配置", 
        QString("platforms_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON文件 (*.json);;所有文件 (*.*)");
    
    if (!fileName.isEmpty() && connector) {
        if (connector->exportPlatformConfig(fileName)) {
            showMessage("平台配置导出成功");
        } else {
            showMessage("平台配置导出失败", true);
        }
    }
}

/**
 * @brief 连接到游戏
 */
void ConnectionDialog::onConnectToGame()
{
    if (!connector) return;
    
    syncUIToConfig(); // 同步配置
    
    connectionProgress->setVisible(true);
    connectionProgress->setRange(0, 0); // 不确定进度
    
    if (connector->connectToGame()) {
        showMessage("连接成功");
    } else {
        showMessage("连接失败", true);
    }
    
    connectionProgress->setVisible(false);
}

/**
 * @brief 断开连接
 */
void ConnectionDialog::onDisconnectFromGame()
{
    if (connector) {
        connector->disconnect();
        showMessage("已断开连接");
    }
}

/**
 * @brief 刷新窗口列表
 */
void ConnectionDialog::onRefreshWindows()
{
    updateWindowList();
}

/**
 * @brief 测试连接
 */
void ConnectionDialog::onTestConnection()
{
    if (!connector || !connector->isConnected()) {
        showMessage("请先连接到游戏", true);
        return;
    }
    
    // 测试截图功能
    QPixmap capture = connector->captureWindow();
    if (!capture.isNull()) {
        currentPreview = capture;
        updatePreview();
        showMessage("连接测试成功 - 截图正常");
    } else {
        showMessage("连接测试失败 - 无法截图", true);
    }
}

/**
 * @brief 截取窗口
 */
void ConnectionDialog::onCaptureWindow()
{
    if (!connector) return;
    
    QPixmap capture = connector->captureWindow();
    if (!capture.isNull()) {
        currentPreview = capture;
        updatePreview();
        showMessage("窗口截图成功");
    } else {
        showMessage("窗口截图失败", true);
    }
}

/**
 * @brief 截取棋盘区域
 */
void ConnectionDialog::onCaptureBoardArea()
{
    if (!connector) return;
    
    QPixmap capture = connector->captureBoardArea();
    if (!capture.isNull()) {
        currentPreview = capture;
        updatePreview();
        showMessage("棋盘截图成功");
    } else {
        showMessage("棋盘截图失败", true);
    }
}

/**
 * @brief 窗口选择改变
 */
void ConnectionDialog::onWindowSelected()
{
    QListWidgetItem* item = windowList->currentItem();
    if (item) {
        QString windowInfo = item->text();
        logMessage(QString("选择窗口: %1").arg(windowInfo));
    }
}

/**
 * @brief 校准棋盘
 */
void ConnectionDialog::onCalibrateBoard()
{
    showMessage("自动校准功能待实现");
}

/**
 * @brief 测试点击
 */
void ConnectionDialog::onTestClick()
{
    if (!connector || !connector->isConnected()) {
        showMessage("请先连接到游戏", true);
        return;
    }
    
    int row = testRowSpin->value();
    int col = testColSpin->value();
    
    if (connector->clickBoardPosition(row, col)) {
        showMessage(QString("测试点击成功: (%1, %2)").arg(row).arg(col));
        
        // 在预览图上标记点击位置
        lastClickPos = QPoint(col, row);
        updatePreview();
    } else {
        showMessage("测试点击失败", true);
    }
}

/**
 * @brief 棋盘区域改变
 */
void ConnectionDialog::onBoardAreaChanged()
{
    updatePlatformConfig();
    updatePreview();
}

/**
 * @brief 格子大小改变
 */
void ConnectionDialog::onCellSizeChanged()
{
    updatePlatformConfig();
    updatePreview();
}

/**
 * @brief 偏移改变
 */
void ConnectionDialog::onOffsetChanged()
{
    updatePlatformConfig();
    updatePreview();
}

/**
 * @brief 连接状态改变
 */
void ConnectionDialog::onConnectionStatusChanged(PlatformConnector::ConnectionStatus status)
{
    updateConnectionStatus();
}

/**
 * @brief 平台改变（来自连接器）
 */
void ConnectionDialog::onPlatformChangedFromConnector(PlatformConnector::PlatformType platform)
{
    Q_UNUSED(platform)
    updateUI();
}

/**
 * @brief 错误发生
 */
void ConnectionDialog::onErrorOccurred(const QString& error)
{
    showMessage(error, true);
    logMessage(QString("错误: %1").arg(error));
}

/**
 * @brief 调试消息
 */
void ConnectionDialog::onDebugMessage(const QString& message)
{
    logMessage(message);
}

/**
 * @brief 游戏窗口找到
 */
void ConnectionDialog::onGameWindowFound(const QString& windowTitle)
{
    showMessage(QString("找到游戏窗口: %1").arg(windowTitle));
}

/**
 * @brief 点击完成
 */
void ConnectionDialog::onClickCompleted(int row, int col)
{
    logMessage(QString("点击完成: (%1, %2)").arg(row).arg(col));
    
    // 更新最后点击位置
    lastClickPos = QPoint(col, row);
    updatePreview();
}

/**
 * @brief 更新界面
 */
void ConnectionDialog::updateUI()
{
    if (!connector) return;
    
    // 更新平台列表
    platformCombo->clear();
    QStringList platforms = connector->getSupportedPlatforms();
    platformCombo->addItems(platforms);
    
    // 选择当前平台
    QString currentPlatform = connector->getCurrentPlatformName();
    int index = platformCombo->findText(currentPlatform);
    if (index >= 0) {
        platformCombo->setCurrentIndex(index);
    }
    
    // 刷新连线方案列表
    onRefreshConnectionSchemes();
    
    // 同步配置到界面
    syncConfigToUI();
    
    // 更新连接状态
    updateConnectionStatus();
    
    // 注意：不在初始化时更新窗口列表，避免卡死
    // 窗口列表将在用户切换到连接标签页时按需更新
    
    // 更新调试信息
    if (debugInfoLabel) {
        debugInfoLabel->setText(connector->getDebugInfo());
    }
}

/**
 * @brief 更新连接状态
 */
void ConnectionDialog::updateConnectionStatus()
{
    if (!connector) return;
    
    PlatformConnector::ConnectionStatus status = connector->getConnectionStatus();
    
    QString statusText;
    QString styleSheet;
    
    switch (status) {
    case PlatformConnector::STATUS_DISCONNECTED:
        statusText = "未连接";
        styleSheet = "QLabel { color: red; font-weight: bold; }";
        connectBtn->setEnabled(true);
        disconnectBtn->setEnabled(false);
        testConnectionBtn->setEnabled(false);
        break;
        
    case PlatformConnector::STATUS_CONNECTING:
        statusText = "连接中...";
        styleSheet = "QLabel { color: orange; font-weight: bold; }";
        connectBtn->setEnabled(false);
        disconnectBtn->setEnabled(true);
        testConnectionBtn->setEnabled(false);
        break;
        
    case PlatformConnector::STATUS_CONNECTED:
        statusText = "已连接";
        styleSheet = "QLabel { color: green; font-weight: bold; }";
        connectBtn->setEnabled(false);
        disconnectBtn->setEnabled(true);
        testConnectionBtn->setEnabled(true);
        break;
        
    case PlatformConnector::STATUS_ERROR:
        statusText = "连接错误";
        styleSheet = "QLabel { color: red; font-weight: bold; }";
        connectBtn->setEnabled(true);
        disconnectBtn->setEnabled(false);
        testConnectionBtn->setEnabled(false);
        break;
    }
    
    if (connectionStatusLabel) {
        connectionStatusLabel->setText(statusText);
        connectionStatusLabel->setStyleSheet(styleSheet);
    }
}

/**
 * @brief 更新平台配置
 */
void ConnectionDialog::updatePlatformConfig()
{
    // 这个方法在配置改变时被调用，可以用来实时更新预览等
    updatePreview();
}

/**
 * @brief 更新窗口列表
 */
void ConnectionDialog::updateWindowList()
{
    if (!windowList) return;
    
    windowList->clear();
    
#ifdef Q_OS_WIN
    // 枚举所有顶级窗口
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        ConnectionDialog* dialog = reinterpret_cast<ConnectionDialog*>(lParam);
        
        if (IsWindowVisible(hwnd)) {
            QString info = dialog->getWindowInfo(hwnd);
            if (!info.isEmpty()) {
                QListWidgetItem* item = new QListWidgetItem(info);
                item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(hwnd)));
                dialog->windowList->addItem(item);
            }
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(this));
#endif
    
    logMessage(QString("找到 %1 个窗口").arg(windowList->count()));
}

/**
 * @brief 更新预览
 */
void ConnectionDialog::updatePreview()
{
    if (!previewLabel || currentPreview.isNull()) {
        return;
    }
    
    QPixmap preview = currentPreview;
    
    // 绘制网格
    if (showGridCheck && showGridCheck->isChecked()) {
        drawBoardGrid(preview);
    }
    
    // 绘制点击位置
    if (lastClickPos.x() >= 0 && lastClickPos.y() >= 0) {
        drawClickPosition(preview, lastClickPos.y(), lastClickPos.x());
    }
    
    // 缩放
    if (previewScaleSlider) {
        int scale = previewScaleSlider->value();
        if (scale != 100) {
            QSize newSize = preview.size() * scale / 100;
            preview = preview.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    
    previewLabel->setPixmap(preview);
    previewLabel->resize(preview.size());
}

/**
 * @brief 预览定时器
 */
void ConnectionDialog::onPreviewTimer()
{
    if (connector && connector->isConnected()) {
        QPixmap capture = connector->captureWindow();
        if (!capture.isNull()) {
            currentPreview = capture;
            updatePreview();
        }
    }
}

/**
 * @brief 状态定时器
 */
void ConnectionDialog::onStatusTimer()
{
    if (debugInfoLabel && connector) {
        debugInfoLabel->setText(connector->getDebugInfo());
    }
}

/**
 * @brief 标签页切换事件处理
 * @param index 标签页索引
 */
void ConnectionDialog::onTabChanged(int index)
{
    // 当切换到连接标签页时，才更新窗口列表
    if (index == 1) { // 连接标签页的索引是1
        updateWindowList();
    }
}

/**
 * @brief 同步配置到界面
 */
void ConnectionDialog::syncConfigToUI()
{
    if (!connector) return;
    
    PlatformConnector::PlatformConfig config = connector->getPlatformConfig(connector->getCurrentPlatformName());
    
    if (platformNameEdit) platformNameEdit->setText(config.name);
    if (windowClassEdit) windowClassEdit->setText(config.windowClass);
    if (windowTitleEdit) windowTitleEdit->setText(config.windowTitle);
    if (clickClassEdit) clickClassEdit->setText(config.clickClass);
    if (clickTitleEdit) clickTitleEdit->setText(config.clickTitle);
    if (useChildWindowCheck) useChildWindowCheck->setChecked(config.useChildWindow);
    
    if (boardXSpin) boardXSpin->setValue(config.boardArea.x());
    if (boardYSpin) boardYSpin->setValue(config.boardArea.y());
    if (boardWidthSpin) boardWidthSpin->setValue(config.boardArea.width());
    if (boardHeightSpin) boardHeightSpin->setValue(config.boardArea.height());
    
    if (offsetXSpin) offsetXSpin->setValue(config.boardOffset.x());
    if (offsetYSpin) offsetYSpin->setValue(config.boardOffset.y());
    
    if (cellWidthSpin) cellWidthSpin->setValue(config.cellSize.width());
    if (cellHeightSpin) cellHeightSpin->setValue(config.cellSize.height());
    
    if (animationDelaySpin) animationDelaySpin->setValue(config.animationDelay);
}

/**
 * @brief 同步界面到配置
 */
void ConnectionDialog::syncUIToConfig()
{
    if (!connector) return;
    
    PlatformConnector::PlatformConfig config = getCurrentConfig();
    
    // 对于自定义平台，更新配置
    if (connector->getCurrentPlatform() == PlatformConnector::PLATFORM_CUSTOM) {
        QString platformName = connector->getCurrentPlatformName();
        connector->addCustomPlatform(platformName, config);
    }
}

/**
 * @brief 获取当前配置
 */
PlatformConnector::PlatformConfig ConnectionDialog::getCurrentConfig() const
{
    PlatformConnector::PlatformConfig config;
    
    if (platformNameEdit) config.name = platformNameEdit->text();
    if (windowClassEdit) config.windowClass = windowClassEdit->text();
    if (windowTitleEdit) config.windowTitle = windowTitleEdit->text();
    if (clickClassEdit) config.clickClass = clickClassEdit->text();
    if (clickTitleEdit) config.clickTitle = clickTitleEdit->text();
    if (useChildWindowCheck) config.useChildWindow = useChildWindowCheck->isChecked();
    
    if (boardXSpin && boardYSpin && boardWidthSpin && boardHeightSpin) {
        config.boardArea = QRect(boardXSpin->value(), boardYSpin->value(), 
                                boardWidthSpin->value(), boardHeightSpin->value());
    }
    
    if (offsetXSpin && offsetYSpin) {
        config.boardOffset = QPoint(offsetXSpin->value(), offsetYSpin->value());
    }
    
    if (cellWidthSpin && cellHeightSpin) {
        config.cellSize = QSize(cellWidthSpin->value(), cellHeightSpin->value());
    }
    
    if (animationDelaySpin) {
        config.animationDelay = animationDelaySpin->value();
    }
    
    return config;
}

/**
 * @brief 设置当前配置
 */
void ConnectionDialog::setCurrentConfig(const PlatformConnector::PlatformConfig& config)
{
    if (platformNameEdit) platformNameEdit->setText(config.name);
    if (windowClassEdit) windowClassEdit->setText(config.windowClass);
    if (windowTitleEdit) windowTitleEdit->setText(config.windowTitle);
    if (clickClassEdit) clickClassEdit->setText(config.clickClass);
    if (clickTitleEdit) clickTitleEdit->setText(config.clickTitle);
    if (useChildWindowCheck) useChildWindowCheck->setChecked(config.useChildWindow);
    
    if (boardXSpin) boardXSpin->setValue(config.boardArea.x());
    if (boardYSpin) boardYSpin->setValue(config.boardArea.y());
    if (boardWidthSpin) boardWidthSpin->setValue(config.boardArea.width());
    if (boardHeightSpin) boardHeightSpin->setValue(config.boardArea.height());
    
    if (offsetXSpin) offsetXSpin->setValue(config.boardOffset.x());
    if (offsetYSpin) offsetYSpin->setValue(config.boardOffset.y());
    
    if (cellWidthSpin) cellWidthSpin->setValue(config.cellSize.width());
    if (cellHeightSpin) cellHeightSpin->setValue(config.cellSize.height());
    
    if (animationDelaySpin) animationDelaySpin->setValue(config.animationDelay);
}

/**
 * @brief 获取窗口信息
 */
QString ConnectionDialog::getWindowInfo(void* hwnd) const
{
#ifdef Q_OS_WIN
    HWND h = static_cast<HWND>(hwnd);
    
    // 获取窗口标题
    wchar_t title[256];
    int titleLen = GetWindowTextW(h, title, sizeof(title) / sizeof(wchar_t));
    QString windowTitle = titleLen > 0 ? QString::fromWCharArray(title, titleLen) : "";
    
    // 获取窗口类名
    wchar_t className[256];
    int classLen = GetClassNameW(h, className, sizeof(className) / sizeof(wchar_t));
    QString windowClass = classLen > 0 ? QString::fromWCharArray(className, classLen) : "";
    
    // 过滤掉一些系统窗口
    if (windowTitle.isEmpty() || windowClass.isEmpty()) {
        return QString();
    }
    
    // 获取进程名（使用更安全的方式，避免权限问题导致卡死）
    DWORD processId;
    GetWindowThreadProcessId(h, &processId);
    
    QString processName;
    // 使用更安全的权限标志，避免因权限不足导致卡死
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        wchar_t processPath[MAX_PATH];
        DWORD pathSize = MAX_PATH;
        // 使用QueryFullProcessImageName替代GetModuleFileNameExW，更安全
        if (QueryFullProcessImageNameW(hProcess, 0, processPath, &pathSize)) {
            QFileInfo fileInfo(QString::fromWCharArray(processPath));
            processName = fileInfo.baseName();
        }
        CloseHandle(hProcess);
    }
    
    // 如果获取进程名失败，使用进程ID作为标识
    if (processName.isEmpty()) {
        processName = QString("PID:%1").arg(processId);
    }
    
    return QString("%1 [%2] (%3)").arg(windowTitle).arg(windowClass).arg(processName);
#else
    Q_UNUSED(hwnd)
    return QString();
#endif
}

/**
 * @brief 管理连线方案
 */
void ConnectionDialog::onManageConnectionSchemes()
{
    ConnectionSchemeDialog dialog(this);
    
    // 如果当前有选中的平台配置，尝试转换为连线方案
    if (connector && platformCombo->currentIndex() >= 0) {
        PlatformConnector::PlatformConfig currentConfig = getCurrentConfig();
        
        ConnectionSchemeDialog::ConnectionScheme scheme;
        scheme.name = platformNameEdit->text();
        scheme.platformName = platformNameEdit->text();
        scheme.windowClass = currentConfig.windowClass;
        scheme.windowTitle = currentConfig.windowTitle;
        scheme.boardArea = currentConfig.boardArea;
        scheme.boardOffset = currentConfig.boardOffset;
        scheme.cellSize = currentConfig.cellSize;
        scheme.animationDelay = currentConfig.animationDelay;
        scheme.useChildWindow = currentConfig.useChildWindow;
        
        dialog.setCurrentScheme(scheme);
    }
    
    if (dialog.exec() == QDialog::Accepted) {
        // 用户可能修改了方案，这里可以选择是否应用选中的方案
        ConnectionSchemeDialog::ConnectionScheme selectedScheme = dialog.getCurrentScheme();
        if (!selectedScheme.name.isEmpty()) {
            showMessage(QString("已选择方案: %1").arg(selectedScheme.name));
        }
    }
}

/**
 * @brief 应用连线方案
 */
void ConnectionDialog::onApplyConnectionScheme()
{
    ConnectionSchemeDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        ConnectionSchemeDialog::ConnectionScheme scheme = dialog.getCurrentScheme();
        
        if (scheme.name.isEmpty()) {
            showMessage("请选择一个有效的连线方案", true);
            return;
        }
        
        // 将方案应用到当前配置
        PlatformConnector::PlatformConfig config;
        config.windowClass = scheme.windowClass;
        config.windowTitle = scheme.windowTitle;
        config.boardArea = scheme.boardArea;
        config.boardOffset = scheme.boardOffset;
        config.cellSize = scheme.cellSize;
        config.animationDelay = scheme.animationDelay;
        config.useChildWindow = scheme.useChildWindow;
        
        // 更新UI
        setCurrentConfig(config);
        
        // 更新平台名称显示
        platformNameEdit->setText(scheme.platformName);
        
        showMessage(QString("已应用连线方案: %1").arg(scheme.name));
        
        // 刷新连线方案列表
        onRefreshConnectionSchemes();
        
        // 自动切换到连接管理标签页
        if (tabWidget) {
            tabWidget->setCurrentIndex(1); // 连接管理是第二个标签页
        }
    }
}

/**
 * @brief 连线方案选择改变
 */
void ConnectionDialog::onConnectionSchemeSelected()
{
    if (!connectionSchemeCombo || connectionSchemeCombo->currentIndex() < 0) {
        return;
    }
    
    QString schemeName = connectionSchemeCombo->currentText();
    if (schemeName.isEmpty() || schemeName == "请选择连线方案") {
        return;
    }
    
    // 从ConnectionSchemeDialog获取方案数据
    ConnectionSchemeDialog dialog(this);
    QList<ConnectionSchemeDialog::ConnectionScheme> schemes = dialog.getAllSchemes();
    
    for (const auto& scheme : schemes) {
        if (scheme.name == schemeName) {
            // 将方案应用到当前配置
            PlatformConnector::PlatformConfig config;
            config.windowClass = scheme.windowClass;
            config.windowTitle = scheme.windowTitle;
            config.boardArea = scheme.boardArea;
            config.boardOffset = scheme.boardOffset;
            config.cellSize = scheme.cellSize;
            config.animationDelay = scheme.animationDelay;
            config.useChildWindow = scheme.useChildWindow;
            
            // 更新UI
            setCurrentConfig(config);
            
            // 更新平台名称显示
            platformNameEdit->setText(scheme.platformName);
            
            showMessage(QString("已选择连线方案: %1").arg(scheme.name));
            break;
        }
    }
}

/**
 * @brief 刷新连线方案列表
 */
void ConnectionDialog::onRefreshConnectionSchemes()
{
    if (!connectionSchemeCombo) {
        return;
    }
    
    connectionSchemeCombo->clear();
    connectionSchemeCombo->addItem("请选择连线方案");
    
    // 从ConnectionSchemeDialog获取所有方案
    ConnectionSchemeDialog dialog(this);
    QList<ConnectionSchemeDialog::ConnectionScheme> schemes = dialog.getAllSchemes();
    
    for (const auto& scheme : schemes) {
        connectionSchemeCombo->addItem(scheme.name);
    }
}

/**
 * @brief 绘制棋盘网格
 */
void ConnectionDialog::drawBoardGrid(QPixmap& pixmap)
{
    if (!connector) return;
    
    PlatformConnector::PlatformConfig config = getCurrentConfig();
    
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::red, 2));
    
    // 绘制棋盘边框
    painter.drawRect(config.boardArea);
    
    // 绘制网格线
    painter.setPen(QPen(Qt::blue, 1));
    
    int startX = config.boardArea.x() + config.boardOffset.x();
    int startY = config.boardArea.y() + config.boardOffset.y();
    
    // 垂直线 (9条)
    for (int i = 0; i <= 8; ++i) {
        int x = startX + i * config.cellSize.width();
        painter.drawLine(x, startY, x, startY + 9 * config.cellSize.height());
    }
    
    // 水平线 (10条)
    for (int i = 0; i <= 9; ++i) {
        int y = startY + i * config.cellSize.height();
        painter.drawLine(startX, y, startX + 8 * config.cellSize.width(), y);
    }
}

/**
 * @brief 绘制点击位置
 */
void ConnectionDialog::drawClickPosition(QPixmap& pixmap, int row, int col)
{
    if (!connector) return;
    
    PlatformConnector::PlatformConfig config = getCurrentConfig();
    
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::green, 3));
    painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
    
    int x = config.boardArea.x() + config.boardOffset.x() + col * config.cellSize.width();
    int y = config.boardArea.y() + config.boardOffset.y() + row * config.cellSize.height();
    
    // 绘制十字标记
    int size = 10;
    painter.drawLine(x - size, y, x + size, y);
    painter.drawLine(x, y - size, x, y + size);
    
    // 绘制圆圈
    painter.drawEllipse(x - size/2, y - size/2, size, size);
}

/**
 * @brief 显示消息
 */
void ConnectionDialog::showMessage(const QString& message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "警告", message);
    } else {
        // 可以使用状态栏或其他方式显示信息
        logMessage(message);
    }
}

/**
 * @brief 记录消息
 */
void ConnectionDialog::logMessage(const QString& message)
{
    if (debugOutput) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        debugOutput->append(QString("[%1] %2").arg(timestamp).arg(message));
        
        // 自动滚动到底部
        QTextCursor cursor = debugOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        debugOutput->setTextCursor(cursor);
    }
}

/**
 * @brief 验证配置
 */
bool ConnectionDialog::validateConfig() const
{
    PlatformConnector::PlatformConfig config = getCurrentConfig();
    
    if (config.windowTitle.isEmpty() && config.windowClass.isEmpty()) {
        return false;
    }
    
    if (config.boardArea.width() <= 0 || config.boardArea.height() <= 0) {
        return false;
    }
    
    if (config.cellSize.width() <= 0 || config.cellSize.height() <= 0) {
        return false;
    }
    
    return true;
}

/**
 * @brief 重置为默认值
 */
void ConnectionDialog::resetToDefaults()
{
    PlatformConnector::PlatformConfig defaultConfig;
    setCurrentConfig(defaultConfig);
}

/**
 * @brief 关闭事件
 */
void ConnectionDialog::closeEvent(QCloseEvent *event)
{
    // 停止定时器
    if (previewTimer) {
        previewTimer->stop();
    }
    if (statusTimer) {
        statusTimer->stop();
    }
    
    // 保存设置
    if (connector) {
        connector->saveSettings();
    }
    
    QDialog::closeEvent(event);
}

/**
 * @brief 显示事件
 */
void ConnectionDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // 更新界面
    updateUI();
    
    // 启动定时器
    if (statusTimer) {
        statusTimer->start();
    }
}

// ============================================================================
// CustomPlatformDialog 实现
// ============================================================================

/**
 * @brief 自定义平台对话框构造函数
 */
CustomPlatformDialog::CustomPlatformDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("自定义平台配置");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(400, 500);
    
    setupUI();
}

/**
 * @brief 设置平台配置
 */
void CustomPlatformDialog::setPlatformConfig(const QString& name, const PlatformConnector::PlatformConfig& config)
{
    nameEdit->setText(name);
    windowClassEdit->setText(config.windowClass);
    windowTitleEdit->setText(config.windowTitle);
    clickClassEdit->setText(config.clickClass);
    clickTitleEdit->setText(config.clickTitle);
    useChildWindowCheck->setChecked(config.useChildWindow);
    
    boardXSpin->setValue(config.boardArea.x());
    boardYSpin->setValue(config.boardArea.y());
    boardWidthSpin->setValue(config.boardArea.width());
    boardHeightSpin->setValue(config.boardArea.height());
    
    offsetXSpin->setValue(config.boardOffset.x());
    offsetYSpin->setValue(config.boardOffset.y());
    
    cellWidthSpin->setValue(config.cellSize.width());
    cellHeightSpin->setValue(config.cellSize.height());
    
    animationDelaySpin->setValue(config.animationDelay);
}

/**
 * @brief 获取平台名称
 */
QString CustomPlatformDialog::getPlatformName() const
{
    return nameEdit->text().trimmed();
}

/**
 * @brief 获取平台配置
 */
PlatformConnector::PlatformConfig CustomPlatformDialog::getPlatformConfig() const
{
    PlatformConnector::PlatformConfig config;
    
    config.name = nameEdit->text().trimmed();
    config.windowClass = windowClassEdit->text().trimmed();
    config.windowTitle = windowTitleEdit->text().trimmed();
    config.clickClass = clickClassEdit->text().trimmed();
    config.clickTitle = clickTitleEdit->text().trimmed();
    config.useChildWindow = useChildWindowCheck->isChecked();
    
    config.boardArea = QRect(boardXSpin->value(), boardYSpin->value(), 
                            boardWidthSpin->value(), boardHeightSpin->value());
    config.boardOffset = QPoint(offsetXSpin->value(), offsetYSpin->value());
    config.cellSize = QSize(cellWidthSpin->value(), cellHeightSpin->value());
    config.animationDelay = animationDelaySpin->value();
    
    return config;
}

/**
 * @brief 确认按钮
 */
void CustomPlatformDialog::onAccept()
{
    if (validateInput()) {
        accept();
    }
}

/**
 * @brief 检测窗口
 */
void CustomPlatformDialog::onDetectWindow()
{
    WindowSelectorDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        windowClassEdit->setText(dialog.getSelectedWindowClass());
        windowTitleEdit->setText(dialog.getSelectedWindowTitle());
    }
}

/**
 * @brief 测试配置
 */
void CustomPlatformDialog::onTestConfig()
{
    QMessageBox::information(this, "提示", "测试配置功能待实现");
}

/**
 * @brief 设置界面
 */
void CustomPlatformDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 基本信息
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    nameEdit = new QLineEdit;
    basicLayout->addRow("平台名称:", nameEdit);
    
    mainLayout->addWidget(basicGroup);
    
    // 窗口信息
    QGroupBox* windowGroup = new QGroupBox("窗口信息");
    QFormLayout* windowLayout = new QFormLayout(windowGroup);
    
    windowClassEdit = new QLineEdit;
    windowLayout->addRow("窗口类名:", windowClassEdit);
    
    windowTitleEdit = new QLineEdit;
    windowLayout->addRow("窗口标题:", windowTitleEdit);
    
    clickClassEdit = new QLineEdit;
    windowLayout->addRow("点击类名:", clickClassEdit);
    
    clickTitleEdit = new QLineEdit;
    windowLayout->addRow("点击标题:", clickTitleEdit);
    
    useChildWindowCheck = new QCheckBox("使用子窗口");
    windowLayout->addRow("", useChildWindowCheck);
    
    QHBoxLayout* windowBtnLayout = new QHBoxLayout;
    detectBtn = new QPushButton("检测窗口");
    connect(detectBtn, &QPushButton::clicked, this, &CustomPlatformDialog::onDetectWindow);
    windowBtnLayout->addWidget(detectBtn);
    windowBtnLayout->addStretch();
    windowLayout->addRow("", windowBtnLayout);
    
    mainLayout->addWidget(windowGroup);
    
    // 棋盘配置
    QGroupBox* boardGroup = new QGroupBox("棋盘配置");
    QFormLayout* boardLayout = new QFormLayout(boardGroup);
    
    boardXSpin = new QSpinBox;
    boardXSpin->setRange(0, 2000);
    boardLayout->addRow("棋盘X:", boardXSpin);
    
    boardYSpin = new QSpinBox;
    boardYSpin->setRange(0, 2000);
    boardLayout->addRow("棋盘Y:", boardYSpin);
    
    boardWidthSpin = new QSpinBox;
    boardWidthSpin->setRange(100, 1000);
    boardWidthSpin->setValue(360);
    boardLayout->addRow("棋盘宽度:", boardWidthSpin);
    
    boardHeightSpin = new QSpinBox;
    boardHeightSpin->setRange(100, 1000);
    boardHeightSpin->setValue(400);
    boardLayout->addRow("棋盘高度:", boardHeightSpin);
    
    offsetXSpin = new QSpinBox;
    offsetXSpin->setRange(0, 100);
    offsetXSpin->setValue(20);
    boardLayout->addRow("X偏移:", offsetXSpin);
    
    offsetYSpin = new QSpinBox;
    offsetYSpin->setRange(0, 100);
    offsetYSpin->setValue(20);
    boardLayout->addRow("Y偏移:", offsetYSpin);
    
    cellWidthSpin = new QSpinBox;
    cellWidthSpin->setRange(10, 100);
    cellWidthSpin->setValue(40);
    boardLayout->addRow("格子宽度:", cellWidthSpin);
    
    cellHeightSpin = new QSpinBox;
    cellHeightSpin->setRange(10, 100);
    cellHeightSpin->setValue(44);
    boardLayout->addRow("格子高度:", cellHeightSpin);
    
    animationDelaySpin = new QSpinBox;
    animationDelaySpin->setRange(0, 2000);
    animationDelaySpin->setValue(500);
    animationDelaySpin->setSuffix(" ms");
    boardLayout->addRow("动画延迟:", animationDelaySpin);
    
    mainLayout->addWidget(boardGroup);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");
    testBtn = new QPushButton("测试");
    
    connect(okBtn, &QPushButton::clicked, this, &CustomPlatformDialog::onAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(testBtn, &QPushButton::clicked, this, &CustomPlatformDialog::onTestConfig);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(testBtn);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
}

/**
 * @brief 验证输入
 */
bool CustomPlatformDialog::validateInput() const
{
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<CustomPlatformDialog*>(this), "错误", "请输入平台名称");
        return false;
    }
    
    if (windowTitleEdit->text().trimmed().isEmpty() && windowClassEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<CustomPlatformDialog*>(this), "错误", "请至少输入窗口标题或窗口类名");
        return false;
    }
    
    return true;
}

// ============================================================================
// WindowSelectorDialog 实现
// ============================================================================

/**
 * @brief 窗口选择对话框构造函数
 */
WindowSelectorDialog::WindowSelectorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("选择窗口");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(600, 400);
    
    setupUI();
    updateWindowList();
}

/**
 * @brief 获取选中的窗口类名
 */
QString WindowSelectorDialog::getSelectedWindowClass() const
{
    return selectedClass;
}

/**
 * @brief 获取选中的窗口标题
 */
QString WindowSelectorDialog::getSelectedWindowTitle() const
{
    return selectedTitle;
}

/**
 * @brief 刷新窗口列表
 */
void WindowSelectorDialog::onRefresh()
{
    updateWindowList();
}

/**
 * @brief 窗口选择改变
 */
void WindowSelectorDialog::onWindowSelected()
{
    QListWidgetItem* item = windowList->currentItem();
    if (item) {
        QString info = item->text();
        infoLabel->setText(QString("选中: %1").arg(info));
        
        // 解析窗口信息
        QRegularExpression rx("^(.+) \\[(.+)\\] \\((.+)\\)$");
        QRegularExpressionMatch match = rx.match(info);
        if (match.hasMatch()) {
            selectedTitle = match.captured(1);
            selectedClass = match.captured(2);
        }
    }
}

/**
 * @brief 确认选择
 */
void WindowSelectorDialog::onAccept()
{
    if (windowList->currentItem()) {
        accept();
    } else {
        QMessageBox::warning(this, "警告", "请先选择一个窗口");
    }
}

/**
 * @brief 设置界面
 */
void WindowSelectorDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 刷新按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    refreshBtn = new QPushButton("刷新列表");
    connect(refreshBtn, &QPushButton::clicked, this, &WindowSelectorDialog::onRefresh);
    
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // 窗口列表
    windowList = new QListWidget;
    connect(windowList, &QListWidget::itemSelectionChanged, this, &WindowSelectorDialog::onWindowSelected);
    connect(windowList, &QListWidget::itemDoubleClicked, this, &WindowSelectorDialog::onAccept);
    mainLayout->addWidget(windowList);
    
    // 信息标签
    infoLabel = new QLabel("请选择一个窗口");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
    mainLayout->addWidget(infoLabel);
    
    // 底部按钮
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");
    
    connect(okBtn, &QPushButton::clicked, this, &WindowSelectorDialog::onAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    bottomLayout->addStretch();
    bottomLayout->addWidget(okBtn);
    bottomLayout->addWidget(cancelBtn);
    
    mainLayout->addLayout(bottomLayout);
}

/**
 * @brief 更新窗口列表
 */
void WindowSelectorDialog::updateWindowList()
{
    windowList->clear();
    
#ifdef Q_OS_WIN
    // 枚举所有顶级窗口
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowSelectorDialog* dialog = reinterpret_cast<WindowSelectorDialog*>(lParam);
        
        if (IsWindowVisible(hwnd)) {
            QString info = dialog->getWindowInfo(hwnd);
            if (!info.isEmpty()) {
                QListWidgetItem* item = new QListWidgetItem(info);
                item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(hwnd)));
                dialog->windowList->addItem(item);
            }
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(this));
#endif
    
    infoLabel->setText(QString("找到 %1 个窗口").arg(windowList->count()));
}

/**
 * @brief 获取窗口信息
 */
QString WindowSelectorDialog::getWindowInfo(void* hwnd) const
{
#ifdef Q_OS_WIN
    HWND h = static_cast<HWND>(hwnd);
    
    // 获取窗口标题
    wchar_t title[256];
    int titleLen = GetWindowTextW(h, title, sizeof(title) / sizeof(wchar_t));
    QString windowTitle = titleLen > 0 ? QString::fromWCharArray(title, titleLen) : "";
    
    // 获取窗口类名
    wchar_t className[256];
    int classLen = GetClassNameW(h, className, sizeof(className) / sizeof(wchar_t));
    QString windowClass = classLen > 0 ? QString::fromWCharArray(className, classLen) : "";
    
    // 过滤掉一些系统窗口
    if (windowTitle.isEmpty() || windowClass.isEmpty()) {
        return QString();
    }
    
    // 获取进程名
    DWORD processId;
    GetWindowThreadProcessId(h, &processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    QString processName;
    if (hProcess) {
        wchar_t processPath[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, nullptr, processPath, MAX_PATH)) {
            QFileInfo fileInfo(QString::fromWCharArray(processPath));
            processName = fileInfo.baseName();
        }
        CloseHandle(hProcess);
    }
    
    return QString("%1 [%2] (%3)").arg(windowTitle).arg(windowClass).arg(processName);
#else
    Q_UNUSED(hwnd)
    return QString();
#endif
}