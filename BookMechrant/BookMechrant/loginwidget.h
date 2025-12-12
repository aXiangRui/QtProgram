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

class LoginWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit LoginWidget(QWidget *parent = 0);
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

private:
    void createUI();
    void applyStyle();
    void createAnimations();
    void createBackground();
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void loadSettings();
    void saveSettings();
    void paintEvent(QPaintEvent *event);

    // 主题颜色
    const QColor PRIMARY_COLOR = QColor(41, 128, 185);     // 主蓝
    const QColor SECONDARY_COLOR = QColor(52, 152, 219);   // 次蓝
    const QColor ACCENT_COLOR = QColor(46, 204, 113);      // 成功绿
    const QColor WARNING_COLOR = QColor(231, 76, 60);      // 错误红
    const QColor BG_COLOR = QColor(245, 247, 250);         // 背景灰

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
};

#endif // LOGINWIDGET_H
