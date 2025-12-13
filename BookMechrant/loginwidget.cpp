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
#include <QScrollArea>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QClipboard>
#include <QStackedWidget>
#include <QProgressBar>
#include <QScreen>
#include <QGuiApplication>
#include <QGridLayout>
#include <QSpacerItem>
#include <QStyle>
#include <QStyleOption>
#include <QDebug>

// ========== 构造函数 ==========
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent), m_opacity(1.0), bgOffset(0), countdownSeconds(0)
{
    // 设置窗口属性 - 全屏模式
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 获取屏幕尺寸并设置全屏
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        setGeometry(screenGeometry);
    } else {
        // 备用方案
        QDesktopWidget *desktop = QApplication::desktop();
        if (desktop) {
            QRect screenRect = desktop->screenGeometry();
            setGeometry(screenRect);
        }
    }

    // 加载字体
    int fontId = QFontDatabase::addApplicationFont(":/fonts/MicrosoftYaHei.ttf");
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QApplication::setFont(QFont(fontFamily, 10));
    }

    createBackground();
    createUI();
    applyStyle();
    createAnimations();
    loadSettings();

    // 倒计时定时器
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &LoginWidget::updateCountdown);

    // 自动登录
    if (autoLogin && !lastUsername.isEmpty()) {
        QTimer::singleShot(1000, this, &LoginWidget::onLoginClicked);
    }
}

LoginWidget::~LoginWidget()
{
    saveSettings();
}

// ========== UI创建 ==========
void LoginWidget::createUI()
{
    // 主容器
    mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 标题栏 ==========
    QWidget *headerWidget = new QWidget(mainContainer);
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(100);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(40, 0, 40, 0);
    headerLayout->setSpacing(20);

    // Logo
    logoLabel = new QLabel("📚", headerWidget);
    logoLabel->setObjectName("logoLabel");
    logoLabel->setFixedSize(60, 60);

    // 标题区域
    QWidget *titleWidget = new QWidget(headerWidget);
    QVBoxLayout *titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 10, 0, 10);
    titleLayout->setSpacing(5);

    titleLabel = new QLabel("图书商家管理系统", titleWidget);
    titleLabel->setObjectName("titleLabel");

    subtitleLabel = new QLabel("Book Merchant Management System", titleWidget);
    subtitleLabel->setObjectName("subtitleLabel");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(titleWidget);
    headerLayout->addStretch();

    // 版本号
    versionLabel = new QLabel("v3.5.0", headerWidget);
    versionLabel->setObjectName("versionLabel");
    headerLayout->addWidget(versionLabel);

    // ========== 登录表单 ==========
    loginBox = new QGroupBox(mainContainer);
    loginBox->setObjectName("loginBox");
    loginBox->setMinimumWidth(500);
    loginBox->setMaximumWidth(600);

    QVBoxLayout *boxLayout = new QVBoxLayout(loginBox);
    boxLayout->setContentsMargins(50, 50, 50, 50);
    boxLayout->setSpacing(30);

    // 表单布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(20);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // 用户名
    usernameEdit = new QLineEdit(loginBox);
    usernameEdit->setObjectName("usernameEdit");
    usernameEdit->setPlaceholderText("请输入用户名/手机号/邮箱");
    usernameEdit->setMinimumHeight(50);
    usernameEdit->setMaxLength(50);

    // 密码
    QWidget *passwordWidget = new QWidget(loginBox);
    QHBoxLayout *passLayout = new QHBoxLayout(passwordWidget);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(10);

    passwordEdit = new QLineEdit(passwordWidget);
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(50);

    eyeButton = new QPushButton("👁", passwordWidget);
    eyeButton->setObjectName("eyeButton");
    eyeButton->setFixedSize(50, 50);
    eyeButton->setCheckable(true);

    passLayout->addWidget(passwordEdit);
    passLayout->addWidget(eyeButton);

    // 服务器选择
    serverCombo = new QComboBox(loginBox);
    serverCombo->setObjectName("serverCombo");
    serverCombo->addItems(QStringList()
        << "本地服务器 (127.0.0.1:8080)"
        << "测试服务器 (test.bookstore.com)"
        << "正式服务器 (api.bookstore.com)");
    serverCombo->setMinimumHeight(45);

    // 选项
    QWidget *optionWidget = new QWidget(loginBox);
    QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);
    optionLayout->setContentsMargins(0, 0, 0, 0);

    rememberCheck = new QCheckBox("记住密码", optionWidget);
    rememberCheck->setObjectName("rememberCheck");

    autoLoginCheck = new QCheckBox("自动登录", optionWidget);
    autoLoginCheck->setObjectName("autoLoginCheck");

    optionLayout->addWidget(rememberCheck);
    optionLayout->addStretch();
    optionLayout->addWidget(autoLoginCheck);

    // 添加表单行
    QLabel *userLabel = new QLabel("账号:", loginBox);
    userLabel->setProperty("accessibleName", "formLabel");
    QLabel *passLabel = new QLabel("密码:", loginBox);
    passLabel->setProperty("accessibleName", "formLabel");
    QLabel *serverLabel = new QLabel("服务器:", loginBox);
    serverLabel->setProperty("accessibleName", "formLabel");

    formLayout->addRow(userLabel, usernameEdit);
    formLayout->addRow(passLabel, passwordWidget);
    formLayout->addRow(serverLabel, serverCombo);

    // 按钮区域
    QWidget *buttonWidget = new QWidget(loginBox);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setSpacing(15);

    loginButton = new QPushButton("登录系统", buttonWidget);
       loginButton->setObjectName("loginButton");
       loginButton->setMinimumHeight(55);

       // 链接按钮
       QWidget *linkWidget = new QWidget(buttonWidget);
       QHBoxLayout *linkLayout = new QHBoxLayout(linkWidget);
       linkLayout->setContentsMargins(0, 0, 0, 0);

       registerButton = new QPushButton("注册账号", linkWidget);
       registerButton->setObjectName("registerButton");

       forgotButton = new QPushButton("忘记密码?", linkWidget);
       forgotButton->setObjectName("forgotButton");

       linkLayout->addWidget(registerButton);
       linkLayout->addStretch();
       linkLayout->addWidget(forgotButton);

       // 消息标签
       errorLabel = new QLabel(loginBox);
       errorLabel->setObjectName("errorLabel");
       errorLabel->hide();

       successLabel = new QLabel(loginBox);
       successLabel->setObjectName("successLabel");
       successLabel->hide();

       // 组装登录框
       boxLayout->addLayout(formLayout);
       boxLayout->addWidget(optionWidget);
       boxLayout->addSpacing(20);
       boxLayout->addWidget(loginButton);
       boxLayout->addWidget(linkWidget);
       boxLayout->addSpacing(15);
       boxLayout->addWidget(errorLabel);
       boxLayout->addWidget(successLabel);

       // ========== 页脚 ==========
       footerLabel = new QLabel(QString("© 2023-2024 图书商家管理系统 | 当前时间: %1")
           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")), mainContainer);
       footerLabel->setObjectName("footerLabel");
       footerLabel->setAlignment(Qt::AlignCenter);
       footerLabel->setFixedHeight(60);

       // ========== 组装主布局 ==========
       // 创建居中容器
       QWidget *centerWidget = new QWidget(mainContainer);
       centerWidget->setObjectName("centerWidget");

       QHBoxLayout *centerHLayout = new QHBoxLayout(centerWidget);
       centerHLayout->setContentsMargins(0, 0, 0, 0);

       QWidget *verticalContainer = new QWidget(centerWidget);
       QVBoxLayout *verticalLayout = new QVBoxLayout(verticalContainer);
       verticalLayout->setContentsMargins(0, 0, 0, 0);

       verticalLayout->addStretch();
       verticalLayout->addWidget(loginBox, 0, Qt::AlignCenter);
       verticalLayout->addStretch();

       centerHLayout->addStretch();
       centerHLayout->addWidget(verticalContainer);
       centerHLayout->addStretch();

       // 最终组装
       mainLayout->addWidget(headerWidget);
       mainLayout->addWidget(centerWidget, 1);
       mainLayout->addWidget(footerLabel);

       // 设置主布局
       QHBoxLayout *finalLayout = new QHBoxLayout(this);
       finalLayout->setContentsMargins(0, 0, 0, 0);
       finalLayout->addWidget(mainContainer);

       // 连接信号
       connect(loginButton, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
       connect(registerButton, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
       connect(forgotButton, &QPushButton::clicked, this, &LoginWidget::onForgotPasswordClicked);
       connect(eyeButton, &QPushButton::clicked, this, &LoginWidget::onEyeButtonClicked);
       connect(rememberCheck, &QCheckBox::stateChanged, this, &LoginWidget::onRememberChanged);
       connect(serverCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoginWidget::onServerChanged);
       connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
       connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);

       // 启动背景动画定时器
       bgTimer = new QTimer(this);
       connect(bgTimer, &QTimer::timeout, this, [this]() {
           bgOffset++;
           if (bgOffset > 1000) bgOffset = 0;
           update();
       });
       bgTimer->start(50);
   }

   // ========== 样式应用 ==========
   void LoginWidget::applyStyle()
   {
       QString styleSheet = QString(R"(
           /* 主容器全屏 */
           #mainContainer {
               background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                   stop:0 %1,
                   stop:0.5 %2,
                   stop:1 %3);
               border-radius: 0px;
               border: none;
               min-width: 100%;
               min-height: 100%;
           }

           /* 标题栏 */
           #headerWidget {
               background: transparent;
               border-bottom: 2px solid rgba(255, 255, 255, 0.15);
           }

           #logoLabel {
               font-size: 40px;
               color: white;
           }

           #titleLabel {
               font-size: 32px;
               font-weight: bold;
               color: white;
               font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
           }

           #subtitleLabel {
               font-size: 16px;
               color: rgba(255, 255, 255, 0.85);
               font-family: 'Segoe UI', Arial;
           }

           #versionLabel {
               font-size: 14px;
               color: rgba(255, 255, 255, 0.7);
               padding: 8px 16px;
               background: rgba(255, 255, 255, 0.12);
               border-radius: 12px;
               font-weight: 500;
           }

           /* 登录框 */
           #loginBox {
               background: white;
               border-radius: 20px;
               border: 1px solid rgba(0, 0, 0, 0.08);
           }

           QGroupBox#loginBox {
               font-size: 16px;
               font-weight: bold;
               color: #2c3e50;
               padding-top: 20px;
           }

           /* 输入框 */
           QLineEdit {
               border: 2px solid #e1e8ed;
               border-radius: 10px;
               padding: 12px 20px;
               font-size: 15px;
               background: #f8fafc;
               selection-background-color: %4;
           }

           QLineEdit:focus {
               border-color: %4;
               background: white;
               box-shadow: 0 0 0 3px rgba(41, 128, 185, 0.1);
           }

           QLineEdit#usernameEdit {
               background-image: url(:/icons/user.png);
               background-position: right 15px center;
               background-repeat: no-repeat;
               padding-right: 50px;
           }

           QLineEdit#passwordEdit {
               background-image: url(:/icons/lock.png);
               background-position: right 15px center;
               background-repeat: no-repeat;
               padding-right: 50px;
           }

           /* 眼睛按钮 */
           #eyeButton {
               background: transparent;
               border: 2px solid #e1e8ed;
               border-radius: 10px;
               font-size: 20px;
               color: #7f8c8d;
           }

           #eyeButton:hover {
               background: #f1f8ff;
           }

           #eyeButton:checked {
               color: %4;
               border-color: %4;
           }

           /* 下拉框 */
           QComboBox {
               border: 2px solid #e1e8ed;
               border-radius: 10px;
               padding: 10px 20px;
               background: #f8fafc;
               font-size: 14px;
           }

           QComboBox:focus {
               border-color: %4;
           }

           QComboBox::drop-down {
               border: none;
               width: 40px;
           }

                                    QComboBox::down-arrow {
                                                image: url(:/icons/down.png);
                                                width: 14px;
                                                height: 14px;
                                            }

                                            /* 复选框 */
                                            QCheckBox {
                                                font-size: 14px;
                                                color: #5d6d7e;
                                                spacing: 10px;
                                            }

                                            QCheckBox::indicator {
                                                width: 20px;
                                                height: 20px;
                                                border: 2px solid #bdc3c7;
                                                border-radius: 5px;
                                            }

                                            QCheckBox::indicator:checked {
                                                background-color: %4;
                                                border-color: %4;
                                                image: url(:/icons/check.png);
                                            }

                                            /* 登录按钮 */
                                            #loginButton {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 %4,
                                                    stop:1 %5);
                                                color: white;
                                                border: none;
                                                border-radius: 10px;
                                                font-size: 18px;
                                                font-weight: bold;
                                                padding: 16px;
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
                                                background: #bdc3c7;
                                            }

                                            /* 链接按钮 */
                                            #registerButton, #forgotButton {
                                                background: transparent;
                                                border: none;
                                                color: %4;
                                                font-size: 14px;
                                                padding: 8px 12px;
                                                text-decoration: none;
                                                font-weight: 500;
                                            }

                                            #registerButton:hover, #forgotButton:hover {
                                                color: %6;
                                                text-decoration: underline;
                                            }

                                            /* 消息标签 */
                                            #errorLabel {
                                                color: %10;
                                                background-color: rgba(231, 76, 60, 0.08);
                                                border: 1px solid rgba(231, 76, 60, 0.2);
                                                border-radius: 8px;
                                                padding: 12px 20px;
                                                font-size: 14px;
                                            }

                                            #successLabel {
                                                color: #27ae60;
                                                background-color: rgba(46, 204, 113, 0.08);
                                                border: 1px solid rgba(46, 204, 113, 0.2);
                                                border-radius: 8px;
                                                padding: 12px 20px;
                                                font-size: 14px;
                                            }

                                            /* 页脚 */
                                            #footerLabel {
                                                color: rgba(255, 255, 255, 0.75);
                                                font-size: 14px;
                                                padding: 15px;
                                                background: rgba(0, 0, 0, 0.1);
                                                border-top: 1px solid rgba(255, 255, 255, 0.1);
                                            }

                                            /* 表单标签 */
                                            QLabel[accessibleName="formLabel"] {
                                                font-weight: bold;
                                                color: #2c3e50;
                                                font-size: 15px;
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
                                        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(loginBox);
                                        shadow->setBlurRadius(40);
                                        shadow->setColor(QColor(0, 0, 0, 120));
                                        shadow->setOffset(0, 10);
                                        loginBox->setGraphicsEffect(shadow);
                                    }

                                    // ========== 创建全屏对话框 ==========
                                    QDialog* LoginWidget::createFullscreenDialog(const QString &title)
                                    {
                                        QDialog *dialog = new QDialog(this);
                                        dialog->setWindowTitle(title);
                                        dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
                                        dialog->setAttribute(Qt::WA_TranslucentBackground);

                                        // 设置对话框大小与父窗口相同（全屏）
                                        dialog->setGeometry(this->geometry());

                                        // 创建主容器
                                        QWidget *container = new QWidget(dialog);
                                        container->setObjectName("dialogContainer");

                                        QVBoxLayout *containerLayout = new QVBoxLayout(container);
                                        containerLayout->setContentsMargins(0, 0, 0, 0);
                                        containerLayout->setSpacing(0);

                                        // 标题栏
                                        QWidget *dialogHeader = new QWidget(container);
                                        dialogHeader->setObjectName("dialogHeader");
                                        dialogHeader->setFixedHeight(80);

                                        QHBoxLayout *headerLayout = new QHBoxLayout(dialogHeader);
                                        headerLayout->setContentsMargins(30, 0, 30, 0);

                                        QLabel *dialogTitle = new QLabel(title, dialogHeader);
                                        dialogTitle->setObjectName("dialogTitle");

                                        QPushButton *closeButton = new QPushButton("×", dialogHeader);
                                        closeButton->setObjectName("closeButton");
                                        closeButton->setFixedSize(40, 40);

                                        headerLayout->addWidget(dialogTitle);
                                        headerLayout->addStretch();
                                        headerLayout->addWidget(closeButton);

                                        // 内容区域
                                        QScrollArea *scrollArea = new QScrollArea(container);
                                        scrollArea->setObjectName("dialogScrollArea");
                                        scrollArea->setWidgetResizable(true);
                                        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                                        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

                                        QWidget *contentWidget = new QWidget;
                                        contentWidget->setObjectName("dialogContent");

                                        scrollArea->setWidget(contentWidget);

                                        // 组装
                                        containerLayout->addWidget(dialogHeader);
                                        containerLayout->addWidget(scrollArea, 1);

                                        QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
                                        dialogLayout->setContentsMargins(0, 0, 0, 0);
                                        dialogLayout->addWidget(container);

                                        // 连接关闭按钮
                                        connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);

                                        return dialog;
                                    }

                                    void LoginWidget::setupDialogStyle(QDialog *dialog)
                                    {
                                        QString dialogStyle = QString(R"(
                                            #dialogContainer {
                                                background: #f8f9fa;  /* 保持浅灰色背景 */
                                                border-radius: 0px;
                                                min-width: 100%;
                                                min-height: 100%;
                                            }

                                            #dialogHeader {
                                                background: white;
                                                border-bottom: 2px solid #e1e8ed;
                                            }

                                            #dialogTitle {
                                                font-size: 26px;
                                                font-weight: bold;
                                                color: #2c3e50;
                                                font-family: 'Microsoft YaHei';
                                            }

                                            #closeButton {
                                                background: #e74c3c;
                                                color: white;
                                                border: none;
                                                border-radius: 20px;
                                                font-size: 24px;
                                                font-weight: bold;
                                                padding: 8px 16px;
                                                min-width: 40px;
                                                min-height: 40px;
                                            }

                                            #closeButton:hover {
                                                background: #c0392b;
                                                transform: scale(1.05);
                                            }

                                            #dialogScrollArea {
                                                background: transparent;
                                                border: none;
                                            }

                                            #dialogScrollArea QWidget {
                                                background: transparent;
                                            }

                                            #dialogContent {
                                                background: transparent;
                                                padding: 40px;
                                            }

                                            /* 对话框中的表单样式 */
                                            .dialogGroupBox {
                                                background: white;
                                                border-radius: 15px;
                                                border: 1px solid rgba(0, 0, 0, 0.1);
                                                padding: 30px;
                                                margin-bottom: 30px;
                                                box-shadow: 0 5px 15px rgba(0, 0, 0, 0.08);
                                            }

                                            .dialogGroupTitle {
                                                font-size: 22px;
                                                font-weight: bold;
                                                color: #2c3e50;
                                                margin-bottom: 25px;
                                                padding-bottom: 12px;
                                                border-bottom: 3px solid #2c3e50;  /* 深蓝色分割线 */
                                            }

                                            .dialogLabel {
                                                font-weight: bold;
                                                color: #34495e;
                                                font-size: 16px;
                                                min-width: 130px;
                                            }

                                            .dialogInput {
                                                border: 2px solid #dfe6e9;
                                                border-radius: 10px;
                                                padding: 14px 18px;
                                                font-size: 15px;
                                                background: #f8f9fa;
                                                min-height: 50px;
                                            }

                                            .dialogInput:focus {
                                                border-color: #2c3e50;
                                                background: white;
                                                box-shadow: 0 0 0 3px rgba(44, 62, 80, 0.2);
                                            }

                                            /* ========== 修改按钮颜色 ========== */
                                            /* 主要按钮：深蓝色 */
                                            .dialogButton {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 #2c3e50,      /* 深蓝色开始 */
                                                    stop:1 #34495e);     /* 中深蓝色结束 */
                                                color: white;
                                                border: none;
                                                border-radius: 10px;
                                                padding: 16px 32px;
                                                font-size: 17px;
                                                font-weight: bold;
                                                min-width: 140px;
                                                box-shadow: 0 4px 12px rgba(44, 62, 80, 0.3);
                                            }

                                            .dialogButton:hover {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 #1a252f,      /* 更深的蓝色 */
                                                    stop:1 #2c3e50);     /* 深蓝色 */
                                                transform: translateY(-3px);
                                                box-shadow: 0 6px 18px rgba(44, 62, 80, 0.4);
                                            }

                                            .dialogButton:pressed {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 #1a252f,
                                                    stop:1 #2c3e50);
                                                transform: translateY(0px);
                                                box-shadow: 0 2px 8px rgba(44, 62, 80, 0.3);
                                            }

                                            .dialogButton:disabled {
                                                background: #95a5a6;
                                                box-shadow: none;
                                            }

                                            /* 次要按钮：深灰色 */
                                            .dialogSecondaryButton {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 #4a5568,      /* 深灰色开始 */
                                                    stop:1 #5d6d7e);     /* 中深灰色结束 */
                                                color: white;
                                                border: none;
                                                border-radius: 10px;
                                                padding: 16px 32px;
                                                font-size: 17px;
                                                font-weight: bold;
                                                min-width: 140px;
                                            }

                                            .dialogSecondaryButton:hover {
                                                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                    stop:0 #3c4858,
                                                    stop:1 #4a5568);
                                                transform: translateY(-2px);
                                                box-shadow: 0 4px 12px rgba(74, 85, 104, 0.3);
                                            }

                                            .requiredStar {
                                                color: #e74c3c;
                                                font-weight: bold;
                                                font-size: 18px;
                                            }

                                            /* 其他文字颜色调整 */
                                            QLabel {
                                                color: #2c3e50;
                                            }

                                            /* 链接按钮颜色 */
                                            QPushButton[accessibleName="linkButton"] {
                                                background: transparent;
                                                border: none;
                                                color: #3498db;
                                                font-size: 14px;
                                                padding: 8px 12px;
                                                text-decoration: underline;
                                            }

                                            QPushButton[accessibleName="linkButton"]:hover {
                                                color: #2980b9;
                                            }
                                        )");

                                        dialog->setStyleSheet(dialogStyle);
                                    }

                                                                      // ========== 注册功能 ==========
                                                                      void LoginWidget::onRegisterClicked()
                                                                      {
                                                                          QDialog *dialog = createFullscreenDialog("商家注册申请");
                                                                          setupDialogStyle(dialog);

                                                                          QWidget *contentWidget = dialog->findChild<QWidget*>("dialogContent");
                                                                          QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
                                                                          contentLayout->setSpacing(30);
                                                                          contentLayout->setAlignment(Qt::AlignTop);

                                                                          // 标题
                                                                          QLabel *mainTitle = new QLabel("🏪 商家信息登记表");
                                                                          mainTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
                                                                          mainTitle->setAlignment(Qt::AlignCenter);

                                                                          QLabel *subTitle = new QLabel("(*为必填项，所有信息需真实有效)");
                                                                          subTitle->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-bottom: 30px;");
                                                                          subTitle->setAlignment(Qt::AlignCenter);

                                                                          contentLayout->addWidget(mainTitle);
                                                                          contentLayout->addWidget(subTitle);

                                                                          // ========== 第一部分：基础信息 ==========
                                                                          QWidget *basicGroup = new QWidget;
                                                                          basicGroup->setProperty("class", "dialogGroupBox");
                                                                          QVBoxLayout *basicLayout = new QVBoxLayout(basicGroup);

                                                                          QLabel *basicTitle = new QLabel("1. 基础信息");
                                                                          basicTitle->setProperty("class", "dialogGroupTitle");

                                                                          QFormLayout *basicForm = new QFormLayout;
                                                                          basicForm->setSpacing(15);
                                                                          basicForm->setLabelAlignment(Qt::AlignRight);

                                                                          // 商家名称
                                                                          QLineEdit *companyNameEdit = new QLineEdit;
                                                                          companyNameEdit->setProperty("class", "dialogInput");
                                                                          companyNameEdit->setPlaceholderText("请输入完整的商家名称");
                                                                          QLabel *companyLabel = new QLabel("商家名称<span class='requiredStar'>*</span>:");
                                                                          companyLabel->setProperty("class", "dialogLabel");

                                                                          // 联系人
                                                                          QLineEdit *contactNameEdit = new QLineEdit;
                                                                          contactNameEdit->setProperty("class", "dialogInput");
                                                                          contactNameEdit->setPlaceholderText("请输入联系人姓名");
                                                                          QLabel *contactLabel = new QLabel("联系人<span class='requiredStar'>*</span>:");
                                                                          contactLabel->setProperty("class", "dialogLabel");

                                                                          // 联系电话
                                                                          QLineEdit *phoneEdit = new QLineEdit;
                                                                          phoneEdit->setProperty("class", "dialogInput");
                                                                          phoneEdit->setPlaceholderText("请输入11位手机号码");
                                                                          phoneEdit->setMaxLength(11);
                                                                          QLabel *phoneLabel = new QLabel("联系电话<span class='requiredStar'>*</span>:");
                                                                          phoneLabel->setProperty("class", "dialogLabel");

                                                                          // 电子邮箱
                                                                          QLineEdit *emailEdit = new QLineEdit;
                                                                          emailEdit->setProperty("class", "dialogInput");
                                                                          emailEdit->setPlaceholderText("请输入常用邮箱");
                                                                          QLabel *emailLabel = new QLabel("电子邮箱<span class='requiredStar'>*</span>:");
                                                                          emailLabel->setProperty("class", "dialogLabel");

                                                                          basicForm->addRow(companyLabel, companyNameEdit);
                                                                          basicForm->addRow(contactLabel, contactNameEdit);
                                                                          basicForm->addRow(phoneLabel, phoneEdit);
                                                                          basicForm->addRow(emailLabel, emailEdit);

                                                                          basicLayout->addWidget(basicTitle);
                                                                          basicLayout->addLayout(basicForm);
                                                                          // ========== 第二部分：商家类型 ==========
                                                                          QWidget *typeGroup = new QWidget;
                                                                          typeGroup->setProperty("class", "dialogGroupBox");
                                                                          QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);

                                                                          QLabel *typeTitle = new QLabel("2. 商家类型");
                                                                          typeTitle->setProperty("class", "dialogGroupTitle");

                                                                          QButtonGroup *typeButtonGroup = new QButtonGroup(typeGroup);
                                                                          QGridLayout *typeGrid = new QGridLayout;
                                                                          typeGrid->setSpacing(15);

                                                                          QStringList businessTypes = {"个体商户", "有限公司", "连锁店", "出版社", "分销商", "其他"};
                                                                          for (int i = 0; i < businessTypes.size(); i++) {
                                                                              QRadioButton *radio = new QRadioButton(businessTypes[i], typeGroup);
                                                                              radio->setStyleSheet("QRadioButton { font-size: 14px; color: #34495e; }");
                                                                              typeButtonGroup->addButton(radio, i);
                                                                              typeGrid->addWidget(radio, i / 3, i % 3);
                                                                          }

                                                                          typeLayout->addWidget(typeTitle);
                                                                          typeLayout->addLayout(typeGrid);

                                                                          // ========== 第三部分：经营信息 ==========
                                                                          QWidget *businessGroup = new QWidget;
                                                                          businessGroup->setProperty("class", "dialogGroupBox");
                                                                          QVBoxLayout *businessLayout = new QVBoxLayout(businessGroup);

                                                                          QLabel *businessTitle = new QLabel("3. 经营信息");
                                                                          businessTitle->setProperty("class", "dialogGroupTitle");

                                                                          QFormLayout *businessForm = new QFormLayout;
                                                                          businessForm->setSpacing(15);
                                                                          businessForm->setLabelAlignment(Qt::AlignRight);

                                                                          // 主营图书类别
                                                                          QLineEdit *bookCategoryEdit = new QLineEdit;
                                                                          bookCategoryEdit->setProperty("class", "dialogInput");
                                                                          bookCategoryEdit->setPlaceholderText("如：文学小说、教材教辅、儿童读物等");
                                                                          QLabel *categoryLabel = new QLabel("主营图书类别:");
                                                                          categoryLabel->setProperty("class", "dialogLabel");

                                                                          // 年销售额预估
                                                                          QComboBox *salesCombo = new QComboBox;
                                                                          salesCombo->setProperty("class", "dialogInput");
                                                                          salesCombo->addItems({"请选择", "50万以下", "50-100万", "100-300万", "300-500万", "500万以上"});
                                                                          QLabel *salesLabel = new QLabel("年销售额预估:");
                                                                          salesLabel->setProperty("class", "dialogLabel");

                                                                          // 营业执照号
                                                                          QLineEdit *licenseEdit = new QLineEdit;
                                                                          licenseEdit->setProperty("class", "dialogInput");
                                                                          licenseEdit->setPlaceholderText("请输入营业执照注册号");
                                                                          QLabel *licenseLabel = new QLabel("营业执照号:");
                                                                          licenseLabel->setProperty("class", "dialogLabel");

                                                                          // 营业执照上传
                                                                          QWidget *uploadWidget = new QWidget;
                                                                          QHBoxLayout *uploadLayout = new QHBoxLayout(uploadWidget);
                                                                          uploadLayout->setContentsMargins(0, 0, 0, 0);

                                                                          QLineEdit *filePathEdit = new QLineEdit;
                                                                          filePathEdit->setProperty("class", "dialogInput");
                                                                          filePathEdit->setPlaceholderText("请上传营业执照扫描件");
                                                                          filePathEdit->setReadOnly(true);

                                                                          QPushButton *uploadButton = new QPushButton("选择文件");
                                                                          uploadButton->setProperty("class", "dialogSecondaryButton");
                                                                          uploadButton->setFixedWidth(100);

                                                                          uploadLayout->addWidget(filePathEdit);
                                                                          uploadLayout->addWidget(uploadButton);

                                                                          QLabel *uploadLabel = new QLabel("营业执照上传:");
                                                                          uploadLabel->setProperty("class", "dialogLabel");

                                                                          businessForm->addRow(categoryLabel, bookCategoryEdit);
                                                                          businessForm->addRow(salesLabel, salesCombo);
                                                                          businessForm->addRow(licenseLabel, licenseEdit);
                                                                          businessForm->addRow(uploadLabel, uploadWidget);

                                                                          businessLayout->addWidget(businessTitle);
                                                                          businessLayout->addLayout(businessForm);

                                                                          // ========== 第四部分：账号设置 ==========
                                                                          QWidget *accountGroup = new QWidget;
                                                                          accountGroup->setProperty("class", "dialogGroupBox");
                                                                          QVBoxLayout *accountLayout = new QVBoxLayout(accountGroup);

                                                                          QLabel *accountTitle = new QLabel("4. 账号设置");
                                                                          accountTitle->setProperty("class", "dialogGroupTitle");

                                                                          QFormLayout *accountForm = new QFormLayout;
                                                                          accountForm->setSpacing(15);
                                                                          accountForm->setLabelAlignment(Qt::AlignRight);

                                                                          // 登录账号
                                                                          QLineEdit *loginAccountEdit = new QLineEdit;
                                                                          loginAccountEdit->setProperty("class", "dialogInput");
                                                                          loginAccountEdit->setPlaceholderText("6-20位字母数字组合");
                                                                          QLabel *accountLabel = new QLabel("登录账号<span class='requiredStar'>*</span>:");
                                                                          accountLabel->setProperty("class", "dialogLabel");

                                                                          // 登录密码
                                                                          QLineEdit *loginPasswordEdit = new QLineEdit;
                                                                          loginPasswordEdit->setProperty("class", "dialogInput");
                                                                          loginPasswordEdit->setPlaceholderText("8-20位，包含字母和数字");
                                                                          loginPasswordEdit->setEchoMode(QLineEdit::Password);
                                                                          QLabel *passwordLabel = new QLabel("登录密码<span class='requiredStar'>*</span>:");
                                                                          passwordLabel->setProperty("class", "dialogLabel");

                                                                          // 确认密码
                                                                          QLineEdit *confirmPasswordEdit = new QLineEdit;
                                                                          confirmPasswordEdit->setProperty("class", "dialogInput");
                                                                          confirmPasswordEdit->setPlaceholderText("请再次输入密码");
                                                                          confirmPasswordEdit->setEchoMode(QLineEdit::Password);
                                                                          QLabel *confirmLabel = new QLabel("确认密码<span class='requiredStar'>*</span>:");
                                                                          confirmLabel->setProperty("class", "dialogLabel");

                                                                          accountForm->addRow(accountLabel, loginAccountEdit);
                                                                          accountForm->addRow(passwordLabel, loginPasswordEdit);
                                                                          accountForm->addRow(confirmLabel, confirmPasswordEdit);

                                                                          accountLayout->addWidget(accountTitle);
                                                                          accountLayout->addLayout(accountForm);

                                                                          // ========== 第五部分：协议确认 ==========
                                                                          QWidget *agreementGroup = new QWidget;
                                                                          agreementGroup->setProperty("class", "dialogGroupBox");
                                                                          QVBoxLayout *agreementLayout = new QVBoxLayout(agreementGroup);

                                                                          QLabel *agreementTitle = new QLabel("5. 协议确认");
                                                                          agreementTitle->setProperty("class", "dialogGroupTitle");

                                                                          QCheckBox *agreementCheck1 = new QCheckBox("我已阅读并同意《图书商家服务协议》");
                                                                          QCheckBox *agreementCheck2 = new QCheckBox("我保证所填写的信息真实有效，并愿意承担相应法律责任");

                                                                          agreementCheck1->setStyleSheet("QCheckBox { font-size: 14px; color: #34495e; margin-bottom: 10px; }");
                                                                          agreementCheck2->setStyleSheet("QCheckBox { font-size: 14px; color: #34495e; }");

                                                                          agreementLayout->addWidget(agreementTitle);
                                                                          agreementLayout->addWidget(agreementCheck1);
                                                                          agreementLayout->addWidget(agreementCheck2);
                                                                          // ========== 按钮区域 ==========
                                                                             QWidget *buttonGroup = new QWidget;
                                                                             QHBoxLayout *buttonGroupLayout = new QHBoxLayout(buttonGroup);
                                                                             buttonGroupLayout->setContentsMargins(0, 20, 0, 0);
                                                                             buttonGroupLayout->setSpacing(30);

                                                                             QPushButton *submitButton = new QPushButton("提交申请");
                                                                             submitButton->setProperty("class", "dialogButton");
                                                                             submitButton->setFixedWidth(180);

                                                                             QPushButton *resetButton = new QPushButton("重置表单");
                                                                             resetButton->setProperty("class", "dialogSecondaryButton");
                                                                             resetButton->setFixedWidth(180);

                                                                             buttonGroupLayout->addStretch();
                                                                             buttonGroupLayout->addWidget(resetButton);
                                                                             buttonGroupLayout->addWidget(submitButton);
                                                                             buttonGroupLayout->addStretch();

                                                                             // ========== 提示信息 ==========
                                                                             QLabel *hintLabel = new QLabel("* 提交申请后，我们将在1-3个工作日内完成审核，审核结果将发送至您的邮箱。");
                                                                             hintLabel->setStyleSheet("font-size: 13px; color: #7f8c8d; margin-top: 20px; padding: 15px; background: rgba(255, 255, 255, 0.5); border-radius: 8px;");
                                                                             hintLabel->setWordWrap(true);
                                                                             hintLabel->setAlignment(Qt::AlignCenter);

                                                                             // ========== 组装所有部件 ==========
                                                                             contentLayout->addWidget(basicGroup);
                                                                             contentLayout->addWidget(typeGroup);
                                                                             contentLayout->addWidget(businessGroup);
                                                                             contentLayout->addWidget(accountGroup);
                                                                             contentLayout->addWidget(agreementGroup);
                                                                             contentLayout->addWidget(buttonGroup);
                                                                             contentLayout->addWidget(hintLabel);

                                                                             // ========== 连接信号 ==========
                                                                             // 上传文件按钮
                                                                             connect(uploadButton, &QPushButton::clicked, [=]() {
                                                                                 QString fileName = QFileDialog::getOpenFileName(dialog, "选择营业执照文件", "", "图片文件 (*.jpg *.jpeg *.png *.pdf)");
                                                                                 if (!fileName.isEmpty()) {
                                                                                     filePathEdit->setText(fileName);
                                                                                 }
                                                                             });

                                                                             // 重置按钮
                                                                             connect(resetButton, &QPushButton::clicked, [=]() {
                                                                                 companyNameEdit->clear();
                                                                                 contactNameEdit->clear();
                                                                                 phoneEdit->clear();
                                                                                 emailEdit->clear();
                                                                                 typeButtonGroup->setExclusive(false);
                                                                                 for (auto button : typeButtonGroup->buttons()) {
                                                                                     button->setChecked(false);
                                                                                 }
                                                                                 typeButtonGroup->setExclusive(true);
                                                                                 bookCategoryEdit->clear();
                                                                                 salesCombo->setCurrentIndex(0);
                                                                                 licenseEdit->clear();
                                                                                 filePathEdit->clear();
                                                                                 loginAccountEdit->clear();
                                                                                 loginPasswordEdit->clear();
                                                                                 confirmPasswordEdit->clear();
                                                                                 agreementCheck1->setChecked(false);
                                                                                 agreementCheck2->setChecked(false);
                                                                             });

                                                                             // 提交按钮
                                                                             connect(submitButton, &QPushButton::clicked, [=]() {
                                                                                 // 验证必填字段
                                                                                 if (companyNameEdit->text().trimmed().isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入商家名称！");
                                                                                     companyNameEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (contactNameEdit->text().trimmed().isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入联系人姓名！");
                                                                                     contactNameEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 QString phone = phoneEdit->text().trimmed();
                                                                                 if (phone.isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入联系电话！");
                                                                                     phoneEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (phone.length() != 11 || !phone.startsWith("1")) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入有效的11位手机号码！");
                                                                                     phoneEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 QString email = emailEdit->text().trimmed();
                                                                                 if (email.isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入电子邮箱！");
                                                                                     emailEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (!email.contains("@") || !email.contains(".")) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入有效的邮箱地址！");
                                                                                     emailEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 // 验证商家类型选择
                                                                                 if (typeButtonGroup->checkedId() == -1) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请选择商家类型！");
                                                                                     return;
                                                                                 }

                                                                                 // 验证账号设置
                                                                                 QString account = loginAccountEdit->text().trimmed();
                                                                                 if (account.isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入登录账号！");
                                                                                     loginAccountEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (account.length() < 6 || account.length() > 20) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "登录账号应为6-20位！");
                                                                                     loginAccountEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 QString password = loginPasswordEdit->text();
                                                                                 if (password.isEmpty()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请输入登录密码！");
                                                                                     loginPasswordEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (password.length() < 8 || password.length() > 20) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "登录密码应为8-20位！");
                                                                                     loginPasswordEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 if (password != confirmPasswordEdit->text()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "两次输入的密码不一致！");
                                                                                     confirmPasswordEdit->clear();
                                                                                     loginPasswordEdit->setFocus();
                                                                                     return;
                                                                                 }

                                                                                 // 验证协议确认
                                                                                 if (!agreementCheck1->isChecked() || !agreementCheck2->isChecked()) {
                                                                                     QMessageBox::warning(dialog, "验证失败", "请阅读并同意相关协议！");
                                                                                     return;
                                                                                 }

                                                                                 // 模拟提交过程
                                                                                 submitButton->setEnabled(false);
                                                                                 submitButton->setText("提交中...");

                                                                                 QProgressBar *progressBar = new QProgressBar;
                                                                                 progressBar->setRange(0, 100);
                                                                                 progressBar->setValue(0);
                                                                                 progressBar->setTextVisible(true);

                                                                                 QTimer *progressTimer = new QTimer;
                                                                                 int progressValue = 0;

                                                                                 connect(progressTimer, &QTimer::timeout, [=, &progressValue]() mutable {
                                                                                     progressValue += 10;
                                                                                     progressBar->setValue(progressValue);

                                                                                     if (progressValue >= 100) {
                                                                                         progressTimer->stop();
                                                                                         progressTimer->deleteLater();

                                                                                         QMessageBox::information(dialog, "提交成功",
                                                                                             "✅ 商家注册申请已提交成功！\n\n"
                                                                                             "我们将在1-3个工作日内完成审核，\n"
                                                                                             "审核结果将发送至您的邮箱：\n" + email + "\n\n"
                                                                                             "请保持电话畅通，注意查收邮件。");

                                                                                         dialog->accept();
                                                                                     }
                                                                                 });

                                                                                 contentLayout->addWidget(progressBar);
                                                                                 progressTimer->start(200);
                                                                             });

                                                                             // 显示对话框
                                                                             dialog->exec();
                                                                             dialog->deleteLater();
                                                                         }

                                                                         // ========== 忘记密码功能 ==========
                                                                         void LoginWidget::onForgotPasswordClicked()
                                                                         {
                                                                             QDialog *dialog = createFullscreenDialog("找回密码");
                                                                             setupDialogStyle(dialog);

                                                                             QWidget *contentWidget = dialog->findChild<QWidget*>("dialogContent");
                                                                             QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
                                                                             contentLayout->setSpacing(30);
                                                                             contentLayout->setAlignment(Qt::AlignTop);

                                                                             // 标题
                                                                             QLabel *mainTitle = new QLabel("🔐 找回密码");
                                                                             mainTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
                                                                             mainTitle->setAlignment(Qt::AlignCenter);

                                                                             QLabel *subTitle = new QLabel("请选择找回密码的方式");
                                                                             subTitle->setStyleSheet("font-size: 16px; color: #7f8c8d; margin-bottom: 40px;");
                                                                             subTitle->setAlignment(Qt::AlignCenter);

                                                                             contentLayout->addWidget(mainTitle);
                                                                             contentLayout->addWidget(subTitle);

                                                                             // 创建堆叠窗口
                                                                             QStackedWidget *stackedWidget = new QStackedWidget;

                                                                             // ========== 页面1：方式选择 ==========
                                                                             QWidget *choosePage = new QWidget;
                                                                             QVBoxLayout *chooseLayout = new QVBoxLayout(choosePage);
                                                                             chooseLayout->setSpacing(30);
                                                                             chooseLayout->setAlignment(Qt::AlignCenter);

                                                                             // 选项卡片1：手机验证
                                                                             QWidget *phoneCard = new QWidget;
                                                                             phoneCard->setProperty("class", "dialogGroupBox");
                                                                             phoneCard->setMinimumHeight(200);
                                                                             QVBoxLayout *phoneLayout = new QVBoxLayout(phoneCard);

                                                                             QLabel *phoneIcon = new QLabel("📱");
                                                                             phoneIcon->setStyleSheet("font-size: 48px; margin-bottom: 15px;");
                                                                             phoneIcon->setAlignment(Qt::AlignCenter);

                                                                             QLabel *phoneTitle = new QLabel("通过手机验证找回");
                                                                             phoneTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
                                                                             phoneTitle->setAlignment(Qt::AlignCenter);

                                                                             QLabel *phoneDesc = new QLabel("通过注册手机接收验证码重置密码\n验证后系统将发送临时密码至您的手机");
                                                                             phoneDesc->setStyleSheet("font-size: 14px; color: #7f8c8d; line-height: 1.5;");
                                                                             phoneDesc->setAlignment(Qt::AlignCenter);
                                                                             phoneDesc->setWordWrap(true);

                                                                             QPushButton *phoneButton = new QPushButton("使用此方式");
                                                                             phoneButton->setProperty("class", "dialogButton");
                                                                             phoneButton->setFixedWidth(180);

                                                                             phoneLayout->addWidget(phoneIcon);
                                                                             phoneLayout->addWidget(phoneTitle);
                                                                             phoneLayout->addWidget(phoneDesc);
                                                                             phoneLayout->addStretch();
                                                                             phoneLayout->addWidget(phoneButton, 0, Qt::AlignCenter);

                                                                             // 选项卡片2：人工客服
                                                                             QWidget *serviceCard = new QWidget;
                                                                             serviceCard->setProperty("class", "dialogGroupBox");
                                                                             serviceCard->setMinimumHeight(200);
                                                                             QVBoxLayout *serviceLayout = new QVBoxLayout(serviceCard);

                                                                             QLabel *serviceIcon = new QLabel("👨‍💼");
                                                                             serviceIcon->setStyleSheet("font-size: 48px; margin-bottom: 15px;");
                                                                             serviceIcon->setAlignment(Qt::AlignCenter);

                                                                             QLabel *serviceTitle = new QLabel("联系人工客服");
                                                                             serviceTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
                                                                             serviceTitle->setAlignment(Qt::AlignCenter);

                                                                             QLabel *serviceDesc = new QLabel("联系客服人员协助您重置密码\n需要验证商家信息和身份");
                                                                             serviceDesc->setStyleSheet("font-size: 14px; color: #7f8c8d; line-height: 1.5;");
                                                                             serviceDesc->setAlignment(Qt::AlignCenter);
                                                                             serviceDesc->setWordWrap(true);

                                                                             QPushButton *serviceButton = new QPushButton("联系客服");
                                                                             serviceButton->setProperty("class", "dialogButton");
                                                                             serviceButton->setFixedWidth(180);

                                                                             serviceLayout->addWidget(serviceIcon);
                                                                             serviceLayout->addWidget(serviceTitle);
                                                                             serviceLayout->addWidget(serviceDesc);
                                                                             serviceLayout->addStretch();
                                                                             serviceLayout->addWidget(serviceButton, 0, Qt::AlignCenter);

                                                                             // 排列卡片
                                                                             QHBoxLayout *cardsLayout = new QHBoxLayout;
                                                                             cardsLayout->setSpacing(40);
                                                                             cardsLayout->addWidget(phoneCard);
                                                                             cardsLayout->addWidget(serviceCard);

                                                                             chooseLayout->addLayout(cardsLayout);
                                                                             chooseLayout->addStretch();
                                                                             // ========== 页面2：手机验证页面 ==========
                                                                             QWidget *phoneVerifyPage = new QWidget;
                                                                             QVBoxLayout *verifyLayout = new QVBoxLayout(phoneVerifyPage);
                                                                             verifyLayout->setSpacing(30);
                                                                             verifyLayout->setAlignment(Qt::AlignTop);

                                                                             QLabel *verifyTitle = new QLabel("📱 手机验证找回密码");
                                                                             verifyTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
                                                                             verifyTitle->setAlignment(Qt::AlignCenter);

                                                                             // 表单
                                                                             QWidget *formGroup = new QWidget;
                                                                             formGroup->setProperty("class", "dialogGroupBox");
                                                                             QVBoxLayout *formLayout = new QVBoxLayout(formGroup);

                                                                             QLabel *formHint = new QLabel("请输入注册时绑定的手机号码");
                                                                             formHint->setStyleSheet("font-size: 15px; color: #34495e; margin-bottom: 25px;");

                                                                             QFormLayout *inputForm = new QFormLayout;
                                                                             inputForm->setSpacing(20);
                                                                             inputForm->setLabelAlignment(Qt::AlignRight);

                                                                             // 手机号输入
                                                                             QLineEdit *phoneInput = new QLineEdit;
                                                                             phoneInput->setProperty("class", "dialogInput");
                                                                             phoneInput->setPlaceholderText("请输入11位手机号码");
                                                                             phoneInput->setMaxLength(11);
                                                                             QLabel *phoneInputLabel = new QLabel("手机号码:");
                                                                             phoneInputLabel->setProperty("class", "dialogLabel");

                                                                             // 验证码输入
                                                                             QWidget *codeWidget = new QWidget;
                                                                             QHBoxLayout *codeLayout = new QHBoxLayout(codeWidget);
                                                                             codeLayout->setContentsMargins(0, 0, 0, 0);
                                                                             codeLayout->setSpacing(15);

                                                                             QLineEdit *codeInput = new QLineEdit;
                                                                             codeInput->setProperty("class", "dialogInput");
                                                                             codeInput->setPlaceholderText("请输入6位验证码");
                                                                             codeInput->setMaxLength(6);

                                                                             QPushButton *sendCodeBtn = new QPushButton("获取验证码");
                                                                             sendCodeBtn->setProperty("class", "dialogSecondaryButton");
                                                                             sendCodeBtn->setFixedWidth(120);

                                                                             codeLayout->addWidget(codeInput);
                                                                             codeLayout->addWidget(sendCodeBtn);

                                                                             QLabel *codeLabel = new QLabel("验证码:");
                                                                             codeLabel->setProperty("class", "dialogLabel");

                                                                             inputForm->addRow(phoneInputLabel, phoneInput);
                                                                             inputForm->addRow(codeLabel, codeWidget);

                                                                             // 按钮
                                                                             QPushButton *verifySubmitButton = new QPushButton("验证并重置");
                                                                             verifySubmitButton->setProperty("class", "dialogButton");
                                                                             verifySubmitButton->setFixedWidth(180);

                                                                             QPushButton *backButton1 = new QPushButton("返回");
                                                                             backButton1->setProperty("class", "dialogSecondaryButton");
                                                                             backButton1->setFixedWidth(100);

                                                                             QHBoxLayout *verifyButtonLayout = new QHBoxLayout;
                                                                             verifyButtonLayout->addWidget(backButton1);
                                                                             verifyButtonLayout->addStretch();
                                                                             verifyButtonLayout->addWidget(verifySubmitButton);

                                                                             formLayout->addWidget(formHint);
                                                                             formLayout->addLayout(inputForm);
                                                                             formLayout->addSpacing(30);
                                                                             formLayout->addLayout(verifyButtonLayout);

                                                                             verifyLayout->addWidget(verifyTitle);
                                                                             verifyLayout->addWidget(formGroup);
                                                                             verifyLayout->addStretch();
                                                                             // ========== 页面3：客服信息页面 ==========
                                                                                QWidget *servicePage = new QWidget;
                                                                                QVBoxLayout *servicePageLayout = new QVBoxLayout(servicePage);
                                                                                servicePageLayout->setSpacing(30);
                                                                                servicePageLayout->setAlignment(Qt::AlignTop);

                                                                                QLabel *servicePageTitle = new QLabel("👨‍💼 人工客服帮助");
                                                                                servicePageTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
                                                                                servicePageTitle->setAlignment(Qt::AlignCenter);

                                                                                // 客服信息卡片
                                                                                QWidget *serviceInfoGroup = new QWidget;
                                                                                serviceInfoGroup->setProperty("class", "dialogGroupBox");
                                                                                QVBoxLayout *serviceInfoLayout = new QVBoxLayout(serviceInfoGroup);

                                                                                QLabel *serviceIcon2 = new QLabel("☎️");
                                                                                serviceIcon2->setStyleSheet("font-size: 60px; margin-bottom: 20px;");
                                                                                serviceIcon2->setAlignment(Qt::AlignCenter);

                                                                                QLabel *phoneNumber = new QLabel("400-1234-5678");
                                                                                phoneNumber->setStyleSheet("font-size: 32px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
                                                                                phoneNumber->setAlignment(Qt::AlignCenter);

                                                                                QLabel *serviceTime = new QLabel("服务时间：周一至周五 9:00-18:00");
                                                                                serviceTime->setStyleSheet("font-size: 16px; color: #7f8c8d; margin-bottom: 30px;");
                                                                                serviceTime->setAlignment(Qt::AlignCenter);

                                                                                QPushButton *copyButton = new QPushButton("复制号码");
                                                                                copyButton->setProperty("class", "dialogButton");
                                                                                copyButton->setFixedWidth(150);

                                                                                // 提示信息
                                                                                QWidget *hintGroup = new QWidget;
                                                                                hintGroup->setProperty("class", "dialogGroupBox");
                                                                                QVBoxLayout *hintLayout = new QVBoxLayout(hintGroup);

                                                                                QLabel *hintTitle = new QLabel("📋 联系客服前请准备：");
                                                                                hintTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 15px;");

                                                                                QLabel *hintList = new QLabel(
                                                                                    "1. 商家注册时的全称\n"
                                                                                    "2. 注册时使用的手机号码\n"
                                                                                    "3. 营业执照号码（如有）\n"
                                                                                    "4. 联系人身份证号（用于身份验证）\n"
                                                                                    "5. 可能需要的其他证明材料"
                                                                                );
                                                                                hintList->setStyleSheet("font-size: 15px; color: #34495e; line-height: 1.8; background: #f8f9fa; padding: 20px; border-radius: 8px;");
                                                                                hintList->setWordWrap(true);

                                                                                QPushButton *backButton2 = new QPushButton("返回");
                                                                                backButton2->setProperty("class", "dialogSecondaryButton");
                                                                                backButton2->setFixedWidth(100);

                                                                                QHBoxLayout *serviceButtonLayout = new QHBoxLayout;
                                                                                serviceButtonLayout->addWidget(backButton2);
                                                                                serviceButtonLayout->addStretch();

                                                                                serviceInfoLayout->addWidget(serviceIcon2);
                                                                                serviceInfoLayout->addWidget(phoneNumber);
                                                                                serviceInfoLayout->addWidget(serviceTime);
                                                                                serviceInfoLayout->addWidget(copyButton, 0, Qt::AlignCenter);
                                                                                serviceInfoLayout->addSpacing(30);

                                                                                hintLayout->addWidget(hintTitle);
                                                                                hintLayout->addWidget(hintList);

                                                                                servicePageLayout->addWidget(servicePageTitle);
                                                                                servicePageLayout->addWidget(serviceInfoGroup);
                                                                                servicePageLayout->addWidget(hintGroup);
                                                                                servicePageLayout->addLayout(serviceButtonLayout);
                                                                                servicePageLayout->addStretch();

                                                                                // ========== 页面4：验证成功页面 ==========
                                                                                QWidget *successPage = new QWidget;
                                                                                QVBoxLayout *successLayout = new QVBoxLayout(successPage);
                                                                                successLayout->setAlignment(Qt::AlignCenter);
                                                                                successLayout->setSpacing(30);

                                                                                QLabel *successIcon = new QLabel("✅");
                                                                                successIcon->setStyleSheet("font-size: 80px; margin-bottom: 20px;");
                                                                                successIcon->setAlignment(Qt::AlignCenter);

                                                                                QLabel *successTitle = new QLabel("密码重置成功！");
                                                                                successTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #27ae60; margin-bottom: 15px;");
                                                                                successTitle->setAlignment(Qt::AlignCenter);

                                                                                QLabel *successMessage = new QLabel(
                                                                                    "临时密码已发送至您的手机\n"
                                                                                    "请使用临时密码登录系统\n"
                                                                                    "登录后请及时修改密码"
                                                                                );
                                                                                successMessage->setStyleSheet("font-size: 16px; color: #34495e; line-height: 1.6; text-align: center;");
                                                                                successMessage->setWordWrap(true);
                                                                                successMessage->setAlignment(Qt::AlignCenter);

                                                                                QPushButton *closeButton = new QPushButton("关闭");
                                                                                closeButton->setProperty("class", "dialogButton");
                                                                                closeButton->setFixedWidth(150);

                                                                                successLayout->addWidget(successIcon);
                                                                                successLayout->addWidget(successTitle);
                                                                                successLayout->addWidget(successMessage);
                                                                                successLayout->addSpacing(30);
                                                                                successLayout->addWidget(closeButton);

                                                                                // ========== 添加所有页面到堆叠窗口 ==========
                                                                                stackedWidget->addWidget(choosePage);      // 索引0
                                                                                stackedWidget->addWidget(phoneVerifyPage); // 索引1
                                                                                stackedWidget->addWidget(servicePage);     // 索引2
                                                                                stackedWidget->addWidget(successPage);     // 索引3

                                                                                contentLayout->addWidget(stackedWidget);

                                                                                // ========== 连接信号 ==========
                                                                                // 选择手机验证
                                                                                connect(phoneButton, &QPushButton::clicked, [=]() {
                                                                                    stackedWidget->setCurrentIndex(1);
                                                                                });

                                                                                // 选择人工客服
                                                                                connect(serviceButton, &QPushButton::clicked, [=]() {
                                                                                    stackedWidget->setCurrentIndex(2);
                                                                                });

                                                                                // 返回按钮
                                                                                connect(backButton1, &QPushButton::clicked, [=]() {
                                                                                    stackedWidget->setCurrentIndex(0);
                                                                                });

                                                                                connect(backButton2, &QPushButton::clicked, [=]() {
                                                                                    stackedWidget->setCurrentIndex(0);
                                                                                });

                                                                                // 复制电话号码
                                                                                connect(copyButton, &QPushButton::clicked, [=]() {
                                                                                    QClipboard *clipboard = QApplication::clipboard();
                                                                                    clipboard->setText("400-1234-5678");
                                                                                    QMessageBox::information(dialog, "复制成功", "客服电话已复制到剪贴板！");
                                                                                });

                                                                                // 发送验证码
                                                                                connect(sendCodeBtn, &QPushButton::clicked, [=]() {
                                                                                    QString phone = phoneInput->text().trimmed();
                                                                                    if (phone.isEmpty()) {
                                                                                        QMessageBox::warning(dialog, "提示", "请输入手机号码！");
                                                                                        phoneInput->setFocus();
                                                                                        return;
                                                                                    }

                                                                                    if (phone.length() != 11 || !phone.startsWith("1")) {
                                                                                        QMessageBox::warning(dialog, "提示", "请输入有效的11位手机号码！");
                                                                                        phoneInput->setFocus();
                                                                                        return;
                                                                                    }
                                                                                    // 开始倒计时
                                                                                          countdownSeconds = 60;
                                                                                          sendCodeBtn->setEnabled(false);
                                                                                          sendCodeBtn->setText(QString("重新发送(%1)").arg(countdownSeconds));
                                                                                          countdownTimer->start(1000);

                                                                                          // 模拟发送验证码
                                                                                          QMessageBox::information(dialog, "验证码已发送",
                                                                                              QString("验证码已发送至手机：%1\n\n验证码：123456（模拟）").arg(phone));
                                                                                      });

                                                                                      // 提交验证
                                                                                      connect(verifySubmitButton, &QPushButton::clicked, [=]() {
                                                                                          QString phone = phoneInput->text().trimmed();
                                                                                          QString code = codeInput->text().trimmed();

                                                                                          if (phone.isEmpty()) {
                                                                                              QMessageBox::warning(dialog, "提示", "请输入手机号码！");
                                                                                              phoneInput->setFocus();
                                                                                              return;
                                                                                          }

                                                                                          if (code.isEmpty()) {
                                                                                              QMessageBox::warning(dialog, "提示", "请输入验证码！");
                                                                                              codeInput->setFocus();
                                                                                              return;
                                                                                          }

                                                                                          if (code != "123456") { // 模拟验证码
                                                                                              QMessageBox::warning(dialog, "验证失败", "验证码错误，请输入正确的验证码！");
                                                                                              codeInput->clear();
                                                                                              codeInput->setFocus();
                                                                                              return;
                                                                                          }

                                                                                          // 模拟处理过程
                                                                                          verifySubmitButton->setEnabled(false);
                                                                                          verifySubmitButton->setText("处理中...");

                                                                                          QTimer::singleShot(1500, [=]() {
                                                                                              stackedWidget->setCurrentIndex(3);
                                                                                          });
                                                                                      });

                                                                                      // 关闭成功页面
                                                                                      connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

                                                                                      // 显示对话框
                                                                                      dialog->exec();
                                                                                      dialog->deleteLater();
                                                                                  }

                                                                                  // ========== 辅助函数 ==========
                                                                                  void LoginWidget::updateCountdown()
                                                                                  {
                                                                                      countdownSeconds--;
                                                                                      if (countdownSeconds > 0) {
                                                                                          // 更新所有对话框中的发送验证码按钮
                                                                                          if (sendCodeButton) {
                                                                                              sendCodeButton->setText(QString("重新发送(%1)").arg(countdownSeconds));
                                                                                          }
                                                                                      } else {
                                                                                          countdownTimer->stop();
                                                                                          if (sendCodeButton) {
                                                                                              sendCodeButton->setEnabled(true);
                                                                                              sendCodeButton->setText("获取验证码");
                                                                                          }
                                                                                      }
                                                                                  }

                                                                                  // ========== 其他原有函数（保持原有逻辑）==========
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
                                                                                      // 全屏模式不需要设置固定大小
                                                                                  }

                                                                                  void LoginWidget::paintEvent(QPaintEvent *event)
                                                                                  {
                                                                                      Q_UNUSED(event);
                                                                                      QPainter painter(this);
                                                                                      painter.setRenderHint(QPainter::Antialiasing);

                                                                                      // 绘制动态背景 - 填满整个窗口
                                                                                      QLinearGradient gradient(0, 0, width(), height());
                                                                                      gradient.setColorAt(0, QColor(41, 128, 185));
                                                                                      gradient.setColorAt(0.5, QColor(52, 152, 219));
                                                                                      gradient.setColorAt(1, QColor(31, 108, 165));

                                                                                      painter.fillRect(rect(), gradient);

                                                                                      // 绘制动态粒子
                                                                                      painter.setPen(Qt::NoPen);
                                                                                      painter.setBrush(QColor(255, 255, 255, 40));

                                                                                      int particleCount = 80;
                                                                                      for (int i = 0; i < particleCount; i++) {
                                                                                          int x = (qrand() + bgOffset * i) % width();
                                                                                          int y = (qrand() + bgOffset * (i + 1)) % height();
                                                                                          int size = 2 + (qrand() % 6);
                                                                                          painter.drawEllipse(x, y, size, size);
                                                                                      }

                                                                                      // 绘制一些较大的"星星"
                                                                                      painter.setBrush(QColor(255, 255, 255, 80));
                                                                                      for (int i = 0; i < 10; i++) {
                                                                                          int x = (qrand() + bgOffset * (i + 10)) % width();
                                                                                          int y = (qrand() + bgOffset * (i + 20)) % height();
                                                                                          painter.drawEllipse(x, y, 8, 8);
                                                                                      }
                                                                                  }

                                                                                  void LoginWidget::resizeEvent(QResizeEvent *event)
                                                                                  {
                                                                                      QWidget::resizeEvent(event);
                                                                                      update();
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
                                                                                      if (loginAnimation) {
                                                                                          loginAnimation->start();
                                                                                      }
                                                                                      // 保存设置
                                                                                      saveSettings();

                                                                                      // 模拟网络请求延迟
                                                                                      QTimer::singleShot(1500, this, [this, username, password]() {
                                                                                          // 模拟登录验证
                                                                                          if ((username == "admin" && password == "admin123") ||
                                                                                              (username == "demo" && password == "demo123")) {
                                                                                              showSuccess("登录成功！正在进入系统...");
                                                                                              QTimer::singleShot(1000, this, &LoginWidget::onLoginSuccess);
                                                                                          } else {
                                                                                              onLoginFailed("用户名或密码错误");
                                                                                          }
                                                                                      });
                                                                                  }

                                                                                  void LoginWidget::onLoginSuccess()
                                                                                  {
                                                                                      loginButton->setEnabled(true);
                                                                                      loginButton->setText("登录系统");
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
                                                                                      QPoint currentPos = pos();
                                                                                      shakeAnim->setKeyValueAt(0, currentPos);
                                                                                      shakeAnim->setKeyValueAt(0.2, currentPos + QPoint(10, 0));
                                                                                      shakeAnim->setKeyValueAt(0.4, currentPos + QPoint(-10, 0));
                                                                                      shakeAnim->setKeyValueAt(0.6, currentPos + QPoint(10, 0));
                                                                                      shakeAnim->setKeyValueAt(0.8, currentPos + QPoint(-10, 0));
                                                                                      shakeAnim->setKeyValueAt(1, currentPos);
                                                                                      shakeAnim->start(QAbstractAnimation::DeleteWhenStopped);
                                                                                  }

                                                                                  void LoginWidget::showError(const QString &message)
                                                                                  {
                                                                                      errorLabel->setText("❌ " + message);
                                                                                      errorLabel->show();
                                                                                      successLabel->hide();
                                                                                      QTimer::singleShot(5000, errorLabel, &QLabel::hide);
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
                                                                                      QTimer::singleShot(3000, successLabel, &QLabel::hide);
                                                                                  }

                                                                                  void LoginWidget::fadeIn()
                                                                                  {
                                                                                      if (fadeAnimation) {
                                                                                          fadeAnimation->start();
                                                                                      }
                                                                                  }
                                                                                  // 槽函数实现（已在前面的lambda中实现）
                                                                                  void LoginWidget::onRegisterSubmit() {}
                                                                                  void LoginWidget::onRegisterReset() {}
                                                                                  void LoginWidget::onRegisterFileSelect() {}
                                                                                  void LoginWidget::onForgotSendCode() {}
                                                                                  void LoginWidget::onForgotSubmit() {}
                                                                                  void LoginWidget::onForgotCopyPhone() {}
