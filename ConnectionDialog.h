#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTabWidget>
#include <QListWidget>
#include <QSplitter>
#include <QProgressBar>
#include <QTimer>
#include <QPixmap>
#include <QScrollArea>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include "PlatformConnector.h"
#include "ConnectionSchemeDialog.h"

/**
 * @brief 连线配置对话框
 * 
 * 功能包括:
 * - 平台选择和配置
 * - 窗口查找和连接测试
 * - 坐标校准和点击测试
 * - 连接状态监控
 * - 配置导入导出
 */
class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();
    
    /**
     * @brief 获取平台连接器
     */
    PlatformConnector* getPlatformConnector() const { return connector; }
    
    /**
     * @brief 设置连接器（外部创建）
     */
    void setPlatformConnector(PlatformConnector* platformConnector);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    // 平台管理
    void onPlatformChanged();
    void onAddCustomPlatform();
    void onEditCustomPlatform();
    void onDeleteCustomPlatform();
    void onImportPlatforms();
    void onExportPlatforms();
    
    // 连线方案管理
    void onManageConnectionSchemes();
    void onApplyConnectionScheme();
    void onConnectionSchemeSelected();
    void onRefreshConnectionSchemes();
    
    // 连接管理
    void onConnectToGame();
    void onDisconnectFromGame();
    void onRefreshWindows();
    void onTestConnection();
    
    // 窗口操作
    void onCaptureWindow();
    void onCaptureBoardArea();
    void onWindowSelected();
    
    // 坐标校准
    void onCalibrateBoard();
    void onTestClick();
    void onBoardAreaChanged();
    void onCellSizeChanged();
    void onOffsetChanged();
    
    // 连接器事件
    void onConnectionStatusChanged(PlatformConnector::ConnectionStatus status);
    void onPlatformChangedFromConnector(PlatformConnector::PlatformType platform);
    void onErrorOccurred(const QString& error);
    void onDebugMessage(const QString& message);
    void onGameWindowFound(const QString& windowTitle);
    void onClickCompleted(int row, int col);
    
    // 界面更新
    void updateUI();
    void updateConnectionStatus();
    void updatePlatformConfig();
    void updateWindowList();
    void updatePreview();
    
    // 定时器
    void onPreviewTimer();
    void onStatusTimer();
    
    // 标签页切换
    void onTabChanged(int index);

private:
    // 界面初始化
    void setupUI();
    void setupPlatformTab();
    void setupConnectionTab();
    void setupCalibrationTab();
    void setupDebugTab();
    
    // 控件创建
    QWidget* createPlatformConfigWidget();
    QWidget* createWindowSelectionWidget();
    QWidget* createCalibrationWidget();
    QWidget* createPreviewWidget();
    QWidget* createDebugWidget();
    
    // 数据同步
    void syncConfigToUI();
    void syncUIToConfig();
    PlatformConnector::PlatformConfig getCurrentConfig() const;
    void setCurrentConfig(const PlatformConnector::PlatformConfig& config);
    
    // 窗口管理
    void findAllWindows();
    QString getWindowInfo(void* hwnd) const;
    
    // 预览功能
    void updatePreviewImage();
    void drawBoardGrid(QPixmap& pixmap);
    void drawClickPosition(QPixmap& pixmap, int row, int col);
    
    // 工具函数
    void showMessage(const QString& message, bool isError = false);
    void logMessage(const QString& message);
    bool validateConfig() const;
    void resetToDefaults();

private:
    // 核心组件
    PlatformConnector* connector;
    bool ownConnector; // 是否拥有连接器对象
    
    // 主界面
    QTabWidget* tabWidget;
    
    // 平台配置标签页
    QWidget* platformTab;
    QComboBox* connectionSchemeCombo;
    QPushButton* refreshSchemesBtn;
    QComboBox* platformCombo;
    QLineEdit* platformNameEdit;
    QLineEdit* windowClassEdit;
    QLineEdit* windowTitleEdit;
    QLineEdit* clickClassEdit;
    QLineEdit* clickTitleEdit;
    QCheckBox* useChildWindowCheck;
    QPushButton* addPlatformBtn;
    QPushButton* editPlatformBtn;
    QPushButton* deletePlatformBtn;
    QPushButton* importBtn;
    QPushButton* exportBtn;
    QPushButton* manageSchemesBtn;
    QPushButton* applySchemesBtn;
    
    // 连接配置标签页
    QWidget* connectionTab;
    QListWidget* windowList;
    QPushButton* refreshWindowsBtn;
    QPushButton* connectBtn;
    QPushButton* disconnectBtn;
    QPushButton* testConnectionBtn;
    QLabel* connectionStatusLabel;
    QProgressBar* connectionProgress;
    
    // 坐标校准标签页
    QWidget* calibrationTab;
    QSpinBox* boardXSpin;
    QSpinBox* boardYSpin;
    QSpinBox* boardWidthSpin;
    QSpinBox* boardHeightSpin;
    QSpinBox* offsetXSpin;
    QSpinBox* offsetYSpin;
    QSpinBox* cellWidthSpin;
    QSpinBox* cellHeightSpin;
    QSpinBox* animationDelaySpin;
    QPushButton* calibrateBoardBtn;
    QPushButton* testClickBtn;
    QSpinBox* testRowSpin;
    QSpinBox* testColSpin;
    
    // 预览区域
    QScrollArea* previewArea;
    QLabel* previewLabel;
    QPushButton* captureWindowBtn;
    QPushButton* captureBoardBtn;
    QCheckBox* showGridCheck;
    QCheckBox* autoUpdateCheck;
    QSlider* previewScaleSlider;
    
    // 调试标签页
    QWidget* debugTab;
    QTextEdit* debugOutput;
    QCheckBox* debugModeCheck;
    QPushButton* clearDebugBtn;
    QPushButton* saveDebugBtn;
    QLabel* debugInfoLabel;
    
    // 定时器
    QTimer* previewTimer;
    QTimer* statusTimer;
    
    // 状态
    QPixmap currentPreview;
    bool isCalibrating;
    QPoint lastClickPos;
    
    // 常量
    static const int PREVIEW_UPDATE_INTERVAL;
    static const int STATUS_UPDATE_INTERVAL;
    static const int MAX_PREVIEW_WIDTH;
    static const int MAX_PREVIEW_HEIGHT;
};

/**
 * @brief 自定义平台配置对话框
 */
class CustomPlatformDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CustomPlatformDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 设置平台配置
     */
    void setPlatformConfig(const QString& name, const PlatformConnector::PlatformConfig& config);
    
    /**
     * @brief 获取平台配置
     */
    QString getPlatformName() const;
    PlatformConnector::PlatformConfig getPlatformConfig() const;
    
private slots:
    void onAccept();
    void onDetectWindow();
    void onTestConfig();
    
private:
    void setupUI();
    bool validateInput() const;
    
private:
    QLineEdit* nameEdit;
    QLineEdit* windowClassEdit;
    QLineEdit* windowTitleEdit;
    QLineEdit* clickClassEdit;
    QLineEdit* clickTitleEdit;
    QSpinBox* boardXSpin;
    QSpinBox* boardYSpin;
    QSpinBox* boardWidthSpin;
    QSpinBox* boardHeightSpin;
    QSpinBox* offsetXSpin;
    QSpinBox* offsetYSpin;
    QSpinBox* cellWidthSpin;
    QSpinBox* cellHeightSpin;
    QSpinBox* animationDelaySpin;
    QCheckBox* useChildWindowCheck;
    QPushButton* detectBtn;
    QPushButton* testBtn;
};

/**
 * @brief 窗口选择对话框
 */
class WindowSelectorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WindowSelectorDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 获取选中的窗口信息
     */
    QString getSelectedWindowClass() const;
    QString getSelectedWindowTitle() const;
    
private slots:
    void onRefresh();
    void onWindowSelected();
    void onAccept();
    
private:
    void setupUI();
    void updateWindowList();
    QString getWindowInfo(void* hwnd) const;
    
private:
    QListWidget* windowList;
    QPushButton* refreshBtn;
    QLabel* infoLabel;
    
    QString selectedClass;
    QString selectedTitle;
};

#endif // CONNECTIONDIALOG_H