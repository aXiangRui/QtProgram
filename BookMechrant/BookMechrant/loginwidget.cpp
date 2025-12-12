#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QSettings>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QGraphicsDropShadowEffect>
#include <QDesktopWidget>
#include <QApplication>
#include <QFontDatabase>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent), m_opacity(1.0), bgOffset(0)
{
    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载字体（如果可用）
    int fontId = QFontDatabase::addApplicationFont(":/fonts/MicrosoftYaHei.ttf");
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QApplication::setFont(QFont(fontFamily, 9));
    }

    createBackground();
    createUI();
    applyStyle();
    createAnimations();
    loadSettings();

    // 如果设置了自动登录，延迟执行
    if (autoLogin && !lastUsername.isEmpty()) {
        QTimer::singleShot(1000, this, SLOT(onLoginClicked()));
    }
}

LoginWidget::~LoginWidget()
{
    saveSettings();
}

void LoginWidget::createUI()
{
    // 主容器
    mainContainer = new QWidget;
    mainContainer->setObjectName("mainContainer");

    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 标题栏 ==========
    QWidget *headerWidget = new QWidget;
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(60);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    logoLabel = new QLabel("📚");
    logoLabel->setObjectName("logoLabel");

    QWidget *titleWidget = new QWidget;
    QVBoxLayout *titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setContentsMargins(10, 0, 0, 0);
    titleLayout->setSpacing(2);

    titleLabel = new QLabel("图书商家管理系统");
    titleLabel->setObjectName("titleLabel");

    subtitleLabel = new QLabel("Book Merchant Management System");
    subtitleLabel->setObjectName("subtitleLabel");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(titleWidget);
    headerLayout->addStretch();

    versionLabel = new QLabel("v3.0");
    versionLabel->setObjectName("versionLabel");
    headerLayout->addWidget(versionLabel);

    // ========== 登录表单 ==========
    loginBox = new QGroupBox;
    loginBox->setObjectName("loginBox");

    QVBoxLayout *boxLayout = new QVBoxLayout(loginBox);
    boxLayout->setContentsMargins(30, 30, 30, 30);
    boxLayout->setSpacing(20);

    // 表单布局
    QFormLayout *formLayout = new QFormLayout;
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    usernameEdit = new QLineEdit;
    usernameEdit->setObjectName("usernameEdit");
    usernameEdit->setPlaceholderText("请输入用户名/手机号/邮箱");
    usernameEdit->setMinimumHeight(40);
    usernameEdit->setMaxLength(50);

    QWidget *passwordWidget = new QWidget;
    QHBoxLayout *passLayout = new QHBoxLayout(passwordWidget);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(5);

    passwordEdit = new QLineEdit;
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(40);

    eyeButton = new QPushButton("👁");
    eyeButton->setObjectName("eyeButton");
    eyeButton->setFixedSize(40, 40);
    eyeButton->setCheckable(true);

    passLayout->addWidget(passwordEdit);
    passLayout->addWidget(eyeButton);

    // 服务器选择
    serverCombo = new QComboBox;
    serverCombo->setObjectName("serverCombo");
    serverCombo->addItems(QStringList()
        << "本地服务器 (127.0.0.1:8080)"
        << "测试服务器 (test.bookstore.com)"
        << "正式服务器 (api.bookstore.com)");
    serverCombo->setMinimumHeight(35);

    // 选项
    QWidget *optionWidget = new QWidget;
    QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);
    optionLayout->setContentsMargins(0, 0, 0, 0);

    rememberCheck = new QCheckBox("记住密码");
    rememberCheck->setObjectName("rememberCheck");
    autoLoginCheck = new QCheckBox("自动登录");
    autoLoginCheck->setObjectName("autoLoginCheck");

    optionLayout->addWidget(rememberCheck);
    optionLayout->addStretch();
    optionLayout->addWidget(autoLoginCheck);

    formLayout->addRow("账号:", usernameEdit);
    formLayout->addRow("密码:", passwordWidget);
    formLayout->addRow("服务器:", serverCombo);

    // 按钮区域
    QWidget *buttonWidget = new QWidget;
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setSpacing(10);

    loginButton = new QPushButton("登录系统");
    loginButton->setObjectName("loginButton");
    loginButton->setMinimumHeight(45);

    QWidget *linkWidget = new QWidget;
    QHBoxLayout *linkLayout = new QHBoxLayout(linkWidget);
    linkLayout->setContentsMargins(0, 0, 0, 0);

    registerButton = new QPushButton("注册账号");
    registerButton->setObjectName("registerButton");

    forgotButton = new QPushButton("忘记密码?");
    forgotButton->setObjectName("forgotButton");

    linkLayout->addWidget(registerButton);
    linkLayout->addStretch();
    linkLayout->addWidget(forgotButton);

    // 消息标签
    errorLabel = new QLabel;
    errorLabel->setObjectName("errorLabel");
    errorLabel->hide();

    successLabel = new QLabel;
    successLabel->setObjectName("successLabel");
    successLabel->hide();

    // 组装登录框
    boxLayout->addLayout(formLayout);
    boxLayout->addWidget(optionWidget);
    boxLayout->addSpacing(10);
    boxLayout->addWidget(loginButton);
    boxLayout->addWidget(linkWidget);
    boxLayout->addWidget(errorLabel);
    boxLayout->addWidget(successLabel);

    // ========== 页脚 ==========
    footerLabel = new QLabel(QString("© 2023 图书商家管理系统 | 当前时间: %1")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
    footerLabel->setObjectName("footerLabel");
    footerLabel->setAlignment(Qt::AlignCenter);

    // ========== 组装主布局 ==========
    QWidget *centerWidget = new QWidget;
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(40, 40, 40, 40);
    centerLayout->setSpacing(30);

    centerLayout->addWidget(headerWidget);
    centerLayout->addWidget(loginBox);
    centerLayout->addStretch();
    centerLayout->addWidget(footerLabel);

    mainLayout->addWidget(centerWidget);


    // 设置主布局
    QHBoxLayout *finalLayout = new QHBoxLayout(this);
    finalLayout->setContentsMargins(20, 20, 20, 20);
    finalLayout->addStretch();
    finalLayout->addWidget(mainContainer);
    finalLayout->addStretch();

    // 连接信号
    connect(loginButton, SIGNAL(clicked()), this, SLOT(onLoginClicked()));
    connect(registerButton, SIGNAL(clicked()), this, SLOT(onRegisterClicked()));
    connect(forgotButton, SIGNAL(clicked()), this, SLOT(onForgotPasswordClicked()));
    connect(eyeButton, SIGNAL(clicked()), this, SLOT(onEyeButtonClicked()));
    connect(rememberCheck, SIGNAL(stateChanged(int)), this, SLOT(onRememberChanged(int)));
    connect(serverCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onServerChanged(int)));
    connect(usernameEdit, SIGNAL(returnPressed()), this, SLOT(onLoginClicked()));
    connect(passwordEdit, SIGNAL(returnPressed()), this, SLOT(onLoginClicked()));

    // 启动背景动画定时器
    bgTimer = new QTimer(this);
    connect(bgTimer, SIGNAL(timeout()), this, SLOT(update()));
    bgTimer->start(50);
}

void LoginWidget::applyStyle()
{
    QString styleSheet = QString(R"(
        #mainContainer {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 %1,
                stop:0.5 %2,
                stop:1 %3);
            border-radius: 20px;
            border: 1px solid rgba(255, 255, 255, 0.3);
        }

        #headerWidget {
            background: transparent;
        }

        #logoLabel {
            font-size: 28px;
            color: white;
        }

        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: white;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }

        #subtitleLabel {
            font-size: 12px;
            color: rgba(255, 255, 255, 0.8);
            font-family: 'Segoe UI', Arial;
        }

        #versionLabel {
            font-size: 11px;
            color: rgba(255, 255, 255, 0.6);
            padding: 4px 10px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 10px;
        }

        #loginBox {
            background: white;
            border-radius: 15px;
            border: 1px solid rgba(0, 0, 0, 0.1);
        }

        QGroupBox#loginBox {
            font-size: 14px;
            font-weight: bold;
            color: #333;
        }

        QLineEdit {
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 14px;
            background: #fafafa;
            selection-background-color: %4;
        }

        QLineEdit:focus {
            border-color: %4;
            background: white;
        }

        QLineEdit#usernameEdit {
            background-image: url(:/icons/user.png);
            background-position: right 10px center;
            background-repeat: no-repeat;
            padding-right: 40px;
        }

        QLineEdit#passwordEdit {
            background-image: url(:/icons/lock.png);
            background-position: right 10px center;
            background-repeat: no-repeat;
            padding-right: 40px;
        }

        #eyeButton {
            background: transparent;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 16px;
            color: #666;
        }

        #eyeButton:hover {
            background: #f5f5f5;
        }

        #eyeButton:checked {
            color: %4;
            border-color: %4;
        }

        QComboBox {
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px 15px;
            background: #fafafa;
            font-size: 13px;
        }

        QComboBox:focus {
            border-color: %4;
        }

        QComboBox::drop-down {
            border: none;
            width: 30px;
        }

        QComboBox::down-arrow {
            image: url(:/icons/down.png);
            width: 12px;
            height: 12px;
        }

        QCheckBox {
            font-size: 13px;
            color: #666;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #ccc;
            border-radius: 4px;
        }

        QCheckBox::indicator:checked {
            background-color: %4;
            border-color: %4;
            image: url(:/icons/check.png);
        }

        #loginButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %4,
                stop:1 %5);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
            padding: 12px;
        }

        #loginButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %6,
                stop:1 %7);
        }

        #loginButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %8,
                stop:1 %9);
        }

        #loginButton:disabled {
            background: #cccccc;
        }

        #registerButton, #forgotButton {
            background: transparent;
            border: none;
            color: %4;
            font-size: 13px;
            padding: 5px;
            text-decoration: underline;
        }

        #registerButton:hover, #forgotButton:hover {
            color: %6;
        }

        #errorLabel {
            color: %10;
            background-color: rgba(231, 76, 60, 0.1);
            border: 1px solid rgba(231, 76, 60, 0.3);
            border-radius: 6px;
            padding: 10px 15px;
            font-size: 13px;
        }

        #successLabel {
            color: #27ae60;
            background-color: rgba(46, 204, 113, 0.1);
            border: 1px solid rgba(46, 204, 113, 0.3);
            border-radius: 6px;
            padding: 10px 15px;
            font-size: 13px;
        }

        #footerLabel {
            color: rgba(255, 255, 255, 0.7);
            font-size: 12px;
            padding: 10px;
        }

        QLabel[accessibleName="formLabel"] {
            font-weight: bold;
            color: #555;
            min-width: 80px;
        }
    )").arg(PRIMARY_COLOR.name())
      .arg(SECONDARY_COLOR.name())
      .arg(PRIMARY_COLOR.darker(110).name())
      .arg(PRIMARY_COLOR.name())
      .arg(SECONDARY_COLOR.name())
      .arg(PRIMARY_COLOR.darker(120).name())
      .arg(SECONDARY_COLOR.darker(120).name())
      .arg(PRIMARY_COLOR.darker(130).name())
      .arg(SECONDARY_COLOR.darker(130).name())
      .arg(WARNING_COLOR.name());

    setStyleSheet(styleSheet);
    // 添加阴影效果
       QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(mainContainer);
       shadow->setBlurRadius(25);
       shadow->setColor(QColor(0, 0, 0, 80));
       shadow->setOffset(0, 5);
       mainContainer->setGraphicsEffect(shadow);
   }

   void LoginWidget::createAnimations()
   {
       // 渐入动画
       fadeAnimation = new QPropertyAnimation(this, "opacity");
       fadeAnimation->setDuration(800);
       fadeAnimation->setStartValue(0.0);
       fadeAnimation->setEndValue(1.0);
       fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);

       // 登录按钮动画
       loginAnimation = new QParallelAnimationGroup(this);

       QPropertyAnimation *pulseAnim = new QPropertyAnimation(loginButton, "geometry");
       pulseAnim->setDuration(200);
       pulseAnim->setKeyValueAt(0, loginButton->geometry());
       pulseAnim->setKeyValueAt(0.5, loginButton->geometry().adjusted(-5, -5, 5, 5));
       pulseAnim->setKeyValueAt(1, loginButton->geometry());

       loginAnimation->addAnimation(pulseAnim);
   }

   void LoginWidget::createBackground()
   {
       // 设置窗口大小和居中
       QDesktopWidget *desktop = QApplication::desktop();
       QRect screenRect = desktop->availableGeometry();
       setFixedSize(screenRect.width() * 0.4, screenRect.height() * 0.7);

       // 计算居中位置
       int x = (screenRect.width() - width()) / 2;
       int y = (screenRect.height() - height()) / 2;
       move(x, y);
   }

   void LoginWidget::paintEvent(QPaintEvent *event)
   {
       Q_UNUSED(event);

       QPainter painter(this);
       painter.setRenderHint(QPainter::Antialiasing);

       // 绘制动态背景
       QLinearGradient gradient(0, 0, width(), height());
       gradient.setColorAt(0, QColor(41, 128, 185));
       gradient.setColorAt(0.5, QColor(52, 152, 219));
       gradient.setColorAt(1, QColor(41, 128, 185).darker(110));

       painter.fillRect(rect(), gradient);

       // 绘制动态粒子
       painter.setPen(Qt::NoPen);
       painter.setBrush(QColor(255, 255, 255, 30));

       int particleCount = 50;
       for (int i = 0; i < particleCount; i++) {
           int x = (qrand() + bgOffset * i) % width();
           int y = (qrand() + bgOffset * (i + 1)) % height();
           int size = 2 + (qrand() % 4);
           painter.drawEllipse(x, y, size, size);
       }

       bgOffset++;
       if (bgOffset > 1000) bgOffset = 0;
   }

   void LoginWidget::loadSettings()
   {
       QSettings settings("BookMerchant", "Login");

       rememberPassword = settings.value("RememberPassword", false).toBool();
       autoLogin = settings.value("AutoLogin", false).toBool();
       lastUsername = settings.value("LastUsername", "").toString();
       lastServer = settings.value("LastServer", "本地服务器 (127.0.0.1:8080)").toString();

       rememberCheck->setChecked(rememberPassword);
       autoLoginCheck->setChecked(autoLogin);

       if (!lastUsername.isEmpty()) {
           usernameEdit->setText(lastUsername);
       }

       int serverIndex = serverCombo->findText(lastServer);
       if (serverIndex >= 0) {
           serverCombo->setCurrentIndex(serverIndex);
       }

       if (rememberPassword) {
           QString savedPassword = settings.value("Password", "").toString();
           // 这里应该解密密码，简单示例直接显示
           passwordEdit->setText(savedPassword);
       }
   }

   void LoginWidget::saveSettings()
   {
       QSettings settings("BookMerchant", "Login");

       settings.setValue("RememberPassword", rememberCheck->isChecked());
       settings.setValue("AutoLogin", autoLoginCheck->isChecked());
       settings.setValue("LastUsername", usernameEdit->text());
       settings.setValue("LastServer", serverCombo->currentText());

       if (rememberCheck->isChecked()) {
           // 这里应该加密密码，简单示例直接保存
           settings.setValue("Password", passwordEdit->text());
       } else {
           settings.remove("Password");
       }
   }

   void LoginWidget::onLoginClicked()
   {
       QString username = usernameEdit->text().trimmed();
       QString password = passwordEdit->text();

       // 验证输入
       if (username.isEmpty()) {
           showError("请输入用户名");
           usernameEdit->setFocus();
           shakeWindow();
           return;
       }

       if (password.isEmpty()) {
           showError("请输入密码");
           passwordEdit->setFocus();
           shakeWindow();
           return;
       }

       // 隐藏错误信息
       errorLabel->hide();

       // 禁用登录按钮
       loginButton->setEnabled(false);
       loginButton->setText("登录中...");

       // 播放按钮动画
       loginAnimation->start();

       // 保存设置
       saveSettings();

       // 模拟网络请求延迟
       QTimer::singleShot(1500, this, [this, username, password]() {
           // 模拟登录验证（实际项目中应该连接服务器）
           if (username == "admin" && password == "admin123") {
               showSuccess("登录成功！正在进入系统...");
               QTimer::singleShot(1000, this, SLOT(onLoginSuccess()));
           } else if (username == "demo" && password == "demo123") {
               showSuccess("演示账号登录成功！");
               QTimer::singleShot(1000, this, SLOT(onLoginSuccess()));
           } else {
               onLoginFailed("用户名或密码错误");
           }
       });
   }
   void LoginWidget::onLoginSuccess()
   {
       loginButton->setEnabled(true);
       loginButton->setText("登录系统");

       // 发送登录成功信号
       emit loginSuccess();
   }

   void LoginWidget::onLoginFailed(const QString &error)
   {
       loginButton->setEnabled(true);
       loginButton->setText("登录系统");

       showError(error);
       shakeWindow();
   }

   void LoginWidget::onRememberChanged(int state)
   {
       rememberPassword = (state == Qt::Checked);
       if (!rememberPassword) {
           autoLoginCheck->setChecked(false);
       }
   }

   void LoginWidget::onServerChanged(int index)
   {
       Q_UNUSED(index);
       saveSettings();
   }

   void LoginWidget::onRegisterClicked()
   {
       emit registerRequested();

       // 显示注册提示
       QMessageBox::information(this, "注册账号",
           "请联系系统管理员获取注册权限\n\n"
           "管理员电话: 400-1234-5678\n"
           "工作时间: 9:00-18:00");
   }

   void LoginWidget::onForgotPasswordClicked()
   {
       emit forgotPassword();

       // 显示忘记密码对话框
       QDialog dialog(this);
       dialog.setWindowTitle("找回密码");
       dialog.setFixedSize(400, 300);

       QVBoxLayout *layout = new QVBoxLayout(&dialog);

       QLabel *titleLabel = new QLabel("🔐 密码找回");
       titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
       titleLabel->setAlignment(Qt::AlignCenter);

       QLineEdit *emailEdit = new QLineEdit;
       emailEdit->setPlaceholderText("请输入注册邮箱");
       emailEdit->setMinimumHeight(40);

       QPushButton *sendButton = new QPushButton("发送重置链接");
       sendButton->setStyleSheet(QString(
           "QPushButton { background-color: %1; color: white; border-radius: 6px; padding: 10px; }"
           "QPushButton:hover { background-color: %2; }"
       ).arg(PRIMARY_COLOR.name()).arg(PRIMARY_COLOR.darker(120).name()));

       QLabel *infoLabel = new QLabel(
           "请输入您注册时使用的邮箱地址，\n"
           "系统将发送密码重置链接到您的邮箱。\n\n"
           "如果无法收到邮件，请联系客服。");
       infoLabel->setWordWrap(true);
       infoLabel->setAlignment(Qt::AlignCenter);

       layout->addWidget(titleLabel);
       layout->addSpacing(20);
       layout->addWidget(emailEdit);
       layout->addWidget(sendButton);
       layout->addSpacing(20);
       layout->addWidget(infoLabel);

       dialog.exec();
   }

   void LoginWidget::onEyeButtonClicked()
   {
       if (eyeButton->isChecked()) {
           passwordEdit->setEchoMode(QLineEdit::Normal);
           eyeButton->setText("👁");
       } else {
           passwordEdit->setEchoMode(QLineEdit::Password);
           eyeButton->setText("👁");
       }
   }

   void LoginWidget::shakeWindow()
   {
       QPropertyAnimation *shakeAnim = new QPropertyAnimation(this, "pos");
       shakeAnim->setDuration(200);
       shakeAnim->setKeyValueAt(0, pos());
       shakeAnim->setKeyValueAt(0.2, pos() + QPoint(10, 0));
       shakeAnim->setKeyValueAt(0.4, pos() + QPoint(-10, 0));
       shakeAnim->setKeyValueAt(0.6, pos() + QPoint(10, 0));
       shakeAnim->setKeyValueAt(0.8, pos() + QPoint(-10, 0));
       shakeAnim->setKeyValueAt(1, pos());
       shakeAnim->start(QAbstractAnimation::DeleteWhenStopped);
   }

   void LoginWidget::showError(const QString &message)
   {
       errorLabel->setText("❌ " + message);
       errorLabel->show();
       successLabel->hide();

       QTimer::singleShot(5000, errorLabel, SLOT(hide()));
   }

   void LoginWidget::showSuccess(const QString &message)
   {
       successLabel->setText("✅ " + message);
       successLabel->show();
       errorLabel->hide();
   }

   void LoginWidget::showWelcomeMessage()
   {
       QTime currentTime = QTime::currentTime();
       QString greeting;

       if (currentTime.hour() < 6) {
           greeting = "夜深了，请注意休息";
       } else if (currentTime.hour() < 9) {
           greeting = "早上好！";
       } else if (currentTime.hour() < 12) {
           greeting = "上午好！";
       } else if (currentTime.hour() < 14) {
           greeting = "中午好！";
       } else if (currentTime.hour() < 18) {
           greeting = "下午好！";
       } else if (currentTime.hour() < 22) {
           greeting = "晚上好！";
       } else {
           greeting = "夜深了，请注意休息";
       }

       successLabel->setText(greeting + " 欢迎回来！");
       successLabel->show();

       QTimer::singleShot(3000, successLabel, SLOT(hide()));
   }

   void LoginWidget::fadeIn()
   {
       fadeAnimation->start();
   }
