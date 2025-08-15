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
#include <QInputDialog>
#include <QHeaderView>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

/**
 * @brief JSON序列化
 */
QJsonObject ConnectionSchemeDialog::ConnectionScheme::toJson() const
{
    QJsonObject obj;
    obj["name"] = name;
    obj["platformName"] = platformName;
    obj["windowClass"] = windowClass;
    obj["windowTitle"] = windowTitle;
    obj["processName"] = processName;
    
    QJsonObject boardAreaObj;
    boardAreaObj["x"] = boardArea.x();
    boardAreaObj["y"] = boardArea.y();
    boardAreaObj["width"] = boardArea.width();
    boardAreaObj["height"] = boardArea.height();
    obj["boardArea"] = boardAreaObj;
    
    QJsonObject boardOffsetObj;
    boardOffsetObj["x"] = boardOffset.x();
    boardOffsetObj["y"] = boardOffset.y();
    obj["boardOffset"] = boardOffsetObj;
    
    QJsonObject cellSizeObj;
    cellSizeObj["width"] = cellSize.width();
    cellSizeObj["height"] = cellSize.height();
    obj["cellSize"] = cellSizeObj;
    
    obj["animationDelay"] = animationDelay;
    obj["useChildWindow"] = useChildWindow;
    obj["description"] = description;
    obj["enabled"] = enabled;
    
    return obj;
}

/**
 * @brief JSON反序列化
 */
void ConnectionSchemeDialog::ConnectionScheme::fromJson(const QJsonObject& json)
{
    name = json["name"].toString();
    platformName = json["platformName"].toString();
    windowClass = json["windowClass"].toString();
    windowTitle = json["windowTitle"].toString();
    processName = json["processName"].toString();
    
    QJsonObject boardAreaObj = json["boardArea"].toObject();
    boardArea = QRect(boardAreaObj["x"].toInt(), boardAreaObj["y"].toInt(),
                     boardAreaObj["width"].toInt(), boardAreaObj["height"].toInt());
    
    QJsonObject boardOffsetObj = json["boardOffset"].toObject();
    boardOffset = QPoint(boardOffsetObj["x"].toInt(), boardOffsetObj["y"].toInt());
    
    QJsonObject cellSizeObj = json["cellSize"].toObject();
    cellSize = QSize(cellSizeObj["width"].toInt(), cellSizeObj["height"].toInt());
    
    animationDelay = json["animationDelay"].toInt(800);
    useChildWindow = json["useChildWindow"].toBool(false);
    description = json["description"].toString();
    enabled = json["enabled"].toBool(true);
}

/**
 * @brief 构造函数
 */
ConnectionSchemeDialog::ConnectionSchemeDialog(QWidget *parent)
    : QDialog(parent)
    , currentSchemeIndex(-1)
    , dataChanged(false)
    , settings(new QSettings(this))
{
    // 设置窗口属性
    setWindowTitle("连线方案管理");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(800, 600);
    
    // 初始化界面
    setupUI();
    
    // 加载数据
    loadSchemes();
    updateSchemeList();
    
    // 如果有方案，选中第一个
    if (!schemes.isEmpty()) {
        schemeList->setCurrentRow(0);
        onSchemeSelectionChanged();
    }
}

/**
 * @brief 析构函数
 */
ConnectionSchemeDialog::~ConnectionSchemeDialog()
{
    if (dataChanged) {
        saveSchemes();
    }
}

/**
 * @brief 获取当前选中的方案
 */
ConnectionSchemeDialog::ConnectionScheme ConnectionSchemeDialog::getCurrentScheme() const
{
    if (currentSchemeIndex >= 0 && currentSchemeIndex < schemes.size()) {
        return schemes[currentSchemeIndex];
    }
    return ConnectionScheme();
}

/**
 * @brief 设置当前方案
 */
void ConnectionSchemeDialog::setCurrentScheme(const ConnectionScheme& scheme)
{
    // 查找匹配的方案
    for (int i = 0; i < schemes.size(); ++i) {
        if (schemes[i].name == scheme.name) {
            schemeList->setCurrentRow(i);
            onSchemeSelectionChanged();
            return;
        }
    }
    
    // 如果没找到，添加新方案
    addScheme(scheme);
}

/**
 * @brief 添加新方案
 */
void ConnectionSchemeDialog::addScheme(const ConnectionScheme& scheme)
{
    schemes.append(scheme);
    updateSchemeList();
    
    // 选中新添加的方案
    schemeList->setCurrentRow(schemes.size() - 1);
    onSchemeSelectionChanged();
    
    dataChanged = true;
}

/**
 * @brief 关闭事件
 */
void ConnectionSchemeDialog::closeEvent(QCloseEvent *event)
{
    if (dataChanged) {
        int ret = QMessageBox::question(this, "保存更改", 
            "连线方案已修改，是否保存更改？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            saveSchemes();
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
            return;
        }
    }
    
    QDialog::closeEvent(event);
}

/**
 * @brief 显示事件
 */
void ConnectionSchemeDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // 刷新窗口列表（在显示时进行，避免构造时卡死）
    // detectAvailableWindows();
}

/**
 * @brief 添加方案
 */
void ConnectionSchemeDialog::onAddScheme()
{
    bool ok;
    QString name = QInputDialog::getText(this, "添加连线方案", 
        "请输入方案名称:", QLineEdit::Normal, "", &ok);
    
    if (!ok || name.isEmpty()) {
        return;
    }
    
    // 检查名称是否重复
    for (const auto& scheme : schemes) {
        if (scheme.name == name) {
            QMessageBox::warning(this, "错误", "方案名称已存在！");
            return;
        }
    }
    
    // 创建新方案
    ConnectionScheme newScheme;
    newScheme.name = name;
    newScheme.platformName = "自定义平台";
    
    schemes.append(newScheme);
    updateSchemeList();
    
    // 选中新方案
    schemeList->setCurrentRow(schemes.size() - 1);
    onSchemeSelectionChanged();
    
    dataChanged = true;
}

/**
 * @brief 编辑方案
 */
void ConnectionSchemeDialog::onEditScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        return;
    }
    
    bool ok;
    QString newName = QInputDialog::getText(this, "编辑连线方案", 
        "请输入新的方案名称:", QLineEdit::Normal, 
        schemes[currentSchemeIndex].name, &ok);
    
    if (!ok || newName.isEmpty()) {
        return;
    }
    
    // 检查名称是否重复（排除当前方案）
    for (int i = 0; i < schemes.size(); ++i) {
        if (i != currentSchemeIndex && schemes[i].name == newName) {
            QMessageBox::warning(this, "错误", "方案名称已存在！");
            return;
        }
    }
    
    // 更新方案名称
    schemes[currentSchemeIndex].name = newName;
    updateSchemeList();
    
    // 保持选中状态
    schemeList->setCurrentRow(currentSchemeIndex);
    
    dataChanged = true;
}

/**
 * @brief 删除方案
 */
void ConnectionSchemeDialog::onDeleteScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        return;
    }
    
    int ret = QMessageBox::question(this, "删除方案", 
        QString("确定要删除方案 '%1' 吗？").arg(schemes[currentSchemeIndex].name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        schemes.removeAt(currentSchemeIndex);
        updateSchemeList();
        
        // 选中下一个方案
        if (currentSchemeIndex >= schemes.size()) {
            currentSchemeIndex = schemes.size() - 1;
        }
        
        if (currentSchemeIndex >= 0) {
            schemeList->setCurrentRow(currentSchemeIndex);
            onSchemeSelectionChanged();
        } else {
            currentSchemeIndex = -1;
            updateSchemeDetails();
        }
        
        dataChanged = true;
    }
}

/**
 * @brief 复制方案
 */
void ConnectionSchemeDialog::onDuplicateScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        return;
    }
    
    ConnectionScheme currentScheme = schemes[currentSchemeIndex];
    
    // 生成新名称
    QString baseName = currentScheme.name;
    QString newName = baseName + " - 副本";
    int counter = 1;
    
    // 确保名称唯一
    while (true) {
        bool nameExists = false;
        for (const auto& scheme : schemes) {
            if (scheme.name == newName) {
                nameExists = true;
                break;
            }
        }
        
        if (!nameExists) {
            break;
        }
        
        counter++;
        newName = QString("%1 - 副本%2").arg(baseName).arg(counter);
    }
    
    currentScheme.name = newName;
    schemes.append(currentScheme);
    updateSchemeList();
    
    // 选中新方案
    schemeList->setCurrentRow(schemes.size() - 1);
    onSchemeSelectionChanged();
    
    dataChanged = true;
}

/**
 * @brief 方案选择改变
 */
void ConnectionSchemeDialog::onSchemeSelectionChanged()
{
    int row = schemeList->currentRow();
    if (row >= 0 && row < schemes.size()) {
        currentSchemeIndex = row;
        updateSchemeDetails();
    } else {
        currentSchemeIndex = -1;
        updateSchemeDetails();
    }
}

/**
 * @brief 导入方案
 */
void ConnectionSchemeDialog::onImportSchemes()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入连线方案", 
        "", "JSON文件 (*.json);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件！");
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "错误", "JSON格式错误：" + error.errorString());
        return;
    }
    
    QJsonArray array = doc.array();
    int importCount = 0;
    
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        ConnectionScheme scheme;
        scheme.fromJson(obj);
        
        // 检查名称是否重复
        bool nameExists = false;
        for (const auto& existingScheme : schemes) {
            if (existingScheme.name == scheme.name) {
                nameExists = true;
                break;
            }
        }
        
        if (nameExists) {
            // 生成新名称
            QString baseName = scheme.name;
            QString newName = baseName + " (导入)";
            int counter = 1;
            
            while (true) {
                bool stillExists = false;
                for (const auto& existingScheme : schemes) {
                    if (existingScheme.name == newName) {
                        stillExists = true;
                        break;
                    }
                }
                
                if (!stillExists) {
                    break;
                }
                
                counter++;
                newName = QString("%1 (导入%2)").arg(baseName).arg(counter);
            }
            
            scheme.name = newName;
        }
        
        schemes.append(scheme);
        importCount++;
    }
    
    if (importCount > 0) {
        updateSchemeList();
        dataChanged = true;
        showMessage(QString("成功导入 %1 个连线方案").arg(importCount));
    } else {
        showMessage("没有导入任何方案");
    }
}

/**
 * @brief 导出所有方案
 */
void ConnectionSchemeDialog::onExportSchemes()
{
    if (schemes.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可导出的方案！");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出连线方案", 
        QString("connection_schemes_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON文件 (*.json);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QJsonArray array;
    for (const auto& scheme : schemes) {
        array.append(scheme.toJson());
    }
    
    QJsonDocument doc(array);
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法创建文件！");
        return;
    }
    
    file.write(doc.toJson());
    showMessage(QString("成功导出 %1 个连线方案").arg(schemes.size()));
}

/**
 * @brief 导出当前方案
 */
void ConnectionSchemeDialog::onExportCurrentScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        QMessageBox::information(this, "提示", "请先选择一个方案！");
        return;
    }
    
    ConnectionScheme currentScheme = schemes[currentSchemeIndex];
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出连线方案", 
        QString("%1_%2.json").arg(currentScheme.name).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON文件 (*.json);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QJsonArray array;
    array.append(currentScheme.toJson());
    
    QJsonDocument doc(array);
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法创建文件！");
        return;
    }
    
    file.write(doc.toJson());
    showMessage(QString("成功导出方案 '%1'").arg(currentScheme.name));
}

/**
 * @brief 测试方案
 */
void ConnectionSchemeDialog::onTestScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        QMessageBox::information(this, "提示", "请先选择一个方案！");
        return;
    }
    
    // 同步UI数据到当前方案
    syncCurrentSchemeToUI();
    
    ConnectionScheme scheme = schemes[currentSchemeIndex];
    
    QString errorMsg;
    if (!validateScheme(scheme, errorMsg)) {
        QMessageBox::warning(this, "方案验证失败", errorMsg);
        return;
    }
    
    // TODO: 实现方案测试逻辑
    // 1. 查找匹配的窗口
    // 2. 截图测试
    // 3. 显示测试结果
    
    showMessage("方案测试功能待实现");
}

/**
 * @brief 捕获窗口
 */
void ConnectionSchemeDialog::onCaptureWindow()
{
    showMessage("窗口捕获功能待实现");
}

/**
 * @brief 检测窗口
 */
void ConnectionSchemeDialog::onDetectWindow()
{
    QStringList windows = detectAvailableWindows();
    
    if (windows.isEmpty()) {
        showMessage("未检测到可用窗口");
        return;
    }
    
    QString windowInfo = "检测到的窗口:\n" + windows.join("\n");
    QMessageBox::information(this, "窗口检测结果", windowInfo);
}

/**
 * @brief 方案数据改变
 */
void ConnectionSchemeDialog::onSchemeDataChanged()
{
    if (currentSchemeIndex >= 0 && currentSchemeIndex < schemes.size()) {
        syncCurrentSchemeToUI();
        dataChanged = true;
    }
}

/**
 * @brief 更新方案列表
 */
void ConnectionSchemeDialog::updateSchemeList()
{
    schemeList->clear();
    
    for (const auto& scheme : schemes) {
        QListWidgetItem* item = new QListWidgetItem(scheme.name);
        
        // 设置图标和状态
        if (!scheme.enabled) {
            item->setForeground(QColor(128, 128, 128));
            item->setText(scheme.name + " (已禁用)");
        }
        
        schemeList->addItem(item);
    }
}

/**
 * @brief 更新方案详情
 */
void ConnectionSchemeDialog::updateSchemeDetails()
{
    bool hasSelection = (currentSchemeIndex >= 0 && currentSchemeIndex < schemes.size());
    
    // 启用/禁用控件
    nameEdit->setEnabled(hasSelection);
    platformNameEdit->setEnabled(hasSelection);
    windowClassEdit->setEnabled(hasSelection);
    windowTitleEdit->setEnabled(hasSelection);
    processNameEdit->setEnabled(hasSelection);
    boardXSpin->setEnabled(hasSelection);
    boardYSpin->setEnabled(hasSelection);
    boardWidthSpin->setEnabled(hasSelection);
    boardHeightSpin->setEnabled(hasSelection);
    offsetXSpin->setEnabled(hasSelection);
    offsetYSpin->setEnabled(hasSelection);
    cellWidthSpin->setEnabled(hasSelection);
    cellHeightSpin->setEnabled(hasSelection);
    animationDelaySpin->setEnabled(hasSelection);
    useChildWindowCheck->setEnabled(hasSelection);
    descriptionEdit->setEnabled(hasSelection);
    enabledCheck->setEnabled(hasSelection);
    
    editSchemeBtn->setEnabled(hasSelection);
    deleteSchemeBtn->setEnabled(hasSelection);
    duplicateSchemeBtn->setEnabled(hasSelection);
    exportCurrentBtn->setEnabled(hasSelection);
    testSchemeBtn->setEnabled(hasSelection);
    
    if (hasSelection) {
        syncUIToCurrentScheme();
    } else {
        // 清空所有控件
        nameEdit->clear();
        platformNameEdit->clear();
        windowClassEdit->clear();
        windowTitleEdit->clear();
        processNameEdit->clear();
        boardXSpin->setValue(0);
        boardYSpin->setValue(0);
        boardWidthSpin->setValue(400);
        boardHeightSpin->setValue(450);
        offsetXSpin->setValue(20);
        offsetYSpin->setValue(25);
        cellWidthSpin->setValue(44);
        cellHeightSpin->setValue(50);
        animationDelaySpin->setValue(800);
        useChildWindowCheck->setChecked(false);
        descriptionEdit->clear();
        enabledCheck->setChecked(true);
    }
}

/**
 * @brief 初始化界面
 */
void ConnectionSchemeDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建分割器
    mainSplitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(mainSplitter);
    
    // 设置左右两侧
    setupSchemeListWidget();
    setupSchemeDetailsWidget();
    
    // 底部按钮
    setupButtonsWidget();
    mainLayout->addLayout(createButtonLayout());
    
    // 设置分割器比例
    mainSplitter->setSizes({250, 550});
}

/**
 * @brief 设置方案列表控件
 */
void ConnectionSchemeDialog::setupSchemeListWidget()
{
    QWidget* leftWidget = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    
    // 标题
    QLabel* titleLabel = new QLabel("连线方案列表");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    leftLayout->addWidget(titleLabel);
    
    // 方案列表
    schemeList = new QListWidget;
    connect(schemeList, &QListWidget::itemSelectionChanged, 
            this, &ConnectionSchemeDialog::onSchemeSelectionChanged);
    leftLayout->addWidget(schemeList);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    addSchemeBtn = new QPushButton("添加");
    editSchemeBtn = new QPushButton("编辑");
    deleteSchemeBtn = new QPushButton("删除");
    duplicateSchemeBtn = new QPushButton("复制");
    
    connect(addSchemeBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onAddScheme);
    connect(editSchemeBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onEditScheme);
    connect(deleteSchemeBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onDeleteScheme);
    connect(duplicateSchemeBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onDuplicateScheme);
    
    buttonLayout->addWidget(addSchemeBtn);
    buttonLayout->addWidget(editSchemeBtn);
    buttonLayout->addWidget(deleteSchemeBtn);
    buttonLayout->addWidget(duplicateSchemeBtn);
    
    leftLayout->addLayout(buttonLayout);
    
    mainSplitter->addWidget(leftWidget);
}

/**
 * @brief 设置方案详情控件
 */
void ConnectionSchemeDialog::setupSchemeDetailsWidget()
{
    QWidget* rightWidget = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    
    // 标题
    QLabel* titleLabel = new QLabel("方案详情");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    rightLayout->addWidget(titleLabel);
    
    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    QWidget* scrollWidget = new QWidget;
    QFormLayout* formLayout = new QFormLayout(scrollWidget);
    
    // 基本信息
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    nameEdit = new QLineEdit;
    connect(nameEdit, &QLineEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    basicLayout->addRow("方案名称:", nameEdit);
    
    platformNameEdit = new QLineEdit;
    connect(platformNameEdit, &QLineEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    basicLayout->addRow("平台名称:", platformNameEdit);
    
    enabledCheck = new QCheckBox("启用此方案");
    connect(enabledCheck, &QCheckBox::toggled, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    basicLayout->addRow("", enabledCheck);
    
    formLayout->addRow(basicGroup);
    
    // 窗口识别
    QGroupBox* windowGroup = new QGroupBox("窗口识别");
    QFormLayout* windowLayout = new QFormLayout(windowGroup);
    
    windowClassEdit = new QLineEdit;
    connect(windowClassEdit, &QLineEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    windowLayout->addRow("窗口类名:", windowClassEdit);
    
    windowTitleEdit = new QLineEdit;
    connect(windowTitleEdit, &QLineEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    windowLayout->addRow("窗口标题:", windowTitleEdit);
    
    processNameEdit = new QLineEdit;
    connect(processNameEdit, &QLineEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    windowLayout->addRow("进程名称:", processNameEdit);
    
    useChildWindowCheck = new QCheckBox("使用子窗口");
    connect(useChildWindowCheck, &QCheckBox::toggled, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    windowLayout->addRow("", useChildWindowCheck);
    
    formLayout->addRow(windowGroup);
    
    // 棋盘区域
    QGroupBox* boardGroup = new QGroupBox("棋盘区域");
    QGridLayout* boardLayout = new QGridLayout(boardGroup);
    
    boardXSpin = new QSpinBox;
    boardXSpin->setRange(0, 9999);
    connect(boardXSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    boardLayout->addWidget(new QLabel("X:"), 0, 0);
    boardLayout->addWidget(boardXSpin, 0, 1);
    
    boardYSpin = new QSpinBox;
    boardYSpin->setRange(0, 9999);
    connect(boardYSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    boardLayout->addWidget(new QLabel("Y:"), 0, 2);
    boardLayout->addWidget(boardYSpin, 0, 3);
    
    boardWidthSpin = new QSpinBox;
    boardWidthSpin->setRange(100, 9999);
    connect(boardWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    boardLayout->addWidget(new QLabel("宽度:"), 1, 0);
    boardLayout->addWidget(boardWidthSpin, 1, 1);
    
    boardHeightSpin = new QSpinBox;
    boardHeightSpin->setRange(100, 9999);
    connect(boardHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    boardLayout->addWidget(new QLabel("高度:"), 1, 2);
    boardLayout->addWidget(boardHeightSpin, 1, 3);
    
    formLayout->addRow(boardGroup);
    
    // 棋盘偏移
    QGroupBox* offsetGroup = new QGroupBox("棋盘偏移");
    QGridLayout* offsetLayout = new QGridLayout(offsetGroup);
    
    offsetXSpin = new QSpinBox;
    offsetXSpin->setRange(-999, 999);
    connect(offsetXSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    offsetLayout->addWidget(new QLabel("X偏移:"), 0, 0);
    offsetLayout->addWidget(offsetXSpin, 0, 1);
    
    offsetYSpin = new QSpinBox;
    offsetYSpin->setRange(-999, 999);
    connect(offsetYSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    offsetLayout->addWidget(new QLabel("Y偏移:"), 0, 2);
    offsetLayout->addWidget(offsetYSpin, 0, 3);
    
    formLayout->addRow(offsetGroup);
    
    // 格子大小
    QGroupBox* cellGroup = new QGroupBox("格子大小");
    QGridLayout* cellLayout = new QGridLayout(cellGroup);
    
    cellWidthSpin = new QSpinBox;
    cellWidthSpin->setRange(10, 200);
    connect(cellWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    cellLayout->addWidget(new QLabel("宽度:"), 0, 0);
    cellLayout->addWidget(cellWidthSpin, 0, 1);
    
    cellHeightSpin = new QSpinBox;
    cellHeightSpin->setRange(10, 200);
    connect(cellHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    cellLayout->addWidget(new QLabel("高度:"), 0, 2);
    cellLayout->addWidget(cellHeightSpin, 0, 3);
    
    formLayout->addRow(cellGroup);
    
    // 其他设置
    QGroupBox* otherGroup = new QGroupBox("其他设置");
    QFormLayout* otherLayout = new QFormLayout(otherGroup);
    
    animationDelaySpin = new QSpinBox;
    animationDelaySpin->setRange(0, 5000);
    animationDelaySpin->setSuffix(" 毫秒");
    connect(animationDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionSchemeDialog::onSchemeDataChanged);
    otherLayout->addRow("动画延迟:", animationDelaySpin);
    
    formLayout->addRow(otherGroup);
    
    // 描述
    QGroupBox* descGroup = new QGroupBox("方案描述");
    QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
    
    descriptionEdit = new QTextEdit;
    descriptionEdit->setMaximumHeight(80);
    connect(descriptionEdit, &QTextEdit::textChanged, this, &ConnectionSchemeDialog::onSchemeDataChanged);
    descLayout->addWidget(descriptionEdit);
    
    formLayout->addRow(descGroup);
    
    scrollArea->setWidget(scrollWidget);
    rightLayout->addWidget(scrollArea);
    
    mainSplitter->addWidget(rightWidget);
}

/**
 * @brief 设置按钮控件
 */
void ConnectionSchemeDialog::setupButtonsWidget()
{
    // 按钮在createButtonLayout中创建
}

/**
 * @brief 创建底部按钮布局
 */
QHBoxLayout* ConnectionSchemeDialog::createButtonLayout()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    // 左侧功能按钮
    importBtn = new QPushButton("导入方案");
    exportBtn = new QPushButton("导出所有");
    exportCurrentBtn = new QPushButton("导出当前");
    testSchemeBtn = new QPushButton("测试方案");
    captureWindowBtn = new QPushButton("捕获窗口");
    detectWindowBtn = new QPushButton("检测窗口");
    
    connect(importBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onImportSchemes);
    connect(exportBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onExportSchemes);
    connect(exportCurrentBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onExportCurrentScheme);
    connect(testSchemeBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onTestScheme);
    connect(captureWindowBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onCaptureWindow);
    connect(detectWindowBtn, &QPushButton::clicked, this, &ConnectionSchemeDialog::onDetectWindow);
    
    buttonLayout->addWidget(importBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(exportCurrentBtn);
    buttonLayout->addWidget(testSchemeBtn);
    buttonLayout->addWidget(captureWindowBtn);
    buttonLayout->addWidget(detectWindowBtn);
    
    buttonLayout->addStretch();
    
    // 右侧确认按钮
    okBtn = new QPushButton("确定");
    cancelBtn = new QPushButton("取消");
    
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    
    return buttonLayout;
}

/**
 * @brief 加载方案
 */
void ConnectionSchemeDialog::loadSchemes()
{
    QString filePath = getSchemesFilePath();
    
    QFile file(filePath);
    if (!file.exists()) {
        // 创建默认方案
        createDefaultSchemes();
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        showMessage("无法读取方案文件", true);
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        showMessage("方案文件格式错误：" + error.errorString(), true);
        return;
    }
    
    schemes.clear();
    QJsonArray array = doc.array();
    
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        ConnectionScheme scheme;
        scheme.fromJson(obj);
        schemes.append(scheme);
    }
}

/**
 * @brief 保存方案
 */
void ConnectionSchemeDialog::saveSchemes()
{
    QString filePath = getSchemesFilePath();
    
    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QJsonArray array;
    for (const auto& scheme : schemes) {
        array.append(scheme.toJson());
    }
    
    QJsonDocument doc(array);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        showMessage("无法保存方案文件", true);
        return;
    }
    
    file.write(doc.toJson());
    dataChanged = false;
}

/**
 * @brief 创建默认方案
 */
void ConnectionSchemeDialog::createDefaultSchemes()
{
    // 天天象棋
    ConnectionScheme ttScheme;
    ttScheme.name = "天天象棋";
    ttScheme.platformName = "天天象棋";
    ttScheme.windowTitle = "天天象棋";
    ttScheme.processName = "TencentChess.exe";
    ttScheme.boardArea = QRect(50, 80, 360, 400);
    ttScheme.boardOffset = QPoint(20, 20);
    ttScheme.cellSize = QSize(40, 44);
    ttScheme.animationDelay = 800;
    ttScheme.description = "腾讯天天象棋游戏";
    schemes.append(ttScheme);
    
    // QQ象棋
    ConnectionScheme qqScheme;
    qqScheme.name = "QQ象棋";
    qqScheme.platformName = "QQ象棋";
    qqScheme.windowTitle = "QQ游戏";
    qqScheme.processName = "QQGame.exe";
    qqScheme.boardArea = QRect(60, 100, 360, 400);
    qqScheme.boardOffset = QPoint(25, 25);
    qqScheme.cellSize = QSize(40, 44);
    qqScheme.animationDelay = 600;
    qqScheme.useChildWindow = true;
    qqScheme.description = "QQ游戏大厅中的象棋";
    schemes.append(qqScheme);
    
    // 中国象棋大师
    ConnectionScheme masterScheme;
    masterScheme.name = "中国象棋大师";
    masterScheme.platformName = "中国象棋大师";
    masterScheme.windowTitle = "中国象棋大师";
    masterScheme.processName = "ChessMaster.exe";
    masterScheme.boardArea = QRect(40, 70, 360, 400);
    masterScheme.boardOffset = QPoint(18, 18);
    masterScheme.cellSize = QSize(40, 44);
    masterScheme.animationDelay = 500;
    masterScheme.description = "中国象棋大师游戏";
    schemes.append(masterScheme);
    
    dataChanged = true;
}

/**
 * @brief 同步UI到当前方案
 */
void ConnectionSchemeDialog::syncUIToCurrentScheme()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        return;
    }
    
    ConnectionScheme& scheme = schemes[currentSchemeIndex];
    
    nameEdit->setText(scheme.name);
    platformNameEdit->setText(scheme.platformName);
    windowClassEdit->setText(scheme.windowClass);
    windowTitleEdit->setText(scheme.windowTitle);
    processNameEdit->setText(scheme.processName);
    boardXSpin->setValue(scheme.boardArea.x());
    boardYSpin->setValue(scheme.boardArea.y());
    boardWidthSpin->setValue(scheme.boardArea.width());
    boardHeightSpin->setValue(scheme.boardArea.height());
    offsetXSpin->setValue(scheme.boardOffset.x());
    offsetYSpin->setValue(scheme.boardOffset.y());
    cellWidthSpin->setValue(scheme.cellSize.width());
    cellHeightSpin->setValue(scheme.cellSize.height());
    animationDelaySpin->setValue(scheme.animationDelay);
    useChildWindowCheck->setChecked(scheme.useChildWindow);
    descriptionEdit->setPlainText(scheme.description);
    enabledCheck->setChecked(scheme.enabled);
}

/**
 * @brief 同步当前方案到UI
 */
void ConnectionSchemeDialog::syncCurrentSchemeToUI()
{
    if (currentSchemeIndex < 0 || currentSchemeIndex >= schemes.size()) {
        return;
    }
    
    ConnectionScheme& scheme = schemes[currentSchemeIndex];
    
    scheme.name = nameEdit->text();
    scheme.platformName = platformNameEdit->text();
    scheme.windowClass = windowClassEdit->text();
    scheme.windowTitle = windowTitleEdit->text();
    scheme.processName = processNameEdit->text();
    scheme.boardArea = QRect(boardXSpin->value(), boardYSpin->value(),
                           boardWidthSpin->value(), boardHeightSpin->value());
    scheme.boardOffset = QPoint(offsetXSpin->value(), offsetYSpin->value());
    scheme.cellSize = QSize(cellWidthSpin->value(), cellHeightSpin->value());
    scheme.animationDelay = animationDelaySpin->value();
    scheme.useChildWindow = useChildWindowCheck->isChecked();
    scheme.description = descriptionEdit->toPlainText();
    scheme.enabled = enabledCheck->isChecked();
}

/**
 * @brief 显示消息
 */
void ConnectionSchemeDialog::showMessage(const QString& message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "错误", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

/**
 * @brief 获取方案文件路径
 */
QString ConnectionSchemeDialog::getSchemesFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(configDir).filePath("connection_schemes.json");
}

/**
 * @brief 验证方案
 */
bool ConnectionSchemeDialog::validateScheme(const ConnectionScheme& scheme, QString& errorMsg) const
{
    if (scheme.name.isEmpty()) {
        errorMsg = "方案名称不能为空";
        return false;
    }
    
    if (scheme.platformName.isEmpty()) {
        errorMsg = "平台名称不能为空";
        return false;
    }
    
    if (scheme.windowTitle.isEmpty() && scheme.windowClass.isEmpty() && scheme.processName.isEmpty()) {
        errorMsg = "至少需要指定窗口标题、窗口类名或进程名称中的一个";
        return false;
    }
    
    if (scheme.boardArea.width() <= 0 || scheme.boardArea.height() <= 0) {
        errorMsg = "棋盘区域大小必须大于0";
        return false;
    }
    
    if (scheme.cellSize.width() <= 0 || scheme.cellSize.height() <= 0) {
        errorMsg = "格子大小必须大于0";
        return false;
    }
    
    return true;
}

/**
 * @brief 检测可用窗口
 */
QStringList ConnectionSchemeDialog::detectAvailableWindows()
{
    QStringList windows;
    
#ifdef Q_OS_WIN
    // Windows平台窗口枚举
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        QStringList* windowList = reinterpret_cast<QStringList*>(lParam);
        
        if (!IsWindowVisible(hwnd)) {
            return TRUE;
        }
        
        wchar_t windowTitle[256];
        wchar_t className[256];
        
        GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));
        GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
        
        QString title = QString::fromWCharArray(windowTitle);
        QString cls = QString::fromWCharArray(className);
        
        if (!title.isEmpty()) {
            DWORD processId;
            GetWindowThreadProcessId(hwnd, &processId);
            
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (hProcess) {
                wchar_t processName[MAX_PATH];
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
                    QString procName = QString::fromWCharArray(processName);
                    QFileInfo fileInfo(procName);
                    procName = fileInfo.baseName();
                    
                    QString windowInfo = QString("标题: %1 | 类名: %2 | 进程: %3")
                                       .arg(title).arg(cls).arg(procName);
                    windowList->append(windowInfo);
                }
                CloseHandle(hProcess);
            }
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));
#endif
    
    return windows;
}

/**
 * @brief 获取窗口信息
 */
QString ConnectionSchemeDialog::getWindowInfo(void* hwnd) const
{
#ifdef Q_OS_WIN
    HWND handle = static_cast<HWND>(hwnd);
    
    wchar_t windowTitle[256];
    wchar_t className[256];
    
    GetWindowTextW(handle, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));
    GetClassNameW(handle, className, sizeof(className) / sizeof(wchar_t));
    
    QString title = QString::fromWCharArray(windowTitle);
    QString cls = QString::fromWCharArray(className);
    
    return QString("标题: %1, 类名: %2").arg(title).arg(cls);
#else
    return "窗口信息获取仅支持Windows平台";
#endif
}