#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QLinearGradient>
#include <QDialog>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QClipboard>
#include <QScrollArea>
#include <QStackedWidget>
#include <QProgressBar>

class LoginWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity) { m_opacity = opacity; update(); }

signals:
    void loginRequested(const QString &username, const QString &password);
    void loginSuccess();
    void registerRequested();
    void forgotPassword();

private slots:
    void onLoginClicked();
    void onLoginSuccess();
    void onLoginFailed(const QString &error);
    void onRememberChanged(int state);
    void onServerChanged(int index);
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onEyeButtonClicked();
    void shakeWindow();
    void showWelcomeMessage();
    void fadeIn();

    // 注册相关槽函数
    void onRegisterSubmit();
    void onRegisterReset();
    void onRegisterFileSelect();

    // 忘记密码相关槽函数
    void onForgotSendCode();
    void onForgotSubmit();
    void onForgotCopyPhone();
    void updateCountdown();

private:
    void createUI();
    void applyStyle();
    void createAnimations();
    void createBackground();
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void loadSettings();
    void saveSettings();

    // 绘图事件
    void paintEvent(QPaintEvent *event) override;

    // 窗口大小变化事件
    void resizeEvent(QResizeEvent *event) override;

    // 创建全屏对话框的辅助函数
    QDialog* createFullscreenDialog(const QString &title);
    void setupDialogStyle(QDialog *dialog);

    // 主题颜色
    const QColor PRIMARY_COLOR = QColor(41, 128, 185);
    const QColor SECONDARY_COLOR = QColor(52, 152, 219);
    const QColor ACCENT_COLOR = QColor(46, 204, 113);
    const QColor WARNING_COLOR = QColor(231, 76, 60);
    const QColor BG_COLOR = QColor(245, 247, 250);

    // UI组件
    QWidget *mainContainer;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *logoLabel;
    QLabel *versionLabel;
    QGroupBox *loginBox;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *eyeButton;
    QCheckBox *rememberCheck;
    QCheckBox *autoLoginCheck;
    QComboBox *serverCombo;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *forgotButton;
    QLabel *errorLabel;
    QLabel *successLabel;
    QLabel *footerLabel;

    // 动画相关
    QPropertyAnimation *fadeAnimation;
    QPropertyAnimation *slideAnimation;
    QParallelAnimationGroup *loginAnimation;
    qreal m_opacity;

    // 背景相关
    QTimer *bgTimer;
    int bgOffset;

    // 设置
    bool rememberPassword;
    bool autoLogin;
    QString lastUsername;
    QString lastServer;

    // 忘记密码倒计时
    QTimer *countdownTimer;
    int countdownSeconds;
    QPushButton *sendCodeButton;
};

#endif // LOGINWIDGET_H
