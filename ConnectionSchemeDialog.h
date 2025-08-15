#ifndef CONNECTIONSCHEMEDIALOG_H
#define CONNECTIONSCHEMEDIALOG_H

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
#include <QListWidget>
#include <QListWidgetItem>
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
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

/**
 * @brief 连线方案配置对话框
 * 
 * 功能包括:
 * - 管理多个连线方案（添加、编辑、删除）
 * - 每个方案包含平台信息、窗口识别参数、棋盘区域等
 * - 支持方案的导入导出
 * - 提供方案测试功能
 */
class ConnectionSchemeDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 连线方案结构
     */
    struct ConnectionScheme {
        QString name;                // 方案名称
        QString platformName;        // 平台名称（如"天天象棋"、"QQ象棋"等）
        QString windowClass;         // 窗口类名
        QString windowTitle;         // 窗口标题（支持模糊匹配）
        QString processName;         // 进程名称
        QRect boardArea;            // 棋盘区域（相对于窗口）
        QPoint boardOffset;         // 棋盘偏移
        QSize cellSize;             // 格子大小
        int animationDelay;         // 动画延迟（毫秒）
        bool useChildWindow;        // 是否使用子窗口
        QString description;        // 方案描述
        bool enabled;               // 是否启用
        
        ConnectionScheme() {
            name = "";
            platformName = "";
            windowClass = "";
            windowTitle = "";
            processName = "";
            boardArea = QRect(0, 0, 400, 450);
            boardOffset = QPoint(20, 25);
            cellSize = QSize(44, 50);
            animationDelay = 800;
            useChildWindow = false;
            description = "";
            enabled = true;
        }
        
        // JSON序列化
        QJsonObject toJson() const;
        void fromJson(const QJsonObject& json);
    };

    explicit ConnectionSchemeDialog(QWidget *parent = nullptr);
    ~ConnectionSchemeDialog();
    
    /**
     * @brief 获取所有连线方案
     */
    QList<ConnectionScheme> getAllSchemes() const { return schemes; }
    
    /**
     * @brief 获取当前选中的方案
     */
    ConnectionScheme getCurrentScheme() const;
    
    /**
     * @brief 设置当前方案
     */
    void setCurrentScheme(const ConnectionScheme& scheme);
    
    /**
     * @brief 添加新方案
     */
    void addScheme(const ConnectionScheme& scheme);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    // 方案管理
    void onAddScheme();
    void onEditScheme();
    void onDeleteScheme();
    void onDuplicateScheme();
    void onSchemeSelectionChanged();
    
    // 导入导出
    void onImportSchemes();
    void onExportSchemes();
    void onExportCurrentScheme();
    
    // 测试功能
    void onTestScheme();
    void onCaptureWindow();
    void onDetectWindow();
    
    // 界面更新
    void onSchemeDataChanged();
    void updateSchemeList();
    void updateSchemeDetails();
    
private:
    // 界面初始化
    void setupUI();
    void setupSchemeListWidget();
    void setupSchemeDetailsWidget();
    void setupButtonsWidget();
    QHBoxLayout* createButtonLayout();
    
    // 数据管理
    void loadSchemes();
    void saveSchemes();
    void createDefaultSchemes();
    void syncUIToCurrentScheme();
    void syncCurrentSchemeToUI();
    
    // 工具函数
    void showMessage(const QString& message, bool isError = false);
    QString getSchemesFilePath() const;
    bool validateScheme(const ConnectionScheme& scheme, QString& errorMsg) const;
    
    // 窗口检测
    QStringList detectAvailableWindows();
    QString getWindowInfo(void* hwnd) const;
    
private:
    // 界面控件
    QSplitter* mainSplitter;
    
    // 左侧方案列表
    QListWidget* schemeList;
    QPushButton* addSchemeBtn;
    QPushButton* editSchemeBtn;
    QPushButton* deleteSchemeBtn;
    QPushButton* duplicateSchemeBtn;
    
    // 右侧方案详情
    QLineEdit* nameEdit;
    QLineEdit* platformNameEdit;
    QLineEdit* windowClassEdit;
    QLineEdit* windowTitleEdit;
    QLineEdit* processNameEdit;
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
    QTextEdit* descriptionEdit;
    QCheckBox* enabledCheck;
    
    // 底部按钮
    QPushButton* importBtn;
    QPushButton* exportBtn;
    QPushButton* exportCurrentBtn;
    QPushButton* testSchemeBtn;
    QPushButton* captureWindowBtn;
    QPushButton* detectWindowBtn;
    QPushButton* okBtn;
    QPushButton* cancelBtn;
    
    // 数据
    QList<ConnectionScheme> schemes;
    int currentSchemeIndex;
    bool dataChanged;
    
    // 设置
    QSettings* settings;
};

#endif // CONNECTIONSCHEMEDIALOG_H