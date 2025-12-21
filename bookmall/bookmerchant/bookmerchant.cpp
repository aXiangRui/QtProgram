#include "bookmerchant.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>
#include <QHeaderView>
#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QThread>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include <QLabel>
#include <QDateTime>
#include <QCoreApplication>
#include <QSet>
#include <QMap>
#include <algorithm>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <cmath>

BookMerchant::BookMerchant(QWidget *parent)
    : QMainWindow(parent)
    , selectedBookRow(-1)
    , selectedOrderRow(-1)
    , selectedMemberRow(-1)
    , isLoggedIn(false)
    , serverIp("127.0.0.1")     // 默认服务器IP
    , serverPort(8888)           // 默认服务器端口
    , currentChatBuyerId(-1)     // 初始化当前聊天买家ID（-1表示与客服聊天）
    , salesChartWidget(nullptr)  // 初始化销量趋势图组件
{
    apiService = new ApiService(this);
    
    // 初始化客服聊天刷新定时器
    chatRefreshTimer = new QTimer(this);
    chatRefreshTimer->setInterval(2000);  // 每2秒刷新一次
    connect(chatRefreshTimer, &QTimer::timeout, this, &BookMerchant::loadChatHistory);
    
    // 初始化买家聊天刷新定时器
    buyerChatRefreshTimer = new QTimer(this);
    buyerChatRefreshTimer->setInterval(2000);  // 每2秒刷新一次
    connect(buyerChatRefreshTimer, &QTimer::timeout, this, &BookMerchant::loadBuyerChatHistory);
    
    // 初始化仪表板数据刷新定时器
    dashboardRefreshTimer = new QTimer(this);
    dashboardRefreshTimer->setInterval(5000);  // 每5秒刷新一次仪表板数据
    connect(dashboardRefreshTimer, &QTimer::timeout, this, &BookMerchant::updateDashboardData);
    
    // 连接信号
    connect(apiService, &ApiService::connected, this, [this]() {
        qDebug() << "已连接到服务器";
        if (loginStatusLabel) {
            loginStatusLabel->setText("✓ 已连接服务器");
            loginStatusLabel->setStyleSheet("color: green;");
        }
    });
    connect(apiService, &ApiService::disconnected, this, [this]() {
        qDebug() << "与服务器断开连接";
    });
    connect(apiService, &ApiService::errorOccurred, this, [this](const QString &error) {
        qDebug() << "网络错误:" << error;
    });
    
    qDebug() << "服务器配置 - IP:" << serverIp << "端口:" << serverPort;
    
    initUI();
    initConnections();
    applyStyle();
    
    setWindowTitle("图书商家管理系统");
    resize(1200, 800);
    
    showLoginPage();
    
    // 程序启动后500ms自动连接服务器
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "自动连接服务器...";
        if (!apiService->isConnected()) {
            if (apiService->connectToServer(serverIp, serverPort)) {
                qDebug() << "自动连接服务器成功";
            } else {
                qDebug() << "自动连接服务器失败，将在登录时重试";
                if (loginStatusLabel) {
                    loginStatusLabel->setText("⚠ 正在连接服务器...");
                    loginStatusLabel->setStyleSheet("color: orange;");
                }
            }
        }
    });
}

BookMerchant::~BookMerchant()
{
}

void BookMerchant::initUI()
{
    stackedWidget = new QStackedWidget(this);
    this->setStyleSheet("background-color:#f5f7fa;");  // 与买家界面保持一致的背景色
    setCentralWidget(stackedWidget);

    // ===== 登录页面 =====
    loginPage = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setAlignment(Qt::AlignCenter);
    loginLayout->setContentsMargins(20, 20, 20, 20);

    // 创建登录卡片容器
    QWidget *loginCard = new QWidget();
    loginCard->setFixedWidth(480);
    loginCard->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 20px;
        }
    )");
    QVBoxLayout *cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setContentsMargins(50, 50, 50, 50);
    cardLayout->setSpacing(25);

    // 标题
    QLabel *loginTitle = new QLabel("🏪 商家登录");
    loginTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setFamily("Microsoft YaHei");
    titleFont.setBold(true);
    titleFont.setPointSize(28);
    loginTitle->setFont(titleFont);
    loginTitle->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    cardLayout->addWidget(loginTitle);

    // 副标题
    QLabel *subTitle = new QLabel("欢迎回来，请登录您的商家账户");
    subTitle->setAlignment(Qt::AlignCenter);
    QFont subFont;
    subFont.setFamily("Microsoft YaHei");
    subFont.setPointSize(12);
    subTitle->setFont(subFont);
    subTitle->setStyleSheet("color: #7f8c8d; margin-bottom: 30px;");
    cardLayout->addWidget(subTitle);

    // 用户名输入框
    QLabel *usernameLabel = new QLabel("用户名");
    usernameLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-bottom: 5px;");
    cardLayout->addWidget(usernameLabel);

    loginUsername = new QLineEdit();
    loginUsername->setPlaceholderText("请输入用户名");
    loginUsername->setMinimumHeight(50);
    loginUsername->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    cardLayout->addWidget(loginUsername);

    // 密码输入框
    QLabel *passwordLabel = new QLabel("密码");
    passwordLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-top: 15px; margin-bottom: 5px;");
    cardLayout->addWidget(passwordLabel);
    
    loginPassword = new QLineEdit();
    loginPassword->setPlaceholderText("请输入密码");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setMinimumHeight(50);
    loginPassword->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    cardLayout->addWidget(loginPassword);

    // 状态标签
    loginStatusLabel = new QLabel("系统将自动连接服务器...");
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; min-height: 20px;");
    cardLayout->addWidget(loginStatusLabel);

    // 登录按钮
    loginButton = new QPushButton("登录");
    loginButton->setMinimumHeight(50);
    loginButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2980b9,
                stop:1 #3498db);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3498db,
                stop:1 #2980b9);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1f5f8f,
                stop:1 #2471a3);
        }
    )");
    cardLayout->addWidget(loginButton);

    // 将卡片添加到主布局
    loginLayout->addStretch();
    loginLayout->addWidget(loginCard, 0, Qt::AlignCenter);
    loginLayout->addStretch();

    stackedWidget->addWidget(loginPage);

    // ===== 主页面 =====
    mainPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    // 顶部标题栏：左侧刷新按钮，中间标题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    mainPageRefreshButton = new QPushButton("🔄 刷新");
    mainPageRefreshButton->setMinimumHeight(40);
    mainPageRefreshButton->setMaximumWidth(120);
    titleLayout->addWidget(mainPageRefreshButton);
    titleLayout->addStretch();
    
    // 欢迎标题
    welcomeLabel = new QLabel("欢迎使用图书商家管理系统");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(R"(
        QLabel {
            font-size: 28px;
            font-weight: bold;
            color: #2c3e50;
            padding: 20px;
            font-family: 'Microsoft YaHei';
        }
    )");
    titleLayout->addWidget(welcomeLabel);
    titleLayout->addStretch();
    // 添加一个相同宽度的占位控件，保持标题居中
    QWidget *spacer = new QWidget();
    spacer->setFixedWidth(mainPageRefreshButton->maximumWidth());
    titleLayout->addWidget(spacer);
    
    mainLayout->addLayout(titleLayout);

    // 导航按钮栏
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(10);
    booksButton = new QPushButton("📚 图书管理");
    ordersButton = new QPushButton("📦 订单管理");
    membersButton = new QPushButton("👥 会员管理");
    statsButton = new QPushButton("📊 统计报表");
    buyerChatButton = new QPushButton("💬 客户消息");
    reviewsButton = new QPushButton("⭐ 评论管理");
    profileButton = new QPushButton("👤 个人中心");
    logoutButton = new QPushButton("🚪 退出登录");

    // 导航按钮使用全局样式（蓝色渐变，与买家一致）
    // 按钮样式已在applyStyle()中统一定义，无需单独设置

    navLayout->addWidget(booksButton);
    navLayout->addWidget(ordersButton);
    navLayout->addWidget(membersButton);
    navLayout->addWidget(statsButton);
    navLayout->addWidget(buyerChatButton);
    navLayout->addWidget(reviewsButton);
    navLayout->addWidget(profileButton);
    navLayout->addStretch();
    navLayout->addWidget(logoutButton);
    mainLayout->addLayout(navLayout);

    // 数据统计卡片区域
    QHBoxLayout *statsCardsLayout = new QHBoxLayout();
    statsCardsLayout->setSpacing(15);

    // 订单数卡片
    QWidget *orderCard = createStatCard("📦", "今日订单", "0", "较昨日 +0%", "#3498db", &orderValueLabel);
    statsCardsLayout->addWidget(orderCard);

    // 销量卡片
    QWidget *salesCard = createStatCard("📈", "今日销量", "0", "较昨日 +0%", "#2ecc71", &salesValueLabel);
    statsCardsLayout->addWidget(salesCard);

    // 收入卡片
    QWidget *revenueCard = createStatCard("💰", "今日收入", "¥0.00", "较昨日 +0%", "#f39c12", &revenueValueLabel);
    statsCardsLayout->addWidget(revenueCard);

    // 图书总数卡片
    QWidget *booksCard = createStatCard("📚", "图书总数", "0", "在售图书", "#9b59b6", &booksValueLabel);
    statsCardsLayout->addWidget(booksCard);

    mainLayout->addLayout(statsCardsLayout);

    // 图表和详细信息区域
    QHBoxLayout *chartLayout = new QHBoxLayout();
    chartLayout->setSpacing(15);

    // 销量趋势图（左侧）- 使用自定义折线图组件
    QWidget *chartContainer = createChartWidget("📊 销量趋势", "近7天销售额变化");
    chartLayout->addWidget(chartContainer, 2);

    // 订单统计（右侧）
    QWidget *orderStatsWidget = createOrderStatsWidget();
    chartLayout->addWidget(orderStatsWidget, 1);

    mainLayout->addLayout(chartLayout);

    // 底部留白
    mainLayout->addStretch();

    stackedWidget->addWidget(mainPage);

    // ===== 图书管理页面 =====
    booksPage = new QWidget();
    QVBoxLayout *booksLayout = new QVBoxLayout(booksPage);
    booksLayout->setContentsMargins(15, 15, 15, 15);  // 减少边距
    booksLayout->setSpacing(10);  // 减少间距

    QLabel *booksTitle = new QLabel("图书管理");
    booksTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    booksLayout->addWidget(booksTitle);

    QHBoxLayout *booksButtonLayout = new QHBoxLayout();
    refreshBooksBtn = new QPushButton("刷新");
    addBookBtn = new QPushButton("添加图书");
    editBookBtn = new QPushButton("编辑图书");
    deleteBookBtn = new QPushButton("删除图书");
    backFromBooksBtn = new QPushButton("返回主页");

    booksButtonLayout->addWidget(refreshBooksBtn);
    booksButtonLayout->addWidget(addBookBtn);
    booksButtonLayout->addWidget(editBookBtn);
    booksButtonLayout->addWidget(deleteBookBtn);
    booksButtonLayout->addStretch();
    booksButtonLayout->addWidget(backFromBooksBtn);

    booksLayout->addLayout(booksButtonLayout);

    booksTable = new QTableWidget();
    booksTable->setColumnCount(9);
    booksTable->setHorizontalHeaderLabels({"ISBN", "书名", "作者", "分类", "子分类", "价格", "库存", "销量", "状态"});
    booksTable->horizontalHeader()->setStretchLastSection(true);
    booksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    booksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    booksLayout->addWidget(booksTable);

    stackedWidget->addWidget(booksPage);

    // ===== 订单管理页面 =====
    ordersPage = new QWidget();
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersPage);
    ordersLayout->setContentsMargins(15, 15, 15, 15);  // 减少边距
    ordersLayout->setSpacing(10);  // 减少间距

    QLabel *ordersTitle = new QLabel("订单管理");
    ordersTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    ordersLayout->addWidget(ordersTitle);

    QHBoxLayout *ordersButtonLayout = new QHBoxLayout();
    refreshOrdersBtn = new QPushButton("刷新订单");
    updateOrderStatusBtn = new QPushButton("📦 发货");
    updateOrderStatusBtn->setStyleSheet("background-color: #27ae60; font-weight: bold;");
    deleteOrderBtn = new QPushButton("删除订单");
    backFromOrdersBtn = new QPushButton("返回主页");
    
    orderStatusCombo = new QComboBox();
    orderStatusCombo->addItems({"待支付", "已支付", "已发货", "已完成", "已取消"});
    orderStatusCombo->setVisible(false);  // 隐藏状态选择框，改用发货按钮

    ordersButtonLayout->addWidget(refreshOrdersBtn);
    ordersButtonLayout->addWidget(updateOrderStatusBtn);
    ordersButtonLayout->addWidget(deleteOrderBtn);
    ordersButtonLayout->addStretch();
    ordersButtonLayout->addWidget(backFromOrdersBtn);

    ordersLayout->addLayout(ordersButtonLayout);

    ordersTable = new QTableWidget();
    ordersTable->setColumnCount(6);
    ordersTable->setHorizontalHeaderLabels({"订单ID", "客户", "总金额", "状态", "下单时间", "发货时间"});
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersLayout->addWidget(ordersTable);

    stackedWidget->addWidget(ordersPage);

    // ===== 会员管理页面 =====
    membersPage = new QWidget();
    QVBoxLayout *membersLayout = new QVBoxLayout(membersPage);
    membersLayout->setContentsMargins(15, 15, 15, 15);  // 减少边距
    membersLayout->setSpacing(10);  // 减少间距

    QLabel *membersTitle = new QLabel("会员管理");
    membersTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    membersLayout->addWidget(membersTitle);

    QHBoxLayout *membersButtonLayout = new QHBoxLayout();
    editMemberBtn = new QPushButton("编辑会员");
    deleteMemberBtn = new QPushButton("删除会员");
    backFromMembersBtn = new QPushButton("返回主页");

    membersButtonLayout->addWidget(editMemberBtn);
    membersButtonLayout->addWidget(deleteMemberBtn);
    membersButtonLayout->addStretch();
    membersButtonLayout->addWidget(backFromMembersBtn);

    membersLayout->addLayout(membersButtonLayout);

    membersTable = new QTableWidget();
    membersTable->setColumnCount(5);
    membersTable->setHorizontalHeaderLabels({"用户ID", "用户名", "邮箱", "会员等级", "注册日期"});
    membersTable->horizontalHeader()->setStretchLastSection(true);
    membersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    membersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    membersLayout->addWidget(membersTable);

    stackedWidget->addWidget(membersPage);

    // ===== 统计报表页面 =====
    statsPage = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsPage);
    statsLayout->setContentsMargins(15, 15, 15, 15);  // 减少边距
    statsLayout->setSpacing(10);  // 减少间距

    QLabel *statsTitle = new QLabel("统计报表");
    statsTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    statsLayout->addWidget(statsTitle);

    // 统计卡片
    QHBoxLayout *statsPageCardsLayout = new QHBoxLayout();
    
    QGroupBox *statsSalesCard = new QGroupBox("总销售额");
    QVBoxLayout *statsSalesCardLayout = new QVBoxLayout(statsSalesCard);
    totalSalesLabel = new QLabel("¥0.00");
    totalSalesLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #4CAF50;");
    totalSalesLabel->setAlignment(Qt::AlignCenter);
    statsSalesCardLayout->addWidget(totalSalesLabel);
    
    QGroupBox *statsOrdersCard = new QGroupBox("总订单数");
    QVBoxLayout *statsOrdersCardLayout = new QVBoxLayout(statsOrdersCard);
    totalOrdersLabel = new QLabel("0");
    totalOrdersLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2196F3;");
    totalOrdersLabel->setAlignment(Qt::AlignCenter);
    statsOrdersCardLayout->addWidget(totalOrdersLabel);
    
    QGroupBox *statsMembersCard = new QGroupBox("总会员数");
    QVBoxLayout *statsMembersCardLayout = new QVBoxLayout(statsMembersCard);
    totalMembersLabel = new QLabel("0");
    totalMembersLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #FF9800;");
    totalMembersLabel->setAlignment(Qt::AlignCenter);
    statsMembersCardLayout->addWidget(totalMembersLabel);
    
    QGroupBox *statsBooksCard = new QGroupBox("图书种类");
    QVBoxLayout *statsBooksCardLayout = new QVBoxLayout(statsBooksCard);
    totalBooksLabel = new QLabel("0");
    totalBooksLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #9C27B0;");
    totalBooksLabel->setAlignment(Qt::AlignCenter);
    statsBooksCardLayout->addWidget(totalBooksLabel);
    
    statsPageCardsLayout->addWidget(statsSalesCard);
    statsPageCardsLayout->addWidget(statsOrdersCard);
    statsPageCardsLayout->addWidget(statsMembersCard);
    statsPageCardsLayout->addWidget(statsBooksCard);
    
    statsLayout->addLayout(statsPageCardsLayout);

    QHBoxLayout *reportButtonLayout = new QHBoxLayout();
    refreshStatsBtn = new QPushButton("刷新统计");
    
    QLabel *dateRangeLabel = new QLabel("日期范围:");
    reportStartDate = new QDateEdit(QDate::currentDate().addDays(-30));
    reportStartDate->setCalendarPopup(true);
    reportEndDate = new QDateEdit(QDate::currentDate());
    reportEndDate->setCalendarPopup(true);
    
    generateSalesReportBtn = new QPushButton("销售报表");
    generateInventoryReportBtn = new QPushButton("库存报表");
    generateMemberReportBtn = new QPushButton("会员报表");
    backFromStatsBtn = new QPushButton("返回主页");

    reportButtonLayout->addWidget(refreshStatsBtn);
    reportButtonLayout->addWidget(dateRangeLabel);
    reportButtonLayout->addWidget(reportStartDate);
    reportButtonLayout->addWidget(new QLabel("-"));
    reportButtonLayout->addWidget(reportEndDate);
    reportButtonLayout->addWidget(generateSalesReportBtn);
    reportButtonLayout->addWidget(generateInventoryReportBtn);
    reportButtonLayout->addWidget(generateMemberReportBtn);
    reportButtonLayout->addStretch();
    reportButtonLayout->addWidget(backFromStatsBtn);

    statsLayout->addLayout(reportButtonLayout);

    reportDisplay = new QTextEdit();
    reportDisplay->setReadOnly(true);
    statsLayout->addWidget(reportDisplay);

    stackedWidget->addWidget(statsPage);
    
    // ===== 评论管理页面 =====
    reviewsPage = new QWidget();
    QVBoxLayout *reviewsLayout = new QVBoxLayout(reviewsPage);
    reviewsLayout->setContentsMargins(15, 15, 15, 15);
    reviewsLayout->setSpacing(10);
    
    QLabel *reviewsTitle = new QLabel("评论管理");
    reviewsTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    reviewsLayout->addWidget(reviewsTitle);
    
    QHBoxLayout *reviewsButtonLayout = new QHBoxLayout();
    refreshReviewsBtn = new QPushButton("刷新评论");
    backFromReviewsBtn = new QPushButton("返回主页");
    reviewsButtonLayout->addWidget(refreshReviewsBtn);
    reviewsButtonLayout->addStretch();
    reviewsButtonLayout->addWidget(backFromReviewsBtn);
    reviewsLayout->addLayout(reviewsButtonLayout);
    
    reviewsTable = new QTableWidget();
    reviewsTable->setColumnCount(6);
    reviewsTable->setHorizontalHeaderLabels({"商品ISBN", "商品名称", "买家", "评分", "评论内容", "评论时间"});
    reviewsTable->horizontalHeader()->setStretchLastSection(true);
    reviewsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reviewsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reviewsTable->setAlternatingRowColors(true);
    reviewsLayout->addWidget(reviewsTable);
    
    stackedWidget->addWidget(reviewsPage);
    
    // ===== 个人中心页面 =====
    profilePage = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profilePage);
    profileLayout->setSpacing(0);
    profileLayout->setContentsMargins(0, 0, 0, 0);

    // 顶部Banner区域
    profileBanner = new QWidget();
    profileBanner->setMinimumHeight(200);
    profileBanner->setMaximumHeight(200);
    profileBanner->setStyleSheet(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #667eea, stop:1 #764ba2);"
        "    border: none;"
        "}"
    );
    QHBoxLayout *bannerLayout = new QHBoxLayout(profileBanner);
    bannerLayout->setContentsMargins(30, 25, 30, 25);
    bannerLayout->setSpacing(20);

    // 左侧欢迎语
    profileWelcomeLabel = new QLabel("你好，商家！");
    profileWelcomeLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "}"
    );
    bannerLayout->addWidget(profileWelcomeLabel);
    bannerLayout->addStretch();

    // 右侧会员卡片
    memberCard = new QWidget();
    memberCard->setStyleSheet(
        "QWidget {"
        "    background-color: rgba(255, 255, 255, 0.95);"
        "    border-radius: 12px;"
        "    padding: 18px 22px;"
        "    min-width: 290px;"
        "    max-width: 290px;"
        "    min-height: 130px;"
        "    max-height: 130px;"
        "}"
    );
    // 会员卡片主布局：左侧信息，右侧按钮
    QHBoxLayout *memberCardMainLayout = new QHBoxLayout(memberCard);
    memberCardMainLayout->setContentsMargins(18, 12, 18, 12);
    memberCardMainLayout->setSpacing(12);

    // 左侧：会员等级和累计充值信息（垂直布局）
    QVBoxLayout *memberInfoLayout = new QVBoxLayout();
    memberInfoLayout->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->setSpacing(0);

    memberCardLabel = new QLabel("普通会员 10.0折");
    memberCardLabel->setStyleSheet(
        "QLabel {"
        "    color: #333333;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    memberCardLabel->setWordWrap(false);
    memberCardLabel->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->addWidget(memberCardLabel);

    // 累计充值金额标签（小字显示在会员等级下方）
    memberCardRechargeLabel = new QLabel("累计充值: ¥0.00");
    memberCardRechargeLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 12px;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "    margin-top: -2px;"
        "}"
    );
    memberCardRechargeLabel->setWordWrap(false);
    memberCardRechargeLabel->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->addWidget(memberCardRechargeLabel);
    memberInfoLayout->addStretch();

    memberCardMainLayout->addLayout(memberInfoLayout);

    // 右侧：查看会员等级规则按钮
    levelInfoBtn = new QPushButton("查看会员等级规则");
    levelInfoBtn->setFixedHeight(20);
    levelInfoBtn->setToolTip("点击查看会员等级说明");
    levelInfoBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border-radius: 5px;"
        "    font-weight: normal;"
        "    font-size: 10px;"
        "    padding: 2px 6px;"
        "    border: 1px solid #2980b9;"
        "    min-width: 80px;"
        "    max-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    memberCardMainLayout->addWidget(levelInfoBtn);

    bannerLayout->addWidget(memberCard);

    profileLayout->addWidget(profileBanner);

    QWidget *profileHeader = new QWidget();
    QHBoxLayout *profileHeaderLayout = new QHBoxLayout(profileHeader);

    backFromProfileBtn = new QPushButton("返回");
    profileHeaderLayout->addWidget(backFromProfileBtn);
    profileHeaderLayout->addStretch();

    QTabWidget *profileTabs = new QTabWidget();

    // 基本信息标签页
    QWidget *basicInfoTab = new QWidget();
    QHBoxLayout *basicInfoMainLayout = new QHBoxLayout(basicInfoTab);
    basicInfoMainLayout->setSpacing(20);
    basicInfoMainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 左侧：个人信息卡片
    QWidget *infoCard = new QWidget();
    infoCard->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "}"
    );
    QVBoxLayout *infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setSpacing(20);
    infoCardLayout->setContentsMargins(20, 20, 20, 20);
    infoCardLayout->setAlignment(Qt::AlignTop);
    
    // 创建输入框
    profileSellerNameEdit = new QLineEdit();
    profileSellerNameEdit->setReadOnly(true);  // 商家名称不能修改
    profilePhoneEdit = new QLineEdit();
    profileEmailEdit = new QLineEdit();
    profileAddressEdit = new QLineEdit();
    
    // 设置输入框样式：底部单线条或浅灰色填充块
    QString inputBoxStyle = 
        "QLineEdit {"
        "    background-color: #f5f5f5;"
        "    border: none;"
        "    border-bottom: 2px solid #3498db;"
        "    padding: 10px 5px;"
        "    font-size: 14px;"
        "    border-radius: 0px;"
        "    margin: 0px;"
        "}"
        "QLineEdit:focus {"
        "    background-color: #ffffff;"
        "    border-bottom: 2px solid #2980b9;"
        "}";
    
    profilePhoneEdit->setStyleSheet(inputBoxStyle);
    profileEmailEdit->setStyleSheet(inputBoxStyle);
    profileAddressEdit->setStyleSheet(inputBoxStyle);
    
    // 商家名称字段：标签在上，输入框在下
    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(8);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *nameLabel = new QLabel("商家名称");
    nameLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    nameLayout->addWidget(nameLabel);
    profileSellerNameEdit->setMinimumHeight(35);
    profileSellerNameEdit->setMaximumHeight(35);
    nameLayout->addWidget(profileSellerNameEdit);
    infoCardLayout->addLayout(nameLayout);
    
    // 电话字段：标签在上，输入框在下
    QVBoxLayout *phoneLayout = new QVBoxLayout();
    phoneLayout->setSpacing(8);
    phoneLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *phoneLabel = new QLabel("电话");
    phoneLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    phoneLayout->addWidget(phoneLabel);
    profilePhoneEdit->setMinimumHeight(35);
    profilePhoneEdit->setMaximumHeight(35);
    phoneLayout->addWidget(profilePhoneEdit);
    infoCardLayout->addLayout(phoneLayout);
    
    // 邮箱字段：标签在上，输入框在下
    QVBoxLayout *emailLayout = new QVBoxLayout();
    emailLayout->setSpacing(8);
    emailLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *emailLabel = new QLabel("邮箱");
    emailLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    emailLayout->addWidget(emailLabel);
    profileEmailEdit->setMinimumHeight(35);
    profileEmailEdit->setMaximumHeight(35);
    emailLayout->addWidget(profileEmailEdit);
    infoCardLayout->addLayout(emailLayout);
    
    // 地址字段：标签在上，输入框在下
    QVBoxLayout *addressLayout = new QVBoxLayout();
    addressLayout->setSpacing(8);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *addressLabel = new QLabel("地址");
    addressLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    addressLayout->addWidget(addressLabel);
    profileAddressEdit->setMinimumHeight(35);
    profileAddressEdit->setMaximumHeight(35);
    addressLayout->addWidget(profileAddressEdit);
    infoCardLayout->addLayout(addressLayout);
    
    // 添加间距
    infoCardLayout->addSpacing(20);
    
    // 更新信息按钮
    updateProfileBtn = new QPushButton("更新信息");
    updateProfileBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    updateProfileBtn->setFixedHeight(40);
    infoCardLayout->addWidget(updateProfileBtn);
    
    basicInfoMainLayout->addWidget(infoCard, 1);
    
    // 右侧：账户资产区域
    QWidget *assetCard = new QWidget();
    assetCard->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "}"
    );
    QVBoxLayout *assetLayout = new QVBoxLayout(assetCard);
    assetLayout->setSpacing(20);
    assetLayout->setContentsMargins(20, 20, 20, 20);
    assetLayout->setAlignment(Qt::AlignTop);
    
    // 账户余额：大数字显示
    QVBoxLayout *balanceLayout = new QVBoxLayout();
    balanceLayout->setSpacing(10);
    QLabel *balanceTitleLabel = new QLabel("账户余额");
    balanceTitleLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; }");
    balanceLayout->addWidget(balanceTitleLabel);
    
    profileBalanceLabel = new QLabel("¥0.00");
    profileBalanceLabel->setStyleSheet(
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 32px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    balanceLayout->addWidget(profileBalanceLabel);
    assetLayout->addLayout(balanceLayout);
    
    // 账户状态显示
    QVBoxLayout *statusLayout = new QVBoxLayout();
    statusLayout->setSpacing(10);
    QLabel *statusTitleLabel = new QLabel("账户状态");
    statusTitleLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; }");
    statusLayout->addWidget(statusTitleLabel);
    
    profileStatusLabel = new QLabel("正常");
    profileStatusLabel->setStyleSheet(
        "QLabel {"
        "    color: #27ae60;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    statusLayout->addWidget(profileStatusLabel);
    assetLayout->addLayout(statusLayout);
    
    // 积分显示：在余额下方
    QVBoxLayout *pointsLayout = new QVBoxLayout();
    pointsLayout->setSpacing(5);
    QLabel *pointsTitleLabel = new QLabel("积分");
    pointsTitleLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; }");
    pointsLayout->addWidget(pointsTitleLabel);
    profilePointsLabel = new QLabel("0");
    profilePointsLabel->setStyleSheet(
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    pointsLayout->addWidget(profilePointsLabel);
    assetLayout->addLayout(pointsLayout);
    
    basicInfoMainLayout->addWidget(assetCard, 1);
    
    profileTabs->addTab(basicInfoTab, "基本信息");
    
    // 申诉功能标签页
    QWidget *appealTab = new QWidget();
    QVBoxLayout *appealTabLayout = new QVBoxLayout(appealTab);
    appealTabLayout->setContentsMargins(20, 20, 20, 20);
    appealTabLayout->setSpacing(20);
    
    QGroupBox *appealGroup = new QGroupBox("申诉功能");
    appealGroup->setStyleSheet(
        "QGroupBox {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "    background-color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px;"
        "}"
    );
    QVBoxLayout *appealLayout = new QVBoxLayout(appealGroup);
    appealLayout->setSpacing(15);
    appealLayout->setContentsMargins(15, 20, 15, 15);
    
    QLabel *appealTitle = new QLabel("如果您认为账户被封禁有误，可以提交申诉：");
    appealTitle->setStyleSheet("QLabel { color: #666666; font-size: 14px; }");
    appealLayout->addWidget(appealTitle);
    
    QLabel *appealReasonLabel = new QLabel("申诉理由:");
    appealReasonLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; font-weight: bold; }");
    appealLayout->addWidget(appealReasonLabel);
    
    appealReasonEdit = new QTextEdit();
    appealReasonEdit->setPlaceholderText("请详细说明申诉理由...");
    appealReasonEdit->setMinimumHeight(120);
    appealReasonEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f5f5f5;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "}"
        "QTextEdit:focus {"
        "    background-color: #ffffff;"
        "    border: 1px solid #3498db;"
        "}"
    );
    appealLayout->addWidget(appealReasonEdit);
    
    QHBoxLayout *appealButtonLayout = new QHBoxLayout();
    submitAppealBtn = new QPushButton("提交申诉");
    submitAppealBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    submitAppealBtn->setFixedHeight(40);
    refreshAppealBtn = new QPushButton("刷新申诉状态");
    refreshAppealBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #7f8c8d;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #6c7a7b;"
        "}"
    );
    refreshAppealBtn->setFixedHeight(40);
    appealButtonLayout->addWidget(submitAppealBtn);
    appealButtonLayout->addWidget(refreshAppealBtn);
    appealButtonLayout->addStretch();
    appealLayout->addLayout(appealButtonLayout);
    
    // 申诉状态显示
    QLabel *appealStatusTitle = new QLabel("申诉状态:");
    appealStatusTitle->setStyleSheet("QLabel { color: #666666; font-size: 13px; font-weight: bold; margin-top: 10px; }");
    appealLayout->addWidget(appealStatusTitle);
    
    appealStatusLabel = new QLabel("暂无申诉记录");
    appealStatusLabel->setStyleSheet(
        "QLabel {"
        "    padding: 10px;"
        "    background-color: #ecf0f1;"
        "    border-radius: 4px;"
        "    color: #7f8c8d;"
        "    font-size: 14px;"
        "}"
    );
    appealLayout->addWidget(appealStatusLabel);
    
    QLabel *reviewCommentTitle = new QLabel("审核意见:");
    reviewCommentTitle->setStyleSheet("QLabel { color: #666666; font-size: 13px; font-weight: bold; margin-top: 10px; }");
    appealLayout->addWidget(reviewCommentTitle);
    
    appealReviewCommentLabel = new QLabel("-");
    appealReviewCommentLabel->setStyleSheet(
        "QLabel {"
        "    padding: 10px;"
        "    background-color: #ecf0f1;"
        "    border-radius: 4px;"
        "    color: #7f8c8d;"
        "    font-size: 14px;"
        "}"
    );
    appealReviewCommentLabel->setWordWrap(true);
    appealLayout->addWidget(appealReviewCommentLabel);
    
    appealTabLayout->addWidget(appealGroup);
    appealTabLayout->addStretch();
    
    profileTabs->addTab(appealTab, "申诉功能");
    
    // 添加客服聊天按钮到基本信息标签页
    QHBoxLayout *chatButtonLayout = new QHBoxLayout();
    chatButton = new QPushButton("联系客服");
    chatButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    chatButton->setFixedHeight(40);
    chatButtonLayout->addWidget(chatButton);
    chatButtonLayout->addStretch();
    infoCardLayout->addLayout(chatButtonLayout);
    
    profileHeaderLayout->addWidget(profileTabs);
    profileLayout->addWidget(profileHeader);
    
    stackedWidget->addWidget(profilePage);
    
    // ===== 聊天页面 =====
    // ===== 客服聊天页面（与管理员聊天）=====
    chatPage = new QWidget();
    QVBoxLayout *chatLayout = new QVBoxLayout(chatPage);
    chatLayout->setContentsMargins(15, 15, 15, 15);
    chatLayout->setSpacing(10);
    
    // 聊天页面标题
    QWidget *chatHeader = new QWidget();
    QHBoxLayout *chatHeaderLayout = new QHBoxLayout(chatHeader);
    backFromChatBtn = new QPushButton("返回");
    QLabel *chatTitleLabel = new QLabel("联系客服");
    chatTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    chatHeaderLayout->addWidget(backFromChatBtn);
    chatHeaderLayout->addStretch();
    chatHeaderLayout->addWidget(chatTitleLabel);
    chatHeaderLayout->addStretch();
    chatLayout->addWidget(chatHeader);
    
    // 聊天显示区域
    chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);
    chatDisplay->setMinimumHeight(400);
    chatDisplay->setStyleSheet("background-color: white; border: 1px solid #ddd; border-radius: 5px; padding: 10px;");
    chatLayout->addWidget(chatDisplay, 1);
    
    // 聊天输入区域
    QLabel *inputLabel = new QLabel("输入消息:");
    chatLayout->addWidget(inputLabel);
    
    chatInput = new QTextEdit();
    chatInput->setPlaceholderText("请输入您的消息...");
    chatInput->setMaximumHeight(100);
    chatInput->setStyleSheet("border: 1px solid #ddd; border-radius: 5px; padding: 5px;");
    chatLayout->addWidget(chatInput);
    
    // 发送按钮
    sendChatBtn = new QPushButton("发送");
    sendChatBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold; padding: 8px 20px;");
    chatLayout->addWidget(sendChatBtn, 0, Qt::AlignRight);
    
    stackedWidget->addWidget(chatPage);
    
    // ===== 客户消息页面（与买家聊天）=====
    buyerChatPage = new QWidget();
    QHBoxLayout *buyerChatMainLayout = new QHBoxLayout(buyerChatPage);
    buyerChatMainLayout->setContentsMargins(15, 15, 15, 15);
    buyerChatMainLayout->setSpacing(10);
    
    // 左侧：买家列表
    QWidget *buyerListContainer = new QWidget();
    buyerListContainer->setMaximumWidth(200);
    QVBoxLayout *buyerListLayout = new QVBoxLayout(buyerListContainer);
    QLabel *buyerListTitle = new QLabel("买家列表");
    buyerListTitle->setStyleSheet("font-size: 14px; font-weight: bold;");
    buyerListLayout->addWidget(buyerListTitle);
    buyerListWidget = new QListWidget();
    buyerListWidget->setStyleSheet("border: 1px solid #ddd; border-radius: 5px;");
    buyerListLayout->addWidget(buyerListWidget);
    QPushButton *refreshBuyerListBtn = new QPushButton("刷新列表");
    buyerListLayout->addWidget(refreshBuyerListBtn);
    connect(refreshBuyerListBtn, &QPushButton::clicked, this, &BookMerchant::loadBuyerList);
    connect(buyerListWidget, &QListWidget::itemClicked, this, &BookMerchant::onBuyerListItemClicked);
    
    // 右侧：聊天区域
    QWidget *buyerChatWidget = new QWidget();
    QVBoxLayout *buyerChatLayout = new QVBoxLayout(buyerChatWidget);
    buyerChatLayout->setContentsMargins(0, 0, 0, 0);
    buyerChatLayout->setSpacing(10);
    
    // 聊天页面标题
    QWidget *buyerChatHeader = new QWidget();
    QHBoxLayout *buyerChatHeaderLayout = new QHBoxLayout(buyerChatHeader);
    backFromBuyerChatBtn = new QPushButton("返回");
    currentBuyerLabel = new QLabel("请选择买家");
    currentBuyerLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    buyerChatHeaderLayout->addWidget(backFromBuyerChatBtn);
    buyerChatHeaderLayout->addStretch();
    buyerChatHeaderLayout->addWidget(currentBuyerLabel);
    buyerChatHeaderLayout->addStretch();
    buyerChatLayout->addWidget(buyerChatHeader);
    
    // 聊天显示区域
    buyerChatDisplay = new QTextEdit();
    buyerChatDisplay->setReadOnly(true);
    buyerChatDisplay->setMinimumHeight(400);
    buyerChatDisplay->setStyleSheet("background-color: white; border: 1px solid #ddd; border-radius: 5px; padding: 10px;");
    buyerChatLayout->addWidget(buyerChatDisplay, 1);
    
    // 聊天输入区域
    QLabel *buyerInputLabel = new QLabel("输入消息:");
    buyerChatLayout->addWidget(buyerInputLabel);
    
    buyerChatInput = new QTextEdit();
    buyerChatInput->setPlaceholderText("请输入您的消息...");
    buyerChatInput->setMaximumHeight(100);
    buyerChatInput->setStyleSheet("border: 1px solid #ddd; border-radius: 5px; padding: 5px;");
    buyerChatLayout->addWidget(buyerChatInput);
    
    // 发送按钮
    sendBuyerChatBtn = new QPushButton("发送");
    sendBuyerChatBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold; padding: 8px 20px;");
    buyerChatLayout->addWidget(sendBuyerChatBtn, 0, Qt::AlignRight);
    
    buyerChatMainLayout->addWidget(buyerListContainer);
    buyerChatMainLayout->addWidget(buyerChatWidget, 1);
    
    stackedWidget->addWidget(buyerChatPage);
}

void BookMerchant::initConnections()
{
    // 登录页面
    connect(loginButton, &QPushButton::clicked, this, &BookMerchant::onLoginClicked);

    // 主页面导航
    connect(mainPageRefreshButton, &QPushButton::clicked, this, &BookMerchant::updateDashboardData);
    connect(booksButton, &QPushButton::clicked, this, &BookMerchant::showBooksPage);
    connect(ordersButton, &QPushButton::clicked, this, &BookMerchant::showOrdersPage);
    connect(membersButton, &QPushButton::clicked, this, &BookMerchant::showMembersPage);
    connect(statsButton, &QPushButton::clicked, this, &BookMerchant::showStatsPage);
    connect(buyerChatButton, &QPushButton::clicked, this, &BookMerchant::showBuyerChatPage);
    connect(reviewsButton, &QPushButton::clicked, this, &BookMerchant::showReviewsPage);
    connect(logoutButton, &QPushButton::clicked, this, &BookMerchant::onLogoutClicked);

    // 图书管理
    connect(refreshBooksBtn, &QPushButton::clicked, this, &BookMerchant::onRefreshBooksClicked);
    connect(addBookBtn, &QPushButton::clicked, this, &BookMerchant::onAddBookClicked);
    connect(editBookBtn, &QPushButton::clicked, this, &BookMerchant::onEditBookClicked);
    connect(deleteBookBtn, &QPushButton::clicked, this, &BookMerchant::onDeleteBookClicked);
    connect(backFromBooksBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(booksTable, &QTableWidget::cellClicked, this, &BookMerchant::onBookTableCellClicked);

    // 订单管理
    connect(refreshOrdersBtn, &QPushButton::clicked, this, &BookMerchant::onRefreshOrdersClicked);
    connect(updateOrderStatusBtn, &QPushButton::clicked, this, &BookMerchant::onUpdateOrderStatusClicked);
    connect(deleteOrderBtn, &QPushButton::clicked, this, &BookMerchant::onDeleteOrderClicked);
    connect(backFromOrdersBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(ordersTable, &QTableWidget::cellClicked, this, &BookMerchant::onOrderTableCellClicked);

    // 会员管理
    connect(editMemberBtn, &QPushButton::clicked, this, &BookMerchant::onEditMemberClicked);
    connect(deleteMemberBtn, &QPushButton::clicked, this, &BookMerchant::onDeleteMemberClicked);
    connect(backFromMembersBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(membersTable, &QTableWidget::cellClicked, this, &BookMerchant::onMemberTableCellClicked);

    // 统计报表
    connect(refreshStatsBtn, &QPushButton::clicked, this, &BookMerchant::onRefreshStatsClicked);
    connect(generateSalesReportBtn, &QPushButton::clicked, this, &BookMerchant::onGenerateSalesReportClicked);
    connect(generateInventoryReportBtn, &QPushButton::clicked, this, &BookMerchant::onGenerateInventoryReportClicked);
    connect(generateMemberReportBtn, &QPushButton::clicked, this, &BookMerchant::onGenerateMemberReportClicked);
    connect(backFromStatsBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    
    // 评论管理
    connect(refreshReviewsBtn, &QPushButton::clicked, this, &BookMerchant::loadReviews);
    connect(backFromReviewsBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    
    // 个人中心
    connect(profileButton, &QPushButton::clicked, this, &BookMerchant::onProfileClicked);
    connect(updateProfileBtn, &QPushButton::clicked, this, &BookMerchant::onUpdateProfileClicked);
    connect(backFromProfileBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(submitAppealBtn, &QPushButton::clicked, this, &BookMerchant::onSubmitAppealClicked);
    connect(refreshAppealBtn, &QPushButton::clicked, this, &BookMerchant::onRefreshAppealClicked);
    connect(levelInfoBtn, &QPushButton::clicked, this, &BookMerchant::onLevelInfoClicked);
    
    // 客服聊天相关
    connect(chatButton, &QPushButton::clicked, this, &BookMerchant::showChatPage);
    connect(backFromChatBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(sendChatBtn, &QPushButton::clicked, this, &BookMerchant::onSendChatClicked);
    
    // 买家聊天相关
    connect(buyerChatButton, &QPushButton::clicked, this, &BookMerchant::showBuyerChatPage);
    connect(backFromBuyerChatBtn, &QPushButton::clicked, this, &BookMerchant::showMainPage);
    connect(sendBuyerChatBtn, &QPushButton::clicked, this, &BookMerchant::onSendBuyerChatClicked);
}

void BookMerchant::applyStyle()
{
    // 定义颜色常量（与买家保持一致）
    const QColor PRIMARY_COLOR(41, 128, 185);      // 主蓝色
    const QColor SECONDARY_COLOR(52, 152, 219);    // 次要蓝色
    const QColor ACCENT_COLOR(46, 204, 113);        // 强调绿色
    const QColor WARNING_COLOR(231, 76, 60);       // 警告红色
    const QColor BG_COLOR(245, 247, 250);           // 背景色
    const QColor TEXT_COLOR(44, 62, 80);           // 文本色
    const QColor BORDER_COLOR(225, 232, 237);      // 边框色
    
    QString styleSheet = QString(R"(
        /* 主窗口背景 */
        QMainWindow {
            background-color: %1;
        }
        
        /* 堆叠窗口 */
        QStackedWidget {
            background-color: %1;
        }
        
        /* 输入框统一样式 */
        QLineEdit {
            border: 2px solid %2;
            border-radius: 10px;
            padding: 12px 20px;
            font-size: 15px;
            background: #f8fafc;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QLineEdit:focus {
            border-color: %4;
            background: white;
        }
        
        QLineEdit:disabled {
            background: #e9ecef;
            color: #6c757d;
        }
        
        /* 按钮统一样式 */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %4,
                stop:1 %5);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: bold;
            padding: 12px 24px;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %6,
                stop:1 %7);
        }
        
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %8,
                stop:1 %9);
        }
        
        QPushButton:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
        
        /* 表格统一样式 */
        QTableWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            gridline-color: %2;
            font-size: 14px;
            color: %3;
        }
        
        QTableWidget::item {
            padding: 8px;
            border: none;
        }
        
        QTableWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        QHeaderView::section {
            background-color: #f8f9fa;
            color: %3;
            padding: 10px;
            border: none;
            border-bottom: 2px solid %2;
            font-weight: bold;
            font-size: 14px;
        }
        
        /* 列表控件统一样式 */
        QListWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 5px;
            font-size: 14px;
            color: %3;
        }
        
        QListWidget::item {
            padding: 10px;
            border-radius: 5px;
            margin: 2px;
        }
        
        QListWidget::item:hover {
            background-color: #f1f8ff;
        }
        
        QListWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        /* 树形控件统一样式 */
        QTreeWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            font-size: 14px;
            color: %3;
        }
        
        QTreeWidget::item {
            padding: 8px;
        }
        
        QTreeWidget::item:hover {
            background-color: #f1f8ff;
        }
        
        QTreeWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        /* 文本编辑框统一样式 */
        QTextEdit {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 10px;
            background: white;
            font-size: 14px;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QTextEdit:focus {
            border-color: %4;
        }
        
        /* 标签统一样式 */
        QLabel {
            color: %3;
            font-size: 14px;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        /* 组合框统一样式 */
        QComboBox {
            border: 2px solid %2;
            border-radius: 10px;
            padding: 10px 20px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QComboBox:focus {
            border-color: %4;
            background: white;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        
        QComboBox::down-arrow {
            width: 12px;
            height: 12px;
        }
        
        /* 复选框统一样式 */
        QCheckBox {
            font-size: 14px;
            color: %3;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %2;
            border-radius: 4px;
        }
        
        QCheckBox::indicator:checked {
            background-color: %4;
            border-color: %4;
        }
        
        /* 微调框统一样式 */
        QSpinBox {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 8px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
        }
        
        QSpinBox:focus {
            border-color: %4;
            background: white;
        }
        
        /* 双精度微调框统一样式 */
        QDoubleSpinBox {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 8px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
        }
        
        QDoubleSpinBox:focus {
            border-color: %4;
            background: white;
        }
        
        /* 日期选择器统一样式 */
        QDateEdit {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 8px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
        }
        
        QDateEdit:focus {
            border-color: %4;
            background: white;
        }
        
        /* 分组框统一样式 */
        QGroupBox {
            border: 2px solid %2;
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 15px;
            font-size: 16px;
            font-weight: bold;
            color: %3;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
            background-color: %1;
        }
        
        /* 滚动条统一样式 */
        QScrollBar:vertical {
            background: #f1f1f1;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background: %4;
            min-height: 30px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: %5;
        }
        
        QScrollBar:horizontal {
            background: #f1f1f1;
            height: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal {
            background: %4;
            min-width: 30px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background: %5;
        }
    )").arg(BG_COLOR.name())
      .arg(BORDER_COLOR.name())
      .arg(TEXT_COLOR.name())
      .arg(PRIMARY_COLOR.name())
      .arg(SECONDARY_COLOR.name())
      .arg(PRIMARY_COLOR.darker(120).name())
      .arg(SECONDARY_COLOR.darker(120).name())
      .arg(PRIMARY_COLOR.darker(130).name())
      .arg(SECONDARY_COLOR.darker(130).name());
    
    // 应用样式到主窗口
    this->setStyleSheet(styleSheet);
    
    // 登录页面特殊样式（渐变背景）
    if (loginPage) {
        loginPage->setStyleSheet(QString(R"(
            QWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 %1,
                    stop:0.5 %2,
                    stop:1 %3);
            }
        )").arg(PRIMARY_COLOR.name())
          .arg(SECONDARY_COLOR.name())
          .arg(PRIMARY_COLOR.darker(110).name()));
    }
}

// ===== 主界面组件创建函数 =====
QWidget* BookMerchant::createStatCard(const QString &icon, const QString &title, const QString &value, const QString &subtitle, const QString &color, QLabel **valueLabelPtr)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(QString(R"(
        QWidget {
            background-color: white;
            border-radius: 15px;
            border: 1px solid #e1e8ed;
        }
    )"));
    card->setMinimumHeight(140);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(10);
    
    // 图标和标题
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet(QString("font-size: 32px; color: %1;").arg(color));
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; font-weight: 500;");
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    cardLayout->addLayout(headerLayout);
    
    // 数值
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));
    cardLayout->addWidget(valueLabel);
    
    // 保存valueLabel的指针以便后续更新
    if (valueLabelPtr) {
        *valueLabelPtr = valueLabel;
    }
    
    // 副标题
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("font-size: 12px; color: #95a5a6;");
    cardLayout->addWidget(subtitleLabel);
    
    cardLayout->addStretch();
    return card;
}

QWidget* BookMerchant::createChartWidget(const QString &title, const QString &subtitle)
{
    QWidget *chartWidget = new QWidget();
    chartWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 15px;
            border: 1px solid #e1e8ed;
        }
    )");
    chartWidget->setMinimumHeight(300);
    
    QVBoxLayout *chartLayout = new QVBoxLayout(chartWidget);
    chartLayout->setContentsMargins(20, 20, 20, 20);
    chartLayout->setSpacing(15);
    
    // 标题
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    chartLayout->addWidget(titleLabel);
    
    // 副标题
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("font-size: 12px; color: #7f8c8d; margin-bottom: 10px;");
    chartLayout->addWidget(subtitleLabel);
    
    // 图表区域（使用自定义折线图组件）
    salesChartWidget = new SalesChartWidget();
    salesChartWidget->setStyleSheet(R"(
        SalesChartWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f8f9fa,
                stop:1 white);
            border: 1px solid #e1e8ed;
            border-radius: 10px;
        }
    )");
    salesChartWidget->setMinimumHeight(200);
    
    chartLayout->addWidget(salesChartWidget);
    chartLayout->addStretch();
    
    return chartWidget;
}

QWidget* BookMerchant::createOrderStatsWidget()
{
    QWidget *statsWidget = new QWidget();
    statsWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
                border-radius: 15px;
            border: 1px solid #e1e8ed;
        }
    )");
    statsWidget->setMinimumHeight(300);
    
    QVBoxLayout *statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->setContentsMargins(20, 20, 20, 20);
    statsLayout->setSpacing(15);
    
    // 标题
    QLabel *titleLabel = new QLabel("📋 订单统计");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    statsLayout->addWidget(titleLabel);
    
    // 订单状态列表
    QVBoxLayout *statusLayout = new QVBoxLayout();
    statusLayout->setSpacing(10);
    
    // 待处理订单
    QWidget *pendingItem = createStatusItem("⏳ 待处理", "0", "#f39c12", &pendingOrdersLabel);
    statusLayout->addWidget(pendingItem);
    
    // 已发货订单
    QWidget *shippedItem = createStatusItem("🚚 已发货", "0", "#3498db", &shippedOrdersLabel);
    statusLayout->addWidget(shippedItem);
    
    // 已完成订单
    QWidget *completedItem = createStatusItem("✅ 已完成", "0", "#2ecc71", &completedOrdersLabel);
    statusLayout->addWidget(completedItem);
    
    // 已取消订单
    QWidget *cancelledItem = createStatusItem("❌ 已取消", "0", "#e74c3c", &cancelledOrdersLabel);
    statusLayout->addWidget(cancelledItem);
    
    statsLayout->addLayout(statusLayout);
    statsLayout->addStretch();
    
    return statsWidget;
}

QWidget* BookMerchant::createStatusItem(const QString &label, const QString &value, const QString &color, QLabel **valueLabelPtr)
{
    QWidget *item = new QWidget();
    item->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border-radius: 8px;
            padding: 10px;
        }
    )");
    
    QHBoxLayout *itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(15, 10, 15, 10);
    
    QLabel *labelWidget = new QLabel(label);
    labelWidget->setStyleSheet("font-size: 14px; color: #2c3e50;");
    itemLayout->addWidget(labelWidget);
    
    itemLayout->addStretch();
    
    QLabel *valueWidget = new QLabel(value);
    valueWidget->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1;").arg(color));
    itemLayout->addWidget(valueWidget);
    
    // 保存valueWidget的指针以便后续更新
    if (valueLabelPtr) {
        *valueLabelPtr = valueWidget;
    }
    
    return item;
}

void BookMerchant::updateDashboardData()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        return;
    }
    
    // 先尝试从服务器获取统计数据
    QJsonObject response = apiService->getDashboardStats(currentSellerId);
    
    if (response["success"].toBool()) {
        // 解析服务器返回的统计数据
        QJsonObject stats = response["stats"].toObject();
        
        // 获取总数据
        int totalOrders = stats["totalOrders"].toInt(0);
        double totalSales = stats["totalSales"].toDouble(0.0);
        int totalBooks = stats["totalBooks"].toInt(0);
        int totalMembers = stats["totalMembers"].toInt(0);
        
        // 更新图书总数（直接使用服务器数据）
        if (booksValueLabel) {
            booksValueLabel->setText(QString::number(totalBooks));
        }
        
        // 计算今日订单、销量和收入（需要从订单数据中计算）
        // 强制重新加载订单数据以获取最新数据（不显示提示，不更新仪表板避免循环）
        loadOrders(false, false);  // 不显示提示，不更新仪表板
        
        int todayOrderCount = 0;
        int todaySalesCount = 0;
        double todayRevenueAmount = 0.0;
        
        if (ordersTable && ordersTable->rowCount() > 0) {
            QDate today = QDate::currentDate();
            QString todayStr = today.toString("yyyy-MM-dd");
            
            // 订单表格列：0-订单ID, 1-客户, 2-总金额, 3-状态, 4-下单时间, 5-发货时间
            for (int i = 0; i < ordersTable->rowCount(); ++i) {
                // 检查下单时间列（第4列，索引4）
                QTableWidgetItem *dateItem = ordersTable->item(i, 4);
                if (dateItem && dateItem->text().contains(todayStr)) {
                    todayOrderCount++;
                    
                    // 获取订单金额（第2列，索引2）
                    QTableWidgetItem *amountItem = ordersTable->item(i, 2);
                    if (amountItem) {
                        QString amountText = amountItem->text();
                        // 移除可能的货币符号和空格
                        amountText.remove("¥").remove("$").remove(",").remove(" ");
                        bool ok;
                        double amount = amountText.toDouble(&ok);
                        if (ok && amount > 0) {
                            todayRevenueAmount += amount;
                        }
                    }
                    
                    // 注意：订单表格中没有直接的数量列，销量需要从订单items中计算
                    // 这里暂时使用订单数量作为销量（每个订单算1个销量单位）
                    // 如果需要更精确的销量，需要从订单的items JSON中解析
                    todaySalesCount += 1;  // 每个订单算1个销量单位
                }
            }
        }
        
        // 更新今日订单数
        if (orderValueLabel) {
            orderValueLabel->setText(QString::number(todayOrderCount));
        }
        
        // 更新今日销量
        if (salesValueLabel) {
            salesValueLabel->setText(QString::number(todaySalesCount));
        }
        
        // 更新今日收入
        if (revenueValueLabel) {
            revenueValueLabel->setText(QString("¥%1").arg(todayRevenueAmount, 0, 'f', 2));
        }
        
        // 更新订单状态统计（从订单表格中统计）
        updateOrderStatusStats();
        
        // 更新销量趋势图
        updateSalesChart();
    } else {
        // 如果服务器没有返回数据，从本地数据计算
        // 先加载最新数据（不显示提示，不更新仪表板避免循环）
        loadBooks();
        loadOrders(false, false);  // 不显示提示，不更新仪表板
        
        // 从本地数据计算统计
        if (booksValueLabel && booksTable) {
            booksValueLabel->setText(QString::number(booksTable->rowCount()));
        }
        
        // 计算今日订单和销量、收入
        int todayOrderCount = 0;
        int todaySalesCount = 0;
        double todayRevenueAmount = 0.0;
        
        if (ordersTable) {
            QDate today = QDate::currentDate();
            QString todayStr = today.toString("yyyy-MM-dd");
            
            for (int i = 0; i < ordersTable->rowCount(); ++i) {
                // 查找日期列（通常在订单表格中）
                QTableWidgetItem *dateItem = nullptr;
                for (int col = 0; col < ordersTable->columnCount(); ++col) {
                    QTableWidgetItem *item = ordersTable->item(i, col);
                    if (item && item->text().contains(todayStr)) {
                        dateItem = item;
                        break;
                    }
                }
                
                if (dateItem) {
                    todayOrderCount++;
                    
                    // 尝试获取订单金额（通常在价格或金额列）
                    for (int col = 0; col < ordersTable->columnCount(); ++col) {
                        QTableWidgetItem *amountItem = ordersTable->item(i, col);
                        if (amountItem) {
                            QString amountText = amountItem->text();
                            // 移除可能的货币符号
                            amountText.remove("¥").remove("$").remove(",");
                            bool ok;
                            double amount = amountText.toDouble(&ok);
                            if (ok && amount > 0) {
                                todayRevenueAmount += amount;
                                break;
                            }
                        }
                    }
                    
                    // 尝试获取数量（用于计算销量）
                    for (int col = 0; col < ordersTable->columnCount(); ++col) {
                        QTableWidgetItem *qtyItem = ordersTable->item(i, col);
                        if (qtyItem) {
                            bool ok;
                            int qty = qtyItem->text().toInt(&ok);
                            if (ok && qty > 0) {
                                todaySalesCount += qty;
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        // 更新显示
        if (orderValueLabel) {
            orderValueLabel->setText(QString::number(todayOrderCount));
        }
        if (salesValueLabel) {
            salesValueLabel->setText(QString::number(todaySalesCount));
        }
        if (revenueValueLabel) {
            revenueValueLabel->setText(QString("¥%1").arg(todayRevenueAmount, 0, 'f', 2));
        }
        
        // 更新订单状态统计（从订单表格中统计）
        updateOrderStatusStats();
        
        // 更新销量趋势图
        updateSalesChart();
    }
}

void BookMerchant::updateOrderStatusStats()
{
    if (!ordersTable) {
        return;
    }
    
    // 初始化计数器
    int pendingCount = 0;
    int shippedCount = 0;
    int completedCount = 0;
    int cancelledCount = 0;
    
    // 订单表格列：0-订单ID, 1-客户, 2-总金额, 3-状态, 4-下单时间, 5-发货时间
    // 从订单表格中统计不同状态的订单数量
    for (int i = 0; i < ordersTable->rowCount(); ++i) {
        QTableWidgetItem *statusItem = ordersTable->item(i, 3);  // 状态列（第3列，索引3）
        if (statusItem) {
            QString status = statusItem->text().trimmed();
            
            // 待处理：包括待支付、已支付（等待发货）
            if (status == "待支付" || status == "已支付" || status == "待处理" || 
                status.contains("待") || status.contains("支付")) {
                pendingCount++;
            } 
            // 已发货：已发货、发货中
            else if (status == "已发货" || status == "发货中" || status.contains("发货")) {
                shippedCount++;
            } 
            // 已完成：已完成、完成、已收货（买家确认收货后状态变为已完成）
            else if (status == "已完成" || status == "完成" || status == "已收货" || status.contains("完成") || status.contains("收货")) {
                completedCount++;
            } 
            // 已取消：已取消、取消
            else if (status == "已取消" || status == "取消" || status.contains("取消")) {
                cancelledCount++;
            }
        }
    }
    
    // 更新标签显示
    if (pendingOrdersLabel) {
        pendingOrdersLabel->setText(QString::number(pendingCount));
    }
    if (shippedOrdersLabel) {
        shippedOrdersLabel->setText(QString::number(shippedCount));
    }
    if (completedOrdersLabel) {
        completedOrdersLabel->setText(QString::number(completedCount));
    }
    if (cancelledOrdersLabel) {
        cancelledOrdersLabel->setText(QString::number(cancelledCount));
    }
    
    qDebug() << "订单状态统计 - 待处理:" << pendingCount << "已发货:" << shippedCount 
             << "已完成:" << completedCount << "已取消:" << cancelledCount;
}

void BookMerchant::updateSalesChart()
{
    if (!isLoggedIn || currentSellerId.isEmpty() || !salesChartWidget) {
        qDebug() << "updateSalesChart: 未登录或salesChartWidget为空";
        return;
    }
    
    // 计算近7天的日期范围
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-6);  // 包括今天共7天
    
    QString startDateStr = startDate.toString("yyyy-MM-dd");
    QString endDateStr = endDate.toString("yyyy-MM-dd");
    
    qDebug() << "updateSalesChart: 获取销售数据 - 卖家ID:" << currentSellerId 
             << "日期范围:" << startDateStr << "到" << endDateStr;
    
    // 从服务器获取近7天的销售数据
    QJsonObject response = apiService->getSalesReport(
        currentSellerId,
        startDateStr,
        endDateStr
    );
    
    qDebug() << "updateSalesChart: 服务器响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    QVector<double> salesData;
    QVector<QString> dateLabels;
    
    if (response["success"].toBool()) {
        // 服务器返回的字段是"data"，不是"sales"
        QJsonArray salesArray = response["data"].toArray();
        if (salesArray.isEmpty() && response.contains("sales")) {
            // 兼容旧版本，如果"data"为空，尝试读取"sales"
            salesArray = response["sales"].toArray();
        }
        qDebug() << "updateSalesChart: 获取到" << salesArray.size() << "条销售记录";
        
        // 创建日期到销售额的映射
        QMap<QString, double> salesMap;
        for (const QJsonValue &value : salesArray) {
            QJsonObject item = value.toObject();
            QString date = item["date"].toString();
            double amount = item["amount"].toDouble(0.0);
            salesMap[date] = amount;
            qDebug() << "updateSalesChart: 日期" << date << "销售额:" << amount;
        }
        
        // 按日期顺序填充数据（近7天）
        for (int i = 0; i < 7; ++i) {
            QDate date = startDate.addDays(i);
            QString dateStr = date.toString("yyyy-MM-dd");
            
            // 显示具体日期（MM/dd格式），如果是今天则显示"今天"
            QString dateLabel;
            if (date == QDate::currentDate()) {
                dateLabel = "今天";
            } else {
                dateLabel = date.toString("MM/dd");
            }
            
            double amount = salesMap.value(dateStr, 0.0);
            dateLabels.append(dateLabel);
            salesData.append(amount);
            qDebug() << "updateSalesChart: 日期" << dateStr << "(" << dateLabel << ") 销售额:" << amount;
        }
    } else {
        QString errorMsg = response["message"].toString();
        if (errorMsg.isEmpty()) {
            errorMsg = response["error"].toString();
        }
        qDebug() << "updateSalesChart: 服务器返回失败:" << errorMsg;
        
        // 如果服务器没有返回数据，使用默认值（全0）
        for (int i = 0; i < 7; ++i) {
            QDate date = startDate.addDays(i);
            
            // 显示具体日期（MM/dd格式），如果是今天则显示"今天"
            QString dateLabel;
            if (date == QDate::currentDate()) {
                dateLabel = "今天";
            } else {
                dateLabel = date.toString("MM/dd");
            }
            
            dateLabels.append(dateLabel);
            salesData.append(0.0);
        }
    }
    
    qDebug() << "updateSalesChart: 最终数据 - 日期标签:" << dateLabels << "销售额:" << salesData;
    
    // 更新图表数据
    salesChartWidget->setSalesData(salesData, dateLabels);
    salesChartWidget->update();  // 触发重绘
}

// ===== 自定义折线图组件实现 =====
// 构造函数：初始化折线图组件
SalesChartWidget::SalesChartWidget(QWidget *parent)
    : QWidget(parent), maxValue(0.0), minValue(0.0)  // 调用父类构造函数，初始化最大值和最小值为0
{
    setMinimumSize(400, 200);  // 设置组件的最小尺寸为400x200像素，确保图表有足够的显示空间
}

// 设置销售数据：接收销售额数据和日期标签，并触发图表重绘
void SalesChartWidget::setSalesData(const QVector<double> &sales, const QVector<QString> &dates)
{
    salesData = sales;      // 保存销售额数据向量
    dateLabels = dates;     // 保存日期标签向量
    calculateScale();       // 根据新数据重新计算Y轴刻度范围
    update();               // 调用update()触发Qt重绘事件，刷新图表显示
}

// 计算Y轴刻度：根据数据范围智能计算合适的Y轴最大值和分度值
void SalesChartWidget::calculateScale()
{
    // 如果数据为空，设置默认的Y轴范围
    if (salesData.isEmpty()) {
        maxValue = 100.0;   // 默认最大值为100
        minValue = 0.0;     // 默认最小值为0
        return;             // 直接返回，不进行后续计算
    }
    
    // 使用STL算法查找销售额数据中的最大值和最小值
    double dataMax = *std::max_element(salesData.begin(), salesData.end());  // 获取销售额最大值
    double dataMin = *std::min_element(salesData.begin(), salesData.end());  // 获取销售额最小值
    
    // 如果所有值都是0，设置默认范围（避免除零错误和显示问题）
    if (dataMax == 0.0 && dataMin == 0.0) {
        maxValue = 100.0;   // 设置默认最大值为100，便于显示空数据状态
        minValue = 0.0;     // 最小值为0
        return;             // 直接返回
    }
    
    // 根据近七天最高销售额智能选择分度值
    // 目标：让图表显示4-8个刻度，分度值要合理（如10, 20, 50, 100, 200, 500, 1000等）
    double step = 10.0;     // 默认分度值为10，用于小数据量
    
    // 根据最大值选择合适的分度值，确保Y轴刻度清晰易读
    if (dataMax <= 50) {
        step = 10.0;        // 0-50范围：分度值10，显示0, 10, 20, 30, 40, 50
    } else if (dataMax <= 100) {
        step = 20.0;        // 50-100范围：分度值20，显示0, 20, 40, 60, 80, 100
    } else if (dataMax <= 200) {
        step = 50.0;        // 100-200范围：分度值50，显示0, 50, 100, 150, 200
    } else if (dataMax <= 500) {
        step = 100.0;       // 200-500范围：分度值100，显示0, 100, 200, 300, 400, 500
    } else if (dataMax <= 1000) {
        step = 200.0;       // 500-1000范围：分度值200，显示0, 200, 400, 600, 800, 1000
    } else if (dataMax <= 2000) {
        step = 500.0;       // 1000-2000范围：分度值500，显示0, 500, 1000, 1500, 2000
    } else if (dataMax <= 5000) {
        step = 1000.0;      // 2000-5000范围：分度值1000，显示0, 1000, 2000, 3000, 4000, 5000
    } else if (dataMax <= 10000) {
        step = 2000.0;      // 5000-10000范围：分度值2000，显示0, 2000, 4000, 6000, 8000, 10000
    } else {
        step = 5000.0;      // 超过10000：分度值5000，适用于大额销售额
    }
    
    // 计算合适的最大值（向上取整到分度值的倍数，并添加10%的余量）
    // 例如：dataMax=85, step=20, 则maxValue = ceil(85*1.1/20)*20 = ceil(4.675)*20 = 5*20 = 100
    maxValue = ceil((dataMax * 1.1) / step) * step;
    
    // 确保至少显示3-4个刻度，避免图表过于紧凑
    if (maxValue < step * 3) {
        maxValue = step * 3;  // 如果计算出的最大值太小，至少保证3个刻度
    }
    
    // 最小值始终为0（销售额不能为负值）
    minValue = 0.0;
}

// 绘制事件：Qt框架在需要重绘组件时自动调用此函数
void SalesChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);        // 标记event参数未使用，避免编译器警告
    
    // 创建QPainter对象，用于在组件上绘制图形
    QPainter painter(this);
    // 启用抗锯齿渲染，使线条和图形更加平滑
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 获取组件的宽度和高度（像素）
    int width = this->width();
    int height = this->height();
    
    // 设置图表边距（为坐标轴、标签预留空间）
    int marginLeft = 50;    // 左边距：用于Y轴和标签
    int marginRight = 20;   // 右边距：防止内容贴边
    int marginTop = 30;     // 上边距：为数据标签预留空间
    int marginBottom = 40;   // 下边距：用于X轴标签
    
    // 计算实际图表绘制区域的宽度和高度
    int chartWidth = width - marginLeft - marginRight;   // 图表宽度 = 总宽度 - 左右边距
    int chartHeight = height - marginTop - marginBottom; // 图表高度 = 总高度 - 上下边距
    
    // 绘制背景：使用浅灰色填充整个组件区域
    painter.fillRect(rect(), QColor(248, 249, 250));
    
    // 检查数据是否为空，如果为空则显示提示信息
    if (salesData.isEmpty() || dateLabels.isEmpty()) {
        // 绘制提示文字："暂无数据"
        painter.setPen(QColor(127, 140, 141));              // 设置文字颜色为灰色
        painter.setFont(QFont("Microsoft YaHei", 12));      // 设置字体为微软雅黑，大小12
        painter.drawText(rect(), Qt::AlignCenter, "暂无数据"); // 在组件中心绘制文字
        return;                                              // 直接返回，不绘制图表
    }
    
    // 计算Y轴刻度（根据maxValue选择合适的分度值，与calculateScale中的逻辑保持一致）
    double step = 10.0;     // 默认分度值为10

    // 根据maxValue选择合适的分度值（与calculateScale中的逻辑保持一致）
    // 确保绘制时的分度值与计算时的分度值一致
    if (maxValue <= 50) {
        step = 10.0;        // 最大值≤50时，分度值为10
    } else if (maxValue <= 100) {
        step = 20.0;        // 最大值≤100时，分度值为20
    } else if (maxValue <= 200) {
        step = 50.0;        // 最大值≤200时，分度值为50
    } else if (maxValue <= 500) {
        step = 100.0;       // 最大值≤500时，分度值为100
    } else if (maxValue <= 1000) {
        step = 200.0;       // 最大值≤1000时，分度值为200
    } else if (maxValue <= 2000) {
        step = 500.0;      // 最大值≤2000时，分度值为500
    } else if (maxValue <= 5000) {
        step = 1000.0;     // 最大值≤5000时，分度值为1000
    } else if (maxValue <= 10000) {
        step = 2000.0;     // 最大值≤10000时，分度值为2000
    } else {
        step = 5000.0;     // 最大值>10000时，分度值为5000
    }
    
    // 计算需要显示的刻度数量（向上取整）
    int numSteps = static_cast<int>(std::ceil(maxValue / step));  // 刻度数量 = 最大值/分度值（向上取整）
    if (numSteps < 3) {
        numSteps = 3;      // 至少显示3个刻度，保证图表可读性
    }
    
    // 绘制网格线和Y轴标签
    painter.setPen(QPen(QColor(225, 232, 237), 1));  // 设置网格线颜色为浅灰色，线宽1像素
    painter.setFont(QFont("Microsoft YaHei", 9));    // 设置Y轴标签字体为微软雅黑，大小9
    
    // 循环绘制每个刻度对应的网格线和标签
    for (int i = 0; i <= numSteps; ++i) {
        double value = i * step;      // 计算当前刻度的数值（0, step, 2*step, ...）
        if (value > maxValue) {
            break;                    // 如果超过最大值，停止绘制
        }
        
        // 计算当前刻度在屏幕上的Y坐标
        // 公式：y = 上边距 + 图表高度 - (图表高度 * 当前值 / 最大值)
        // 因为屏幕坐标系Y轴向下，所以需要从底部向上计算
        int y = marginTop + chartHeight - (chartHeight * value / maxValue);
        
        // 绘制水平网格线（从左边界到右边界）
        painter.drawLine(marginLeft, y, marginLeft + chartWidth, y);
        
        // 绘制Y轴标签（显示数值）
        QString label = QString::number(value, 'f', 0);  // 将数值转换为字符串，不显示小数
        painter.setPen(QColor(127, 140, 141));          // 设置标签文字颜色为灰色
        // 在Y轴左侧绘制标签，右对齐，垂直居中
        painter.drawText(0, y - 10, marginLeft - 10, 20, Qt::AlignRight | Qt::AlignVCenter, label);
        painter.setPen(QPen(QColor(225, 232, 237), 1)); // 恢复网格线画笔
    }
    
    // 绘制折线图（至少需要2个数据点才能绘制折线）
    if (salesData.size() > 1) {
        QPainterPath path;           // 创建路径对象，用于绘制平滑的折线
        QVector<QPointF> points;      // 存储所有数据点的坐标
        
        // 计算每个数据点在屏幕上的坐标
        for (int i = 0; i < salesData.size(); ++i) {
            // 计算X坐标：从左边界开始，根据数据点索引均匀分布
            // 公式：x = 左边距 + (图表宽度 * 索引 / (数据点总数-1))
            // 例如：7个数据点，索引0-6，x坐标从左边界均匀分布到右边界
            double x = marginLeft + (chartWidth * i / (salesData.size() - 1));
            
            // 使用maxValue作为基准计算Y坐标（minValue始终为0）
            // 将销售额归一化到0-1范围：normalizedValue = 销售额 / 最大值
            double normalizedValue = salesData[i] / maxValue;
            
            // 计算Y坐标：从底部向上计算
            // 公式：y = 上边距 + 图表高度 - (图表高度 * 归一化值)
            // 例如：normalizedValue=0时，y在底部；normalizedValue=1时，y在顶部
            double y = marginTop + chartHeight - (chartHeight * normalizedValue);
            
            // 将计算出的坐标点添加到向量中
            points.append(QPointF(x, y));
        }
        
        // 绘制折线：使用蓝色粗线连接所有数据点
        painter.setPen(QPen(QColor(41, 128, 185), 3));  // 设置画笔为蓝色，线宽3像素
        path.moveTo(points[0]);                           // 将路径起点移动到第一个数据点
        // 依次连接所有数据点，形成折线
        for (int i = 1; i < points.size(); ++i) {
            path.lineTo(points[i]);                      // 从上一个点画线到当前点
        }
        painter.drawPath(path);                          // 绘制完整的路径（折线）
        
        // 绘制数据点：在每个数据点位置绘制圆形标记
        painter.setBrush(QColor(41, 128, 185));         // 设置填充颜色为蓝色
        painter.setPen(QPen(QColor(255, 255, 255), 2)); // 设置边框为白色，线宽2像素
        // 遍历所有数据点，绘制圆形
        for (const QPointF &point : points) {
            painter.drawEllipse(point, 5, 5);             // 以point为中心，绘制半径为5的圆形
        }
        
        // 绘制数据标签（在数据点上方显示销售额数值）
        painter.setPen(QColor(44, 62, 80));             // 设置标签文字颜色为深灰色
        painter.setFont(QFont("Microsoft YaHei", 8));   // 设置字体为微软雅黑，大小8
        // 遍历所有数据点，在点上方绘制数值标签
        for (int i = 0; i < points.size(); ++i) {
            QString valueText = QString::number(salesData[i], 'f', 0);  // 将销售额转换为字符串
            // 定义标签文本的矩形区域（在数据点上方，宽度50，高度20）
            QRectF textRect(points[i].x() - 25, points[i].y() - 25, 50, 20);
            // 在矩形区域中心绘制文本
            painter.drawText(textRect, Qt::AlignCenter, valueText);
        }
    }
    
    // 绘制X轴标签（日期）：在X轴下方显示日期信息
    painter.setPen(QColor(127, 140, 141));             // 设置日期标签颜色为灰色
    painter.setFont(QFont("Microsoft YaHei", 9));       // 设置字体为微软雅黑，大小9
    // 遍历所有日期标签，在X轴下方绘制
    for (int i = 0; i < dateLabels.size(); ++i) {
        // 计算日期标签的X坐标（与数据点的X坐标对齐）
        // 使用qMax防止除零错误：如果只有一个数据点，使用1作为除数
        double x = marginLeft + (chartWidth * i / qMax(1, salesData.size() - 1));
        // 定义标签文本的矩形区域（在X轴下方，宽度60，高度20）
        QRectF textRect(x - 30, height - marginBottom, 60, 20);
        // 在矩形区域中心绘制日期文本
        painter.drawText(textRect, Qt::AlignCenter, dateLabels[i]);
    }
    
    // 绘制坐标轴：使用深色粗线绘制X轴和Y轴
    painter.setPen(QPen(QColor(44, 62, 80), 2));       // 设置坐标轴颜色为深灰色，线宽2像素
    // 绘制Y轴：从顶部到底部的垂直线
    painter.drawLine(marginLeft, marginTop, marginLeft, height - marginBottom);
    // 绘制X轴：从左边界到右边界的水平线
    painter.drawLine(marginLeft, height - marginBottom, width - marginRight, height - marginBottom);
}

// ===== 登录相关 =====
void BookMerchant::onLoginClicked()
{
    QString username = loginUsername->text().trimmed();
    QString password = loginPassword->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码！");
        return;
    }

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        loginStatusLabel->setText("正在连接服务器...");
        loginStatusLabel->setStyleSheet("color: blue;");
        
        if (!apiService->connectToServer(serverIp, serverPort)) {
            loginStatusLabel->setText("✗ 连接服务器失败");
            loginStatusLabel->setStyleSheet("color: red;");
            QMessageBox::warning(this, "连接失败", 
                QString("无法连接到服务器 %1:%2\n请确保服务器正在运行").arg(serverIp).arg(serverPort));
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
        loginStatusLabel->setText("✓ 已连接服务器");
        loginStatusLabel->setStyleSheet("color: green;");
    }

    // 通过TCP请求登录
    qDebug() << "=== 商家端登录请求 ===";
    qDebug() << "用户名:" << username;
    QJsonObject response = apiService->login(username, password);
    
    qDebug() << "服务器响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    if (response["success"].toBool()) {
        // 使用toInt()获取userId，更可靠
        int userId = response["userId"].toInt();
        QString userIdStr = QString::number(userId);
        QString userType = response["userType"].toString();
        
        qDebug() << "登录成功，userId(整数):" << userId << "userId(字符串):" << userIdStr << "userType:" << userType;
        
        // 检查是否是商家账号
        if (userType != "seller") {
            qDebug() << "✗ 登录失败：返回的用户类型不是seller，而是:" << userType;
            loginStatusLabel->setText("✗ 登录失败");
            loginStatusLabel->setStyleSheet("color: red;");
            QMessageBox::warning(this, "登录失败", 
                QString("登录失败：该账号不是商家账号（用户类型：%1）。\n请使用商家账号登录。").arg(userType));
            return;
        }
        
        // 检查userId是否有效（必须是大于0的整数）
        if (userId <= 0) {
            qDebug() << "✗ 登录失败：userId无效（必须大于0），当前值:" << userId;
            loginStatusLabel->setText("✗ 登录失败");
            loginStatusLabel->setStyleSheet("color: red;");
            QMessageBox::warning(this, "登录失败", 
                QString("登录失败：商家ID无效（ID: %1），请检查账号信息或联系管理员").arg(userId));
            return;
        }
        
        // 商家登录成功，设置相关信息
        currentSellerId = userIdStr;
        currentSellerName = username;
        isLoggedIn = true;
        
        loginUsername->clear();
        loginPassword->clear();
        
        welcomeLabel->setText("欢迎, " + currentSellerName + "！");
        showMainPage();
        
        // 启动仪表板数据刷新定时器
        if (dashboardRefreshTimer) {
            dashboardRefreshTimer->start();
        }
        
        QMessageBox::information(this, "成功", "登录成功！");
        qDebug() << "✓ 商家登录成功，已设置currentSellerId:" << currentSellerId;
    } else {
        QString error = response["error"].toString();
        if (error.isEmpty()) {
            error = response["message"].toString();
        }
        if (error.isEmpty()) {
            error = "未知错误";
        }
        loginStatusLabel->setText("✗ 登录失败");
        loginStatusLabel->setStyleSheet("color: red;");
        QMessageBox::warning(this, "登录失败", "登录失败：" + error);
        qDebug() << "✗ 登录失败：" << error;
    }
}

void BookMerchant::onLogoutClicked()
{
    // 停止仪表板数据刷新定时器
    if (dashboardRefreshTimer) {
        dashboardRefreshTimer->stop();
    }
    
    isLoggedIn = false;
    currentSellerId.clear();
    currentSellerName.clear();
    apiService->disconnectFromServer();
    showLoginPage();
    QMessageBox::information(this, "提示", "已退出登录");
}

// ===== 图书管理 =====
void BookMerchant::onRefreshBooksClicked()
{
    loadBooks();
}

void BookMerchant::loadBooks()
{
    QJsonObject response = apiService->getSellerBooks(currentSellerId);
    
    if (response["success"].toBool()) {
        QJsonArray books = response["books"].toArray();
        
        booksTable->setRowCount(0);
        for (const QJsonValue &bookValue : books) {
            QJsonObject book = bookValue.toObject();
            int row = booksTable->rowCount();
            booksTable->insertRow(row);
            
            booksTable->setItem(row, 0, new QTableWidgetItem(book["isbn"].toString()));
            booksTable->setItem(row, 1, new QTableWidgetItem(book["title"].toString()));
            booksTable->setItem(row, 2, new QTableWidgetItem(book["author"].toString()));
            // 兼容旧数据：优先使用category1，如果没有则使用category
            QString category1 = book.contains("category1") ? book["category1"].toString() : 
                                (book.contains("category") ? book["category"].toString() : "");
            QString category2 = book.contains("category2") ? book["category2"].toString() : 
                                (book.contains("subCategory") ? book["subCategory"].toString() : "");
            booksTable->setItem(row, 3, new QTableWidgetItem(category1));
            booksTable->setItem(row, 4, new QTableWidgetItem(category2));
            booksTable->setItem(row, 5, new QTableWidgetItem(QString::number(book["price"].toDouble(), 'f', 2)));
            booksTable->setItem(row, 6, new QTableWidgetItem(QString::number(book["stock"].toInt())));
            // 显示销量，如果服务器返回了sales字段则使用，否则默认为0
            int sales = book.contains("sales") ? book["sales"].toInt() : 0;
            booksTable->setItem(row, 7, new QTableWidgetItem(QString::number(sales)));
            // 显示状态，如果为空则显示"待审核"
            QString status = book["status"].toString();
            if (status.isEmpty()) {
                status = "待审核";
            }
            booksTable->setItem(row, 8, new QTableWidgetItem(status));
        }
    } else {
        QMessageBox::warning(this, "错误", "加载图书失败：" + response["message"].toString());
    }
    
    // 更新仪表板数据（图书数量可能已变化）
    updateDashboardData();
}

void BookMerchant::onAddBookClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加图书");
    dialog.setMinimumWidth(400);
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    QLineEdit *idEdit = new QLineEdit();
    QLineEdit *titleEdit = new QLineEdit();
    QLineEdit *authorEdit = new QLineEdit();
    
    // 一级分类下拉框
    QComboBox *categoryCombo = new QComboBox();
    categoryCombo->addItem("文学小说");
    categoryCombo->addItem("人文社科");
    categoryCombo->addItem("经济管理");
    categoryCombo->addItem("科学技术");
    categoryCombo->addItem("教育考试");
    categoryCombo->addItem("生活艺术");
    categoryCombo->addItem("少儿童书");
    categoryCombo->addItem("其他");
    
    // 二级分类下拉框（根据一级分类动态更新）
    QComboBox *subCategoryCombo = new QComboBox();
    
    // 定义分类映射：一级分类 -> 二级分类列表
    QMap<QString, QStringList> categoryMap;
    categoryMap["文学小说"] = QStringList() << "当代小说" << "悬疑/推理" << "科幻/奇幻" << "中国古典文学" 
                                             << "外国文学" << "武侠/仙侠" << "散文/随笔" << "诗歌/戏剧" << "其他";
    categoryMap["人文社科"] = QStringList() << "历史（中国史/世界史）" << "哲学/宗教" << "心理学" << "政治/军事" 
                                             << "法律" << "社会科学" << "文化/人类学" << "传记/回忆录" << "其他";
    categoryMap["经济管理"] = QStringList() << "经济学理论" << "企业管理" << "投资理财" << "市场营销" 
                                             << "职场励志" << "会计/金融" << "电子商务" << "名人传记（商业）" << "其他";
    categoryMap["科学技术"] = QStringList() << "计算机/互联网" << "科普读物" << "物理学" << "数学" 
                                             << "化学" << "医学/卫生" << "建筑/工程" << "自然科学" << "其他";
    categoryMap["教育考试"] = QStringList() << "中小学教辅" << "外语学习" << "考试/考证（公考/考研）" 
                                             << "教材/课本" << "工具书/字典" << "职业培训" << "其他";
    categoryMap["生活艺术"] = QStringList() << "烹饪/美食" << "旅游/地图" << "两性/情感" << "家居/园艺" 
                                             << "运动/健身" << "绘画/书法" << "摄影/设计" << "音乐/影视" << "其他";
    categoryMap["少儿童书"] = QStringList() << "0-2岁启蒙" << "3-6岁绘本" << "7-10岁科普" << "儿童文学" 
                                             << "少儿英语" << "动漫/卡通" << "其他";
    categoryMap["其他"] = QStringList();  // 其他大类没有子分类
    
    // 一级分类改变时，更新二级分类下拉框
    QObject::connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                     [&](int index) {
        QString category1 = categoryCombo->itemText(index);
        subCategoryCombo->clear();
        if (categoryMap.contains(category1)) {
            subCategoryCombo->addItems(categoryMap[category1]);
        }
    });
    
    // 初始化二级分类（默认选择第一个一级分类）
    subCategoryCombo->addItems(categoryMap["文学小说"]);
    
    QDoubleSpinBox *priceEdit = new QDoubleSpinBox();
    priceEdit->setRange(0, 9999.99);
    priceEdit->setDecimals(2);
    QSpinBox *stockEdit = new QSpinBox();
    stockEdit->setRange(0, 999999);
    QTextEdit *descEdit = new QTextEdit();
    descEdit->setMaximumHeight(100);
    
    // 封面图片选择
    QLabel *coverImageLabel = new QLabel("未选择图片");
    coverImageLabel->setMinimumSize(150, 200);
    coverImageLabel->setMaximumSize(150, 200);
    coverImageLabel->setAlignment(Qt::AlignCenter);
    coverImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f5f5f5;");
    coverImageLabel->setScaledContents(true);
    
    QPushButton *selectImageBtn = new QPushButton("选择封面图片");
    QString coverImageBase64;  // 存储选中的图片的Base64编码
    
    QObject::connect(selectImageBtn, &QPushButton::clicked, [&]() {
        QString imagePath = QFileDialog::getOpenFileName(&dialog, "选择封面图片", "", 
                                                         "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!imagePath.isEmpty()) {
            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                // 缩放图片以适应显示
                QPixmap scaledPixmap = pixmap.scaled(150, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverImageLabel->setPixmap(scaledPixmap);
                
                // 将图片转换为Base64编码
                QImage image = pixmap.toImage();
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");  // 保存为PNG格式
                coverImageBase64 = byteArray.toBase64();
            }
        }
    });
    
    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(coverImageLabel);
    imageLayout->addWidget(selectImageBtn);
    
    form->addRow("ISBN:", idEdit);
    form->addRow("书名:", titleEdit);
    form->addRow("作者:", authorEdit);
    form->addRow("一级分类:", categoryCombo);
    form->addRow("二级分类:", subCategoryCombo);
    form->addRow("价格:", priceEdit);
    form->addRow("库存:", stockEdit);
    form->addRow("描述:", descEdit);
    form->addRow("封面图片:", imageLayout);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addRow(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject bookData;
        bookData["isbn"] = idEdit->text();
        bookData["title"] = titleEdit->text();
        bookData["author"] = authorEdit->text();
        bookData["category1"] = categoryCombo->currentText();
        bookData["category2"] = subCategoryCombo->currentText();
        bookData["price"] = priceEdit->value();
        bookData["stock"] = stockEdit->value();
        bookData["status"] = "正常";
        bookData["description"] = descEdit->toPlainText();  // 书籍描述
        // 如果选择了图片，则添加；否则为空字符串（服务器会使用默认图片）
        bookData["coverImage"] = coverImageBase64;
        
        QJsonObject response = apiService->addBook(currentSellerId, bookData);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "图书添加成功！");
            loadBooks();
        } else {
            QMessageBox::warning(this, "错误", "添加图书失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onEditBookClicked()
{
    if (selectedBookRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的图书！");
        return;
    }
    
    QString bookId = booksTable->item(selectedBookRow, 0)->text();
    
    QDialog dialog(this);
    dialog.setWindowTitle("编辑图书");
    dialog.setMinimumWidth(400);
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    QLineEdit *titleEdit = new QLineEdit(booksTable->item(selectedBookRow, 1)->text());
    QLineEdit *authorEdit = new QLineEdit(booksTable->item(selectedBookRow, 2)->text());
    
    // 获取当前图书的分类
    QString currentCategory1 = booksTable->item(selectedBookRow, 3)->text();
    QString currentCategory2 = booksTable->item(selectedBookRow, 4)->text();
    
    // 一级分类下拉框
    QComboBox *categoryCombo = new QComboBox();
    categoryCombo->addItem("文学小说");
    categoryCombo->addItem("人文社科");
    categoryCombo->addItem("经济管理");
    categoryCombo->addItem("科学技术");
    categoryCombo->addItem("教育考试");
    categoryCombo->addItem("生活艺术");
    categoryCombo->addItem("少儿童书");
    categoryCombo->addItem("其他");
    
    // 设置当前选中的一级分类
    int categoryIndex = categoryCombo->findText(currentCategory1);
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
    
    // 二级分类下拉框（根据一级分类动态更新）
    QComboBox *subCategoryCombo = new QComboBox();
    
    // 定义分类映射：一级分类 -> 二级分类列表
    QMap<QString, QStringList> categoryMap;
    categoryMap["文学小说"] = QStringList() << "当代小说" << "悬疑/推理" << "科幻/奇幻" << "中国古典文学" 
                                             << "外国文学" << "武侠/仙侠" << "散文/随笔" << "诗歌/戏剧" << "其他";
    categoryMap["人文社科"] = QStringList() << "历史（中国史/世界史）" << "哲学/宗教" << "心理学" << "政治/军事" 
                                             << "法律" << "社会科学" << "文化/人类学" << "传记/回忆录" << "其他";
    categoryMap["经济管理"] = QStringList() << "经济学理论" << "企业管理" << "投资理财" << "市场营销" 
                                             << "职场励志" << "会计/金融" << "电子商务" << "名人传记（商业）" << "其他";
    categoryMap["科学技术"] = QStringList() << "计算机/互联网" << "科普读物" << "物理学" << "数学" 
                                             << "化学" << "医学/卫生" << "建筑/工程" << "自然科学" << "其他";
    categoryMap["教育考试"] = QStringList() << "中小学教辅" << "外语学习" << "考试/考证（公考/考研）" 
                                             << "教材/课本" << "工具书/字典" << "职业培训" << "其他";
    categoryMap["生活艺术"] = QStringList() << "烹饪/美食" << "旅游/地图" << "两性/情感" << "家居/园艺" 
                                             << "运动/健身" << "绘画/书法" << "摄影/设计" << "音乐/影视" << "其他";
    categoryMap["少儿童书"] = QStringList() << "0-2岁启蒙" << "3-6岁绘本" << "7-10岁科普" << "儿童文学" 
                                             << "少儿英语" << "动漫/卡通" << "其他";
    categoryMap["其他"] = QStringList();  // 其他大类没有子分类
    
    // 初始化二级分类（根据当前一级分类）
    if (categoryMap.contains(currentCategory1)) {
        subCategoryCombo->addItems(categoryMap[currentCategory1]);
        int subCategoryIndex = subCategoryCombo->findText(currentCategory2);
        if (subCategoryIndex >= 0) {
            subCategoryCombo->setCurrentIndex(subCategoryIndex);
        }
    }
    
    // 一级分类改变时，更新二级分类下拉框
    QObject::connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                     [&](int index) {
        QString category1 = categoryCombo->itemText(index);
        subCategoryCombo->clear();
        if (categoryMap.contains(category1)) {
            subCategoryCombo->addItems(categoryMap[category1]);
        }
    });
    
    QDoubleSpinBox *priceEdit = new QDoubleSpinBox();
    priceEdit->setRange(0, 9999.99);
    priceEdit->setDecimals(2);
    priceEdit->setValue(booksTable->item(selectedBookRow, 5)->text().toDouble());
    QSpinBox *stockEdit = new QSpinBox();
    stockEdit->setRange(0, 999999);
    stockEdit->setValue(booksTable->item(selectedBookRow, 6)->text().toInt());
    
    // 描述输入框（编辑时，初始为空，需要从服务器获取或留空）
    QTextEdit *descEdit = new QTextEdit();
    descEdit->setMaximumHeight(100);
    descEdit->setPlaceholderText("请输入书籍描述...");
    
    // 封面图片选择（编辑时）
    QLabel *coverImageLabel = new QLabel("未选择图片");
    coverImageLabel->setMinimumSize(150, 200);
    coverImageLabel->setMaximumSize(150, 200);
    coverImageLabel->setAlignment(Qt::AlignCenter);
    coverImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f5f5f5;");
    coverImageLabel->setScaledContents(true);
    
    QPushButton *selectImageBtn = new QPushButton("选择封面图片");
    QString coverImageBase64;  // 存储选中的图片的Base64编码
    
    QObject::connect(selectImageBtn, &QPushButton::clicked, [&]() {
        QString imagePath = QFileDialog::getOpenFileName(&dialog, "选择封面图片", "", 
                                                         "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!imagePath.isEmpty()) {
            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                // 缩放图片以适应显示
                QPixmap scaledPixmap = pixmap.scaled(150, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                coverImageLabel->setPixmap(scaledPixmap);
                
                // 将图片转换为Base64编码
                QImage image = pixmap.toImage();
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");  // 保存为PNG格式
                coverImageBase64 = byteArray.toBase64();
            }
        }
    });
    
    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(coverImageLabel);
    imageLayout->addWidget(selectImageBtn);
    
    form->addRow("书名:", titleEdit);
    form->addRow("作者:", authorEdit);
    form->addRow("一级分类:", categoryCombo);
    form->addRow("二级分类:", subCategoryCombo);
    form->addRow("价格:", priceEdit);
    form->addRow("库存:", stockEdit);
    form->addRow("描述:", descEdit);
    form->addRow("封面图片:", imageLayout);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addRow(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject bookData;
        bookData["title"] = titleEdit->text();
        bookData["author"] = authorEdit->text();
        bookData["category1"] = categoryCombo->currentText();
        bookData["category2"] = subCategoryCombo->currentText();
        bookData["price"] = priceEdit->value();
        bookData["stock"] = stockEdit->value();
        bookData["description"] = descEdit->toPlainText();  // 书籍描述
        // 如果选择了新图片，则更新；否则不更新封面图片
        if (!coverImageBase64.isEmpty()) {
            bookData["coverImage"] = coverImageBase64;
        }
        
        QJsonObject response = apiService->updateBook(currentSellerId, bookId, bookData);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "图书更新成功！");
            loadBooks();
        } else {
            QMessageBox::warning(this, "错误", "更新图书失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onDeleteBookClicked()
{
    if (selectedBookRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的图书！");
        return;
    }
    
    QString bookId = booksTable->item(selectedBookRow, 0)->text();
    QString bookName = booksTable->item(selectedBookRow, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", 
        "确定要删除图书 \"" + bookName + "\" 吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->deleteBook(currentSellerId, bookId);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "图书删除成功！");
            loadBooks();
            selectedBookRow = -1;
        } else {
            QMessageBox::warning(this, "错误", "删除图书失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onBookTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedBookRow = row;
}

// ===== 订单管理 =====
void BookMerchant::onRefreshOrdersClicked()
{
    loadOrders();
}

void BookMerchant::loadOrders(bool showEmptyMessage, bool updateDashboard)
{
    if (currentSellerId.isEmpty()) {
        qWarning() << "loadOrders: 卖家ID为空，无法加载订单";
        QMessageBox::warning(this, "错误", "请先登录");
        return;
    }
    
    qDebug() << "loadOrders: 开始加载订单，卖家ID:" << currentSellerId;
    
    QJsonObject response = apiService->getSellerOrders(currentSellerId);
    
    // 调试：打印完整响应
    qDebug() << "loadOrders: 收到响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    if (response["success"].toBool()) {
        QJsonArray orders = response["orders"].toArray();
        
        // 更新统计信息
        double totalSales = response["totalSales"].toDouble(0);
        int totalOrders = response["total"].toInt(0);
        int paidOrders = response["paidOrders"].toInt(0);
        int shippedOrders = response["shippedOrders"].toInt(0);
        
        qDebug() << "loadOrders: 订单统计 - 总订单:" << totalOrders << "总销售额:" << totalSales << "已支付:" << paidOrders << "已发货:" << shippedOrders;
        qDebug() << "loadOrders: 订单数组大小:" << orders.size();
        
        // 更新主页统计标签
        if (totalOrdersLabel) {
            totalOrdersLabel->setText(QString::number(totalOrders));
        }
        if (totalSalesLabel) {
            totalSalesLabel->setText(QString("¥%1").arg(totalSales, 0, 'f', 2));
        }
        
        ordersTable->setRowCount(0);
        
        if (orders.isEmpty()) {
            qDebug() << "loadOrders: 订单列表为空";
            // 只在用户主动点击订单管理时提示一次
            if (showEmptyMessage) {
                QMessageBox::information(this, "提示", "您还没有任何订单");
            }
        } else {
            for (const QJsonValue &orderValue : orders) {
                QJsonObject order = orderValue.toObject();
                
                // 调试：打印每个订单的详细信息
                qDebug() << "loadOrders: 处理订单:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
                
                int row = ordersTable->rowCount();
                ordersTable->insertRow(row);
                
                QString orderId = order["orderId"].toString();
                if (orderId.isEmpty()) {
                    qWarning() << "loadOrders: 订单ID为空，跳过该订单";
                    continue;
                }
                
                ordersTable->setItem(row, 0, new QTableWidgetItem(orderId));
                
                // 客户信息
                QString customer = order["customer"].toString();
                if (customer.isEmpty()) {
                    customer = QString("用户%1").arg(order["userId"].toInt());
                }
                ordersTable->setItem(row, 1, new QTableWidgetItem(customer));
                
                // 总金额
                double totalAmount = order["totalAmount"].toDouble();
                if (totalAmount == 0.0) {
                    totalAmount = order["amount"].toDouble();
                }
                ordersTable->setItem(row, 2, new QTableWidgetItem(QString::number(totalAmount, 'f', 2)));
                
                // 订单状态
                QString status = order["status"].toString();
                ordersTable->setItem(row, 3, new QTableWidgetItem(status));
                
                // 下单时间
                QString orderDate = order["orderDate"].toString();
                if (orderDate.isEmpty()) {
                    orderDate = order["createTime"].toString();
                }
                ordersTable->setItem(row, 4, new QTableWidgetItem(orderDate));
                
                // 发货时间（如果已发货）
                QString shipTime = order["shipTime"].toString();
                QString displayShipTime = shipTime.isEmpty() ? "未发货" : shipTime;
                ordersTable->setItem(row, 5, new QTableWidgetItem(displayShipTime));
            }
            
            qDebug() << "loadOrders: 成功加载了" << ordersTable->rowCount() << "个订单到表格";
            
            // 强制刷新表格显示
            ordersTable->resizeColumnsToContents();
            ordersTable->update();
            ordersTable->repaint();
        }
    } else {
        QString errorMsg = response["message"].toString();
        qWarning() << "loadOrders: 加载订单失败:" << errorMsg;
        QMessageBox::warning(this, "错误", "加载订单失败：" + errorMsg);
    }
    
    // 更新订单状态统计
    updateOrderStatusStats();
    
    // 更新仪表板数据（订单数据可能已变化），但避免循环调用
    if (updateDashboard) {
        updateDashboardData();
    }
}

void BookMerchant::onUpdateOrderStatusClicked()
{
    if (selectedOrderRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要操作的订单！");
        return;
    }
    
    QString orderId = ordersTable->item(selectedOrderRow, 0)->text();
    QString currentStatus = ordersTable->item(selectedOrderRow, 3)->text();
    
    // 检查订单状态，只有"已支付"状态的订单才能发货
    if (currentStatus != "已支付") {
        QMessageBox::warning(this, "提示", 
            QString("只有【已支付】状态的订单才能发货！\n当前订单状态：%1").arg(currentStatus));
        return;
    }
    
    // 弹出发货确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "确认发货", 
        QString("确定要将订单 %1 标记为已发货吗？").arg(orderId),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 调用发货API
    QJsonObject request;
    request["action"] = "shipOrder";
    request["orderId"] = orderId;
    request["sellerId"] = currentSellerId;
    request["trackingNumber"] = QString("SF%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000);
    
    QJsonObject response = apiService->getTcpClient()->sendRequest(request, 10000);
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", 
            QString("发货成功！\n物流单号：%1").arg(response["trackingNumber"].toString()));
        loadOrders();  // 刷新订单列表
    } else {
        QMessageBox::warning(this, "错误", "发货失败：" + response["message"].toString());
    }
}

void BookMerchant::onDeleteOrderClicked()
{
    if (selectedOrderRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的订单！");
        return;
    }
    
    QString orderId = ordersTable->item(selectedOrderRow, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", 
        "确定要删除订单 \"" + orderId + "\" 吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->deleteOrder(currentSellerId, orderId);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "订单删除成功！");
            loadOrders();
            selectedOrderRow = -1;
        } else {
            QMessageBox::warning(this, "错误", "删除订单失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onOrderTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedOrderRow = row;
}

// ===== 会员管理 =====
void BookMerchant::loadMembers()
{
    QJsonObject response = apiService->getMembers(currentSellerId);
    
    if (response["success"].toBool()) {
        QJsonArray members = response["members"].toArray();
        
        membersTable->setRowCount(0);
        for (const QJsonValue &memberValue : members) {
            QJsonObject member = memberValue.toObject();
            int row = membersTable->rowCount();
            membersTable->insertRow(row);
            
            membersTable->setItem(row, 0, new QTableWidgetItem(QString::number(member["userId"].toInt())));
            membersTable->setItem(row, 1, new QTableWidgetItem(member["username"].toString()));
            membersTable->setItem(row, 2, new QTableWidgetItem(member["email"].toString()));
            
            // 显示会员等级（memberLevel字符串，如"普通会员"、"银卡会员"等）
            QString memberLevel = member["memberLevel"].toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
            membersTable->setItem(row, 3, new QTableWidgetItem(memberLevel));
            // 卖家不能看到会员余额，已移除余额列
            membersTable->setItem(row, 4, new QTableWidgetItem(member["registerDate"].toString()));
        }
    } else {
        QMessageBox::warning(this, "错误", "加载会员失败：" + response["message"].toString());
    }
}

void BookMerchant::onEditMemberClicked()
{
    if (selectedMemberRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的会员！");
        return;
    }
    
    QString memberId = membersTable->item(selectedMemberRow, 0)->text();
    QString currentMemberLevel = membersTable->item(selectedMemberRow, 3)->text();  // 会员等级在第3列（索引3）
    
    QDialog dialog(this);
    dialog.setWindowTitle("编辑会员");
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    QLineEdit *emailEdit = new QLineEdit(membersTable->item(selectedMemberRow, 2)->text());  // 邮箱在第2列
    QComboBox *levelCombo = new QComboBox();
    levelCombo->addItems({"普通会员", "银卡会员", "金卡会员", "白金会员", "钻石会员"});
    // 设置当前选中的会员等级
    int currentIndex = levelCombo->findText(currentMemberLevel);
    if (currentIndex >= 0) {
        levelCombo->setCurrentIndex(currentIndex);
    }
    
    form->addRow("邮箱:", emailEdit);
    form->addRow("会员等级:", levelCombo);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addRow(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject memberData;
        memberData["email"] = emailEdit->text();
        memberData["memberLevel"] = levelCombo->currentText();
        
        QJsonObject response = apiService->updateMember(currentSellerId, memberId, memberData);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "会员更新成功！");
            loadMembers();
        } else {
            QMessageBox::warning(this, "错误", "更新会员失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onDeleteMemberClicked()
{
    if (selectedMemberRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的会员！");
        return;
    }
    
    QString memberId = membersTable->item(selectedMemberRow, 0)->text();
    QString username = membersTable->item(selectedMemberRow, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", 
        "确定要删除会员 \"" + username + "\" 吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->deleteMember(currentSellerId, memberId);
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "会员删除成功！");
            loadMembers();
            selectedMemberRow = -1;
        } else {
            QMessageBox::warning(this, "错误", "删除会员失败：" + response["message"].toString());
        }
    }
}

void BookMerchant::onMemberTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedMemberRow = row;
}

// ===== 统计报表 =====
void BookMerchant::onRefreshStatsClicked()
{
    loadStats();
}

void BookMerchant::loadStats()
{
    QJsonObject response = apiService->getDashboardStats(currentSellerId);
    
    if (response["success"].toBool()) {
        QJsonObject stats = response["stats"].toObject();
        
        totalSalesLabel->setText("¥" + QString::number(stats["totalSales"].toDouble(), 'f', 2));
        totalOrdersLabel->setText(QString::number(stats["totalOrders"].toInt()));
        totalMembersLabel->setText(QString::number(stats["totalMembers"].toInt()));
        totalBooksLabel->setText(QString::number(stats["totalBooks"].toInt()));
        
        reportDisplay->setPlainText("统计数据已更新");
    } else {
        QMessageBox::warning(this, "错误", "加载统计数据失败：" + response["message"].toString());
    }
}

void BookMerchant::onGenerateSalesReportClicked()
{
    QString startDate = reportStartDate->date().toString("yyyy-MM-dd");
    QString endDate = reportEndDate->date().toString("yyyy-MM-dd");
    
    QJsonObject response = apiService->getSalesReport(currentSellerId, startDate, endDate);
    
    if (response["success"].toBool()) {
        QString report = "=== 销售报表 ===\n";
        report += "日期范围: " + startDate + " 至 " + endDate + "\n\n";
        
        QJsonArray data = response["data"].toArray();
        if (data.isEmpty()) {
            report += "该日期范围内无销售数据\n";
        } else {
            report += "日期\t\t订单数\t销售额\n";
            report += "----------------------------------------\n";
            double totalAmount = 0.0;
            int totalCount = 0;
            for (const QJsonValue &value : data) {
                QJsonObject item = value.toObject();
                QString date = item["date"].toString();
                int count = item["count"].toInt();
                double amount = item["amount"].toDouble();
                totalAmount += amount;
                totalCount += count;
                report += date + "\t" + QString::number(count) + "\t¥" + QString::number(amount, 'f', 2) + "\n";
            }
            report += "----------------------------------------\n";
            report += "总计\t\t" + QString::number(totalCount) + "\t¥" + QString::number(totalAmount, 'f', 2) + "\n";
        }
        
        reportDisplay->setPlainText(report);
    } else {
        QMessageBox::warning(this, "错误", "生成销售报表失败：" + response["message"].toString());
    }
}

void BookMerchant::onGenerateInventoryReportClicked()
{
    QString startDate = reportStartDate->date().toString("yyyy-MM-dd");
    QString endDate = reportEndDate->date().toString("yyyy-MM-dd");
    
    QJsonObject response = apiService->getInventoryReport(currentSellerId, startDate, endDate);
    
    if (response["success"].toBool()) {
        QString report = "=== 库存报表 ===\n";
        report += "日期范围: " + startDate + " 至 " + endDate + "\n\n";
        
        QJsonArray data = response["data"].toArray();
        if (data.isEmpty()) {
            report += "该日期范围内无库存数据\n";
        } else {
            report += "分类\t\t图书数\t库存量\t订单数\n";
            report += "----------------------------------------\n";
            for (const QJsonValue &value : data) {
                QJsonObject item = value.toObject();
                QString category = item["category"].toString();
                int count = item["count"].toInt();
                int stock = item["stock"].toInt();
                int orderCount = item["orderCount"].toInt();
                report += category + "\t" + QString::number(count) + "\t" + QString::number(stock) + "\t" + QString::number(orderCount) + "\n";
            }
        }
        
        reportDisplay->setPlainText(report);
    } else {
        QMessageBox::warning(this, "错误", "生成库存报表失败：" + response["message"].toString());
    }
}

void BookMerchant::onGenerateMemberReportClicked()
{
    QString startDate = reportStartDate->date().toString("yyyy-MM-dd");
    QString endDate = reportEndDate->date().toString("yyyy-MM-dd");
    
    QJsonObject response = apiService->getMemberReport(currentSellerId, startDate, endDate);
    
    if (response["success"].toBool()) {
        QString report = "=== 会员报表 ===\n";
        report += "日期范围: " + startDate + " 至 " + endDate + "\n\n";
        
        QJsonArray data = response["data"].toArray();
        if (data.isEmpty()) {
            report += "该日期范围内无会员数据\n";
        } else {
            report += "会员等级\t\t数量\n";
            report += "----------------------------------------\n";
            for (const QJsonValue &value : data) {
                QJsonObject item = value.toObject();
                QString level = item["level"].toString();
                int count = item["count"].toInt();
                report += level + "\t\t" + QString::number(count) + "\n";
            }
        }
        
        reportDisplay->setPlainText(report);
    } else {
        QMessageBox::warning(this, "错误", "生成会员报表失败：" + response["message"].toString());
    }
}

// ===== 页面切换 =====
void BookMerchant::showLoginPage()
{
    stackedWidget->setCurrentWidget(loginPage);
}

void BookMerchant::showMainPage()
{
    updateDashboardData();  // 立即更新一次仪表板数据
    
    // 启动仪表板自动刷新定时器
    if (dashboardRefreshTimer && !dashboardRefreshTimer->isActive()) {
        dashboardRefreshTimer->start();
    }
    
    stackedWidget->setCurrentWidget(mainPage);
}

void BookMerchant::showBooksPage()
{
    loadBooks();
    stackedWidget->setCurrentWidget(booksPage);
}

void BookMerchant::showOrdersPage()
{
    // 用户主动点击订单管理，显示空订单提示，并更新仪表板
    loadOrders(true, true);
    stackedWidget->setCurrentWidget(ordersPage);
}

void BookMerchant::showMembersPage()
{
    loadMembers();
    stackedWidget->setCurrentWidget(membersPage);
}

void BookMerchant::showStatsPage()
{
    loadStats();
    stackedWidget->setCurrentWidget(statsPage);
}

void BookMerchant::showReviewsPage()
{
    loadReviews();
    stackedWidget->setCurrentWidget(reviewsPage);
}

void BookMerchant::showProfilePage()
{
    // 停止聊天刷新定时器
    if (chatRefreshTimer && chatRefreshTimer->isActive()) {
        chatRefreshTimer->stop();
    }
    
    onRefreshProfileClicked();
    onRefreshAppealClicked();
    stackedWidget->setCurrentWidget(profilePage);
}

void BookMerchant::onProfileClicked()
{
    showProfilePage();
}

void BookMerchant::onRefreshProfileClicked()
{
    if (currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    
    QJsonObject response = apiService->getSellerProfile(currentSellerId);
    
    if (response["success"].toBool()) {
        // 更新欢迎语
        QString sellerName = response["sellerName"].toString();
        if (sellerName.isEmpty()) {
            sellerName = currentSellerName;
        }
        profileWelcomeLabel->setText(QString("你好，%1！").arg(sellerName));
        
        // 更新输入框
        profileSellerNameEdit->setText(sellerName);
        profileEmailEdit->setText(response["email"].toString());
        profilePhoneEdit->setText(response["phoneNumber"].toString().isEmpty() ? "" : response["phoneNumber"].toString());
        profileAddressEdit->setText(response["address"].toString().isEmpty() ? "" : response["address"].toString());
        
        // 更新账户余额
        double balance = response["balance"].toDouble(0.0);
        profileBalanceLabel->setText(QString("¥%1").arg(balance, 0, 'f', 2));
        
        // 更新账户状态
        QString status = response["status"].toString();
        if (status.isEmpty()) {
            status = "正常";
        }
        profileStatusLabel->setText(status);
        if (status == "封禁") {
            profileStatusLabel->setStyleSheet(
                "QLabel {"
                "    color: #e74c3c;"
                "    font-size: 18px;"
                "    font-weight: bold;"
                "    background: transparent;"
                "    border: none;"
                "    margin: 0px;"
                "    padding: 0px;"
                "}"
            );
        } else {
            profileStatusLabel->setStyleSheet(
                "QLabel {"
                "    color: #27ae60;"
                "    font-size: 18px;"
                "    font-weight: bold;"
                "    background: transparent;"
                "    border: none;"
                "    margin: 0px;"
                "    padding: 0px;"
                "}"
            );
        }
        
        // 更新会员卡片
        QString memberLevel = response.value("memberLevel").toString();
        if (memberLevel.isEmpty()) {
            memberLevel = "普通会员";
        }
        double memberDiscount = response.value("memberDiscount").toDouble(1.0);
        double totalRecharge = response.value("totalRecharge").toDouble(0.0);
        
        QString levelText = QString("%1 %2折").arg(memberLevel).arg(memberDiscount * 10, 0, 'f', 1);
        memberCardLabel->setText(levelText);
        memberCardRechargeLabel->setText(QString("累计充值: ¥%1").arg(totalRecharge, 0, 'f', 2));
        
        // 更新积分
        int points = response.value("points").toInt(0);
        profilePointsLabel->setText(QString::number(points));
    } else {
        QMessageBox::warning(this, "错误", "获取个人信息失败：" + response["message"].toString());
    }
}

void BookMerchant::onSubmitAppealClicked()
{
    if (currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    
    QString appealReason = appealReasonEdit->toPlainText().trimmed();
    if (appealReason.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入申诉理由");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认提交", 
        "确定要提交申诉吗？", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    QJsonObject response = apiService->submitAppeal(currentSellerId, appealReason);
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", response["message"].toString());
        appealReasonEdit->clear();
        onRefreshAppealClicked();
    } else {
        QMessageBox::warning(this, "错误", "提交申诉失败：" + response["message"].toString());
    }
}

void BookMerchant::onUpdateProfileClicked()
{
    if (currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    
    QString email = profileEmailEdit->text().trimmed();
    QString phone = profilePhoneEdit->text().trimmed();
    QString address = profileAddressEdit->text().trimmed();
    
    // 验证邮箱格式
    if (!email.isEmpty() && !email.contains("@")) {
        QMessageBox::warning(this, "提示", "请输入有效的邮箱地址");
        return;
    }
    
    QJsonObject sellerData;
    sellerData["sellerId"] = currentSellerId;
    if (!email.isEmpty()) {
        sellerData["email"] = email;
    }
    if (!phone.isEmpty()) {
        sellerData["phoneNumber"] = phone;
    }
    if (!address.isEmpty()) {
        sellerData["address"] = address;
    }
    
    QJsonObject response = apiService->updateSellerProfile(sellerData);
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", "个人信息更新成功");
        onRefreshProfileClicked();  // 刷新显示
    } else {
        QMessageBox::warning(this, "错误", "更新失败：" + response["message"].toString());
    }
}

void BookMerchant::onLevelInfoClicked()
{
    QString info = QString(
        "会员等级说明：\n\n"
        "• 普通会员：累计充值 0 元\n"
        "  享受折扣：无折扣\n\n"
        "• 银卡会员：累计充值 200 元\n"
        "  享受折扣：9.5折\n\n"
        "• 金卡会员：累计充值 1,000 元\n"
        "  享受折扣：9折\n\n"
        "• 铂金会员：累计充值 3,000 元\n"
        "  享受折扣：8.5折\n\n"
        "• 钻石会员：累计充值 8,000 元\n"
        "  享受折扣：8折\n\n"
        "• 黑钻会员：累计充值 20,000 元\n"
        "  享受折扣：7.5折\n\n"
        "注：会员等级根据累计充值总额自动升级"
    );
    
    QMessageBox::information(this, "会员等级说明", info);
}

void BookMerchant::onRefreshAppealClicked()
{
    if (currentSellerId.isEmpty()) {
        return;
    }
    
    QJsonObject response = apiService->getAppeal(currentSellerId);
    
    if (response["success"].toBool()) {
        QJsonObject appeal = response["appeal"].toObject();
        if (!appeal.isEmpty()) {
            QString status = appeal["status"].toString();
            appealStatusLabel->setText(status);
            
            if (status == "待审核") {
                appealStatusLabel->setStyleSheet("padding: 5px; background-color: #f39c12; color: white; border-radius: 3px; font-weight: bold;");
            } else if (status == "已通过") {
                appealStatusLabel->setStyleSheet("padding: 5px; background-color: #27ae60; color: white; border-radius: 3px; font-weight: bold;");
            } else if (status == "未通过") {
                appealStatusLabel->setStyleSheet("padding: 5px; background-color: #e74c3c; color: white; border-radius: 3px; font-weight: bold;");
            } else {
                appealStatusLabel->setStyleSheet("padding: 5px; background-color: #ecf0f1; border-radius: 3px;");
            }
            
            QString reviewComment = appeal["reviewComment"].toString();
            if (!reviewComment.isEmpty()) {
                appealReviewCommentLabel->setText(reviewComment);
            } else {
                appealReviewCommentLabel->setText("-");
            }
        } else {
            appealStatusLabel->setText("暂无申诉记录");
            appealStatusLabel->setStyleSheet("padding: 5px; background-color: #ecf0f1; border-radius: 3px;");
            appealReviewCommentLabel->setText("-");
        }
    } else {
        appealStatusLabel->setText("获取申诉状态失败");
        appealStatusLabel->setStyleSheet("padding: 5px; background-color: #ecf0f1; border-radius: 3px;");
    }
}

void BookMerchant::showChatPage()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    // 重置最后消息时间
    lastChatMessageTime = QDateTime();
    
    // 显示客服聊天页面并加载历史消息
    loadChatHistory();
    stackedWidget->setCurrentWidget(chatPage);
    
    // 启动客服聊天刷新定时器
    if (!chatRefreshTimer->isActive()) {
        chatRefreshTimer->start();
    }
    
    // 停止买家聊天刷新定时器（如果正在运行）
    if (buyerChatRefreshTimer && buyerChatRefreshTimer->isActive()) {
        buyerChatRefreshTimer->stop();
    }
}

void BookMerchant::loadChatHistory()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    // 获取与客服的聊天历史（receiverId为空表示发送给管理员/客服）
    QJsonObject response = apiService->getChatHistory(
        currentSellerId,
        "seller"
    );
    
    if (response["success"].toBool()) {
        QJsonArray messages = response["messages"].toArray();
        
        // 获取当前显示的最后一条消息时间（用于增量更新）
        QDateTime currentLastTime = lastChatMessageTime;
        bool hasNewMessages = false;
        
        // 如果lastChatMessageTime无效，说明是首次加载，清空显示
        if (!lastChatMessageTime.isValid()) {
            chatDisplay->clear();
        }
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            QString content = msg["content"].toString();
            QString sendTime = msg["sendTime"].toString();
            int senderId = msg["senderId"].toInt();
            
            // 格式化时间
            QDateTime dateTime = QDateTime::fromString(sendTime, "yyyy-MM-dd hh:mm:ss");
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(sendTime, Qt::ISODate);
            }
            
            // 如果是增量更新，只显示新消息
            if (lastChatMessageTime.isValid() && dateTime <= lastChatMessageTime) {
                continue;
            }
            
            // 客服聊天页面：只显示与客服/管理员的聊天消息
            // 过滤掉买家发来的消息（这些应该在客户消息页面显示）
            int receiverId = msg["receiverId"].toInt();
            QString receiverType = msg["receiverType"].toString();
            
            // 只显示：
            // 1. 卖家发送给客服的消息（receiverId为空或receiverType为admin）
            // 2. 客服/管理员发送给卖家的消息（senderType为admin）
            if (senderType == "buyer") {
                // 跳过买家发来的消息
                continue;
            }
            
            QString timeStr = dateTime.isValid() ? dateTime.toString("yyyy-MM-dd hh:mm") : sendTime;
            
            // 显示消息
            QString senderName;
            if (senderType == "seller") {
                senderName = "我";
            } else if (senderType == "admin") {
                senderName = "客服";
            } else {
                senderName = "未知";
            }
            
            chatDisplay->append(QString("[%1] %2: %3").arg(timeStr).arg(senderName).arg(content));
            
            // 更新最后一条消息时间
            if (dateTime.isValid() && (!lastChatMessageTime.isValid() || dateTime > lastChatMessageTime)) {
                lastChatMessageTime = dateTime;
                hasNewMessages = true;
            }
        }
        
        // 如果有新消息，滚动到底部
        if (hasNewMessages || !currentLastTime.isValid()) {
            QTextCursor cursor = chatDisplay->textCursor();
            cursor.movePosition(QTextCursor::End);
            chatDisplay->setTextCursor(cursor);
        }
    }
}

void BookMerchant::onSendChatClicked()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    QString message = chatInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请输入消息内容");
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 发送消息给客服（receiverId为空表示发送给管理员/客服）
    QJsonObject response = apiService->sendChatMessage(
        currentSellerId,
        "seller",
        "",  // 发送给客服，receiverId为空
        "",  // 接收者是管理员，但receiverId为空时表示发送给所有管理员
        message  // 消息内容
    );
    
    if (response["success"].toBool()) {
        chatInput->clear();
        
        // 不在这里直接显示消息，避免与定时器刷新时重复显示
        // 立即触发一次聊天历史加载，让服务器返回的消息被正确显示
        // loadChatHistory()会正确更新lastChatMessageTime
        loadChatHistory();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}

void BookMerchant::loadBuyerList()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    // 获取所有聊天历史，提取买家ID
    QJsonObject response = apiService->getChatHistory(
        currentSellerId,
        "seller"
    );
    
    if (response["success"].toBool()) {
        buyerListWidget->clear();
        
        // 提取所有买家ID
        QSet<int> buyerIds;
        QJsonArray messages = response["messages"].toArray();
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            int senderId = msg["senderId"].toInt();
            int receiverId = msg["receiverId"].toInt();
            
            // 如果消息是买家发送给卖家的，记录买家ID
            if (senderType == "buyer" && receiverId == currentSellerId.toInt()) {
                buyerIds.insert(senderId);
            }
            // 如果消息是卖家发送给买家的，也记录买家ID
            else if (senderType == "seller" && receiverId > 0) {
                buyerIds.insert(receiverId);
            }
        }
        
        // 添加买家到列表
        QList<int> sortedBuyerIds = buyerIds.values();
        std::sort(sortedBuyerIds.begin(), sortedBuyerIds.end());
        
        for (int buyerId : sortedBuyerIds) {
            QListWidgetItem *item = new QListWidgetItem(QString("买家 ID:%1").arg(buyerId));
            item->setData(Qt::UserRole, buyerId);
            buyerListWidget->addItem(item);
        }
        
        if (buyerIds.isEmpty()) {
            QListWidgetItem *emptyItem = new QListWidgetItem("暂无买家消息");
            emptyItem->setFlags(Qt::NoItemFlags);  // 禁用点击
            buyerListWidget->addItem(emptyItem);
        }
    }
}

void BookMerchant::onBuyerListItemClicked(QListWidgetItem *item)
{
    if (!item || !item->flags().testFlag(Qt::ItemIsEnabled)) {
        return;  // 如果是禁用项（如"暂无买家消息"），不处理
    }
    
    int buyerId = item->data(Qt::UserRole).toInt();
    if (buyerId <= 0) {
        return;
    }
    
    currentChatBuyerId = buyerId;
    
    // 更新当前买家标签
    currentBuyerLabel->setText(QString("与买家(ID:%1)聊天").arg(buyerId));
    
    // 重置最后消息时间，重新加载聊天记录
    lastBuyerChatMessageTime = QDateTime();
    loadBuyerChatHistory();
}

void BookMerchant::showBuyerChatPage()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    // 加载买家列表
    loadBuyerList();
    
    // 重置当前聊天买家ID
    currentChatBuyerId = -1;
    currentBuyerLabel->setText("请选择买家");
    lastBuyerChatMessageTime = QDateTime();  // 重置最后消息时间
    
    // 清空聊天显示
    buyerChatDisplay->clear();
    
    // 显示买家聊天页面
    stackedWidget->setCurrentWidget(buyerChatPage);
    
    // 启动买家聊天刷新定时器
    if (!buyerChatRefreshTimer->isActive()) {
        buyerChatRefreshTimer->start();
    }
    
    // 停止客服聊天刷新定时器（如果正在运行）
    if (chatRefreshTimer && chatRefreshTimer->isActive()) {
        chatRefreshTimer->stop();
    }
}

void BookMerchant::loadBuyerChatHistory()
{
    if (!isLoggedIn || currentSellerId.isEmpty() || currentChatBuyerId <= 0) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    // 获取与特定买家的聊天历史
    QJsonObject response = apiService->getChatHistory(
        currentSellerId,
        "seller",
        QString::number(currentChatBuyerId),
        "buyer"
    );
    
    if (response["success"].toBool()) {
        QJsonArray messages = response["messages"].toArray();
        
        // 获取当前显示的最后一条消息时间（用于增量更新）
        QDateTime currentLastTime = lastBuyerChatMessageTime;
        bool hasNewMessages = false;
        
        // 如果lastBuyerChatMessageTime无效，说明是首次加载，清空显示
        if (!lastBuyerChatMessageTime.isValid()) {
            buyerChatDisplay->clear();
        }
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            QString content = msg["content"].toString();
            QString sendTime = msg["sendTime"].toString();
            int senderId = msg["senderId"].toInt();
            int msgReceiverId = msg["receiverId"].toInt();
            
            // 格式化时间
            QDateTime dateTime = QDateTime::fromString(sendTime, "yyyy-MM-dd hh:mm:ss");
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(sendTime, Qt::ISODate);
            }
            
            // 如果是增量更新，只显示新消息
            if (lastBuyerChatMessageTime.isValid() && dateTime <= lastBuyerChatMessageTime) {
                continue;
            }
            
            // 只显示与当前买家的消息
            if (senderType == "buyer" && senderId != currentChatBuyerId) {
                continue;
            }
            if (senderType == "seller" && msgReceiverId != currentChatBuyerId) {
                continue;
            }
            
            QString timeStr = dateTime.isValid() ? dateTime.toString("yyyy-MM-dd hh:mm") : sendTime;
            
            // 显示消息
            QString senderName;
            if (senderType == "seller") {
                senderName = "我";
            } else if (senderType == "buyer") {
                senderName = QString("买家(ID:%1)").arg(senderId);
            } else {
                senderName = "未知";
            }
            
            buyerChatDisplay->append(QString("[%1] %2: %3").arg(timeStr).arg(senderName).arg(content));
            
            // 更新最后一条消息时间
            if (dateTime.isValid() && (!lastBuyerChatMessageTime.isValid() || dateTime > lastBuyerChatMessageTime)) {
                lastBuyerChatMessageTime = dateTime;
                hasNewMessages = true;
            }
        }
        
        // 如果有新消息，滚动到底部
        if (hasNewMessages || !currentLastTime.isValid()) {
            QTextCursor cursor = buyerChatDisplay->textCursor();
            cursor.movePosition(QTextCursor::End);
            buyerChatDisplay->setTextCursor(cursor);
        }
    }
}

void BookMerchant::onSendBuyerChatClicked()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    if (currentChatBuyerId <= 0) {
        QMessageBox::warning(this, "发送失败", "请先选择买家");
        return;
    }
    
    QString message = buyerChatInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请输入消息内容");
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 发送消息给特定买家
    QJsonObject response = apiService->sendChatMessage(
        currentSellerId,
        "seller",
        QString::number(currentChatBuyerId),
        "buyer",
        message
    );
    
    if (response["success"].toBool()) {
        buyerChatInput->clear();
        
        // 不在这里直接显示消息，避免与定时器刷新时重复显示
        // 立即触发一次聊天历史加载，让服务器返回的消息被正确显示
        // loadBuyerChatHistory()会正确更新lastBuyerChatMessageTime
        loadBuyerChatHistory();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}

// ===== 评论管理 =====
void BookMerchant::loadReviews()
{
    if (!isLoggedIn || currentSellerId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 先获取该卖家的所有商品
    QJsonObject booksResponse = apiService->getSellerBooks(currentSellerId);
    if (!booksResponse["success"].toBool()) {
        QMessageBox::warning(this, "错误", "获取商品列表失败：" + booksResponse["message"].toString());
        return;
    }
    
    QJsonArray books = booksResponse["books"].toArray();
    
    // 清空表格
    reviewsTable->setRowCount(0);
    
    // 直接获取该卖家的所有商品评论（通过服务器端API）
    QJsonObject sellerReviewsResponse = apiService->getSellerReviews(currentSellerId);
    QJsonArray allReviews;
    
    if (sellerReviewsResponse["success"].toBool()) {
        allReviews = sellerReviewsResponse["reviews"].toArray();
    } else {
        qWarning() << "获取卖家评论失败:" << sellerReviewsResponse["message"].toString();
    }
    
    // 创建商品ID到商品信息的映射（用于显示商品名称，如果商品列表中有的话）
    QMap<QString, QString> bookIdToTitleMap;
    for (const QJsonValue &bookValue : books) {
        QJsonObject book = bookValue.toObject();
        QString bookId = book["isbn"].toString();
        QString bookTitle = book["title"].toString();
        bookIdToTitleMap[bookId] = bookTitle;
    }
    
    // 显示所有评论
    int totalReviews = 0;
    for (const QJsonValue &reviewValue : allReviews) {
        QJsonObject review = reviewValue.toObject();
        int row = reviewsTable->rowCount();
        reviewsTable->insertRow(row);
        
        QString bookId = review["bookId"].toString();
        QString bookTitle = review["bookTitle"].toString();
        
        // 如果服务器返回的商品名称为空，尝试从商品列表中获取
        if (bookTitle.isEmpty() && bookIdToTitleMap.contains(bookId)) {
            bookTitle = bookIdToTitleMap[bookId];
        }
        // 如果还是为空，使用商品ID
        if (bookTitle.isEmpty()) {
            bookTitle = bookId;
        }
        
        reviewsTable->setItem(row, 0, new QTableWidgetItem(bookId));
        reviewsTable->setItem(row, 1, new QTableWidgetItem(bookTitle));
        reviewsTable->setItem(row, 2, new QTableWidgetItem(review["username"].toString()));
        reviewsTable->setItem(row, 3, new QTableWidgetItem(QString::number(review["rating"].toInt()) + "分"));
        reviewsTable->setItem(row, 4, new QTableWidgetItem(review["comment"].toString()));
        reviewsTable->setItem(row, 5, new QTableWidgetItem(review["reviewTime"].toString()));
        
        totalReviews++;
    }
    
    // 如果没有评论，显示提示
    if (totalReviews == 0) {
        int row = reviewsTable->rowCount();
        reviewsTable->insertRow(row);
        QTableWidgetItem *noReviewItem = new QTableWidgetItem("暂无评论");
        noReviewItem->setTextAlignment(Qt::AlignCenter);
        reviewsTable->setItem(row, 0, noReviewItem);
        reviewsTable->setSpan(row, 0, 1, 6);  // 合并6列
    }
    
    qDebug() << "评论加载完成，共" << totalReviews << "条评论";
}
