#ifndef SYSTEMMANAGEWIDGET_H
#define SYSTEMMANAGEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QProgressBar>

class SystemManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemManageWidget(QWidget *parent = 0);

private slots:
    void onSaveShopInfo();
    void onSaveCashierSettings();
    void onBackupData();
    void onRestoreData();
    void onChangePassword();
    void onChangeTheme(int index);
    void onExportData();
    void onClearLogs();
    void onTestPrint();
    void onResetSettings();

    // 新增槽函数
    void onSaveAllSettings();
    void onImportSettings();
    void onSystemCheck();
    void onUpdateDatabase();
    void resetStatus();

private:
    // UI 创建方法
    void createTabs();
    void createShopInfoTab();
    void createCashierTab();
    void createDataTab();
    void createPersonalTab();
    void createAboutTab();

    // XML 文件操作（直接编写）
    bool saveSettingsToXml();
    bool loadSettingsFromXml();
    void createDefaultXmlConfig();
    QString getConfigPath() const;

    // 辅助方法
    QString escapeXml(const QString &text) const;
    QString unescapeXml(const QString &text) const;
    QString boolToString(bool value) const;
    bool stringToBool(const QString &str, bool defaultValue = false) const;

    // 验证方法
    bool validateShopInfo();
    bool validatePasswordChange();
    void showSuccessMessage(const QString &message);
    void showErrorMessage(const QString &message);

    // ========== 店铺信息控件 ==========
    QLineEdit *shopNameEdit;
    QLineEdit *shopOwnerEdit;
    QLineEdit *phoneEdit;
    QLineEdit *addressEdit;
    QTimeEdit *openTimeEdit;
    QTimeEdit *closeTimeEdit;
    QTextEdit *descriptionEdit;
    QLineEdit *emailEdit;
    QLineEdit *websiteEdit;

    // ========== 收银设置控件 ==========
    QComboBox *printerCombo;
    QComboBox *barcodeCombo;
    QComboBox *paymentCombo;
    QCheckBox *autoPrintCheck;
    QCheckBox *soundCheck;
    QCheckBox *showStockCheck;
    QCheckBox *autoDiscountCheck;
    QCheckBox *memberDiscountCheck;
    QSpinBox *timeoutSpin;
    QSpinBox *receiptWidthSpin;
    QSpinBox *vatRateSpin;
    QLineEdit *footerTextEdit;
    QLineEdit *headerTextEdit;

    // ========== 数据管理控件 ==========
    QPushButton *backupButton;
    QPushButton *restoreButton;
    QPushButton *exportButton;
    QPushButton *clearLogsButton;
    QPushButton *optimizeButton;
    QLabel *backupInfoLabel;
    QLabel *dbSizeLabel;
    QLabel *lastBackupLabel;
    QLabel *recordCountLabel;
    QProgressBar *storageBar;

    // ========== 个人设置控件 ==========
    QLineEdit *oldPassEdit;
    QLineEdit *newPassEdit;
    QLineEdit *confirmPassEdit;
    QComboBox *themeCombo;
    QComboBox *languageCombo;
    QComboBox *fontSizeCombo;
    QCheckBox *shortcutCheck;
    QCheckBox *autoLoginCheck;
    QCheckBox *notificationCheck;
    QCheckBox *rememberLastCheck;
    QPushButton *resetButton;
    QPushButton *testNotifyButton;

    // ========== 关于信息控件 ==========
    QLabel *versionLabel;
    QLabel *copyrightLabel;
    QLabel *developerLabel;
    QTextEdit *licenseText;
    QPushButton *checkUpdateButton;
    QPushButton *helpButton;

    // ========== 主界面控件 ==========
    QTabWidget *tabWidget;
    QWidget *shopInfoTab;
    QWidget *cashierTab;
    QWidget *dataTab;
    QWidget *personalTab;
    QWidget *aboutTab;

    // ========== 工具栏控件 ==========
    QPushButton *saveAllButton;
    QPushButton *loadButton;
    QPushButton *defaultButton;
    QLabel *statusLabel;

    // 配置常量
    static const QString CONFIG_FILENAME;
    static const QColor PRIMARY_COLOR;
    static const QColor SUCCESS_COLOR;
    static const QColor WARNING_COLOR;
    static const QColor DANGER_COLOR;
};

#endif // SYSTEMMANAGEWIDGET_H
