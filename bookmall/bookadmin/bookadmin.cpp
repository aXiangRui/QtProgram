#include "bookadmin.h"
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>
#include <QByteArray>
#include <QBuffer>
#include <QPixmap>
#include <QImage>
#include "apiservice.h"
#include <QDebug>

BookAdmin::BookAdmin(QWidget *parent)
    : QMainWindow(parent)
    , selectedUserRow(-1)
    , selectedSellerRow(-1)
    , selectedBookRow(-1)
    , selectedOrderRow(-1)
    , currentReviewUserId(-1)
    , selectedAppealRow(-1)
    , selectedPendingSellerRow(-1)
    , currentChatUserId(-1)
    , currentChatUserType("")
    , isLoggedIn(false)
    , serverIp("127.0.0.1")
    , serverPort(8888)
{
    apiService = new ApiService(this);
    
    // 初始化聊天刷新定时器
    chatRefreshTimer = new QTimer(this);
    chatRefreshTimer->setInterval(2000);  // 每2秒刷新一次
    connect(chatRefreshTimer, &QTimer::timeout, this, &BookAdmin::loadChatHistory);
    
    // 初始化仪表盘自动刷新定时器
    dashboardRefreshTimer = new QTimer(this);
    dashboardRefreshTimer->setInterval(30000);  // 每30秒自动刷新一次
    connect(dashboardRefreshTimer, &QTimer::timeout, this, &BookAdmin::loadStats);
    
    // 连接信号
    connect(apiService, &ApiService::connected, this, [this]() {
        qDebug() << "已连接到服务器";
        if (loginStatusLabel) {
            loginStatusLabel->setText("✓ 已连接服务器");
            loginStatusLabel->setStyleSheet("color: green;");
        }
    });
    
    qDebug() << "服务器配置 - IP:" << serverIp << "端口:" << serverPort;
    
    initUI();
    initConnections();
    applyStyle();
    
    setWindowTitle("图书管理系统 - 超级管理员");
    resize(1400, 900);
    
    showLoginPage();
    
    // 自动连接服务器
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "自动连接服务器...";
        if (!apiService->isConnected()) {
            if (apiService->connectToServer(serverIp, serverPort)) {
                qDebug() << "自动连接服务器成功";
            } else {
                qDebug() << "自动连接服务器失败";
            }
        }
    });
}

BookAdmin::~BookAdmin()
{
}

void BookAdmin::initUI()
{
    stackedWidget = new QStackedWidget(this);
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
    QLabel *loginTitle = new QLabel("🔐 管理员登录");
    loginTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setFamily("Microsoft YaHei");
    titleFont.setBold(true);
    titleFont.setPointSize(36);
    loginTitle->setFont(titleFont);
    loginTitle->setStyleSheet("color: #2c3e50; font-size: 36px; font-weight: bold; margin-bottom: 10px;");
    cardLayout->addWidget(loginTitle);

    // 副标题
    QLabel *subTitle = new QLabel("欢迎回来，请登录管理员账户");
    subTitle->setAlignment(Qt::AlignCenter);
    QFont subFont;
    subFont.setFamily("Microsoft YaHei");
    subFont.setPointSize(12);
    subTitle->setFont(subFont);
    subTitle->setStyleSheet("color: #7f8c8d; margin-bottom: 30px;");
    cardLayout->addWidget(subTitle);

    // 用户名输入框
    QLabel *usernameLabel = new QLabel("账号");
    usernameLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-bottom: 5px;");
    cardLayout->addWidget(usernameLabel);
    
    loginUsername = new QLineEdit();
    loginUsername->setPlaceholderText("请输入管理员账号");
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
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setPlaceholderText("请输入密码");
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
    loginStatusLabel->setObjectName("loginStatusLabel");
    cardLayout->addWidget(loginStatusLabel);

    // 登录按钮
    loginButton = new QPushButton("登录系统");
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

    // ===== 仪表盘页面 =====
    dashboardPage = new QWidget();
    QVBoxLayout *dashLayout = new QVBoxLayout(dashboardPage);
    dashLayout->setContentsMargins(20, 20, 20, 20);
    dashLayout->setSpacing(15);

    // 顶部标题栏：左侧刷新按钮，中间标题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    dashboardRefreshButton = new QPushButton("🔄 刷新");
    dashboardRefreshButton->setMinimumHeight(40);
    dashboardRefreshButton->setMaximumWidth(120);
    titleLayout->addWidget(dashboardRefreshButton);
    titleLayout->addStretch();
    
    welcomeLabel = new QLabel("管理员控制中心");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 28px; font-weight: 600; padding: 20px;");
    titleLayout->addWidget(welcomeLabel);
    titleLayout->addStretch();
    // 添加一个相同宽度的占位控件，保持标题居中
    QWidget *spacer = new QWidget();
    spacer->setFixedWidth(dashboardRefreshButton->maximumWidth());
    titleLayout->addWidget(spacer);
    
    dashLayout->addLayout(titleLayout);

    // 顶部工具栏（只保留系统统计和退出登录）
    QHBoxLayout *navLayout = new QHBoxLayout();
    statsButton = new QPushButton("📊 系统统计");
    chatButton = new QPushButton("💬 聊天管理");
    logoutButton = new QPushButton("🚪 退出登录");
    
    statsButton->setMinimumHeight(50);
    chatButton->setMinimumHeight(50);
    logoutButton->setMinimumHeight(50);

    navLayout->addStretch();
    navLayout->addWidget(statsButton);
    navLayout->addWidget(chatButton);
    navLayout->addWidget(logoutButton);

    dashLayout->addLayout(navLayout);

    // 统计卡片 - 第一行：总用户数、总买家数、总商家数、待审核商家数
    QHBoxLayout *statsCardsRow1Layout = new QHBoxLayout();
    
    // 总用户数卡片（带用户管理按钮）
    QGroupBox *usersCard = new QGroupBox("总用户数");
    QVBoxLayout *usersCardLayout = new QVBoxLayout(usersCard);
    totalUsersLabel = new QLabel("0");
    totalUsersLabel->setAlignment(Qt::AlignCenter);
    totalUsersLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #3498db;");
    usersCardLayout->addWidget(totalUsersLabel);
    usersButton = new QPushButton("👥 用户管理");
    usersButton->setMinimumHeight(40);
    usersCardLayout->addWidget(usersButton);
    
    // 总买家数卡片（带买家管理按钮）
    QGroupBox *buyersCard = new QGroupBox("总买家数");
    QVBoxLayout *buyersCardLayout = new QVBoxLayout(buyersCard);
    totalBuyersLabel = new QLabel("0");
    totalBuyersLabel->setAlignment(Qt::AlignCenter);
    totalBuyersLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #9b59b6;");
    buyersCardLayout->addWidget(totalBuyersLabel);
    buyersButton = new QPushButton("🛒 买家管理");
    buyersButton->setMinimumHeight(40);
    buyersCardLayout->addWidget(buyersButton);
    
    // 总商家数卡片（带商家管理按钮）
    QGroupBox *sellersCard = new QGroupBox("总商家数");
    QVBoxLayout *sellersCardLayout = new QVBoxLayout(sellersCard);
    totalSellersLabel = new QLabel("0");
    totalSellersLabel->setAlignment(Qt::AlignCenter);
    totalSellersLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #e74c3c;");
    sellersCardLayout->addWidget(totalSellersLabel);
    sellersButton = new QPushButton("🏪 商家管理");
    sellersButton->setMinimumHeight(40);
    sellersCardLayout->addWidget(sellersButton);
    
    // 待审核商家数卡片（带审核管理按钮）
    QGroupBox *pendingSellersCard = new QGroupBox("待审核商家数");
    QVBoxLayout *pendingSellersCardLayout = new QVBoxLayout(pendingSellersCard);
    pendingSellersLabel = new QLabel("0");
    pendingSellersLabel->setAlignment(Qt::AlignCenter);
    pendingSellersLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #f39c12;");
    pendingSellersCardLayout->addWidget(pendingSellersLabel);
    reviewPendingSellersButton = new QPushButton("✓ 审核管理");
    reviewPendingSellersButton->setMinimumHeight(40);
    pendingSellersCardLayout->addWidget(reviewPendingSellersButton);
    
    statsCardsRow1Layout->addWidget(usersCard);
    statsCardsRow1Layout->addWidget(buyersCard);
    statsCardsRow1Layout->addWidget(sellersCard);
    statsCardsRow1Layout->addWidget(pendingSellersCard);
    
    dashLayout->addLayout(statsCardsRow1Layout);
    
    // 统计卡片 - 第二行：总图书数、总订单数（各占一半）
    QHBoxLayout *statsCardsRow2Layout = new QHBoxLayout();
    
    // 总图书数卡片（带图书管理按钮，占50%）
    QGroupBox *booksCard = new QGroupBox("总图书数");
    booksCard->setMinimumHeight(150);
    QVBoxLayout *booksCardLayout = new QVBoxLayout(booksCard);
    totalBooksLabel = new QLabel("0");
    totalBooksLabel->setAlignment(Qt::AlignCenter);
    totalBooksLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #2ecc71;");
    booksCardLayout->addWidget(totalBooksLabel);
    booksButton = new QPushButton("📚 图书管理");
    booksButton->setMinimumHeight(40);
    booksCardLayout->addWidget(booksButton);
    
    // 总订单数卡片（带订单管理按钮，占50%）
    QGroupBox *ordersCard = new QGroupBox("总订单数");
    ordersCard->setMinimumHeight(150);
    QVBoxLayout *ordersCardLayout = new QVBoxLayout(ordersCard);
    totalOrdersLabel = new QLabel("0");
    totalOrdersLabel->setAlignment(Qt::AlignCenter);
    totalOrdersLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #f39c12;");
    ordersCardLayout->addWidget(totalOrdersLabel);
    ordersButton = new QPushButton("📦 订单管理");
    ordersButton->setMinimumHeight(40);
    ordersCardLayout->addWidget(ordersButton);
    
    // 申诉管理卡片
    QGroupBox *appealsCard = new QGroupBox("申诉管理");
    appealsCard->setMinimumHeight(150);
    QVBoxLayout *appealsCardLayout = new QVBoxLayout(appealsCard);
    totalAppealsLabel = new QLabel("0");
    totalAppealsLabel->setAlignment(Qt::AlignCenter);
    totalAppealsLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #9b59b6;");
    appealsCardLayout->addWidget(totalAppealsLabel);
    appealsButton = new QPushButton("📝 申诉审核");
    appealsButton->setMinimumHeight(40);
    appealsCardLayout->addWidget(appealsButton);
    
    // 设置第二行三个卡片各占33%宽度
    statsCardsRow2Layout->addWidget(booksCard, 1);
    statsCardsRow2Layout->addWidget(ordersCard, 1);
    statsCardsRow2Layout->addWidget(appealsCard, 1);
    
    dashLayout->addLayout(statsCardsRow2Layout);
    dashLayout->addStretch();

    stackedWidget->addWidget(dashboardPage);

    // ===== 用户管理页面 =====
    usersPage = new QWidget();
    QVBoxLayout *usersLayout = new QVBoxLayout(usersPage);
    usersLayout->setContentsMargins(15, 15, 15, 15);
    usersLayout->setSpacing(10);

    usersTitleLabel = new QLabel("用户管理");
    usersTitleLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    usersLayout->addWidget(usersTitleLabel);

    QHBoxLayout *usersButtonLayout = new QHBoxLayout();
    refreshUsersBtn = new QPushButton("刷新");
    banUserBtn = new QPushButton("封禁用户");
    unbanUserBtn = new QPushButton("解封用户");
    backFromUsersBtn = new QPushButton("返回");

    usersButtonLayout->addWidget(refreshUsersBtn);
    usersButtonLayout->addWidget(banUserBtn);
    usersButtonLayout->addWidget(unbanUserBtn);
    usersButtonLayout->addStretch();
    usersButtonLayout->addWidget(backFromUsersBtn);

    usersLayout->addLayout(usersButtonLayout);

    usersTable = new QTableWidget();
    usersTable->setColumnCount(8);
    usersTable->setHorizontalHeaderLabels({"用户ID", "用户名", "邮箱", "注册日期", "状态", "余额", "会员等级", "身份"});
    usersTable->horizontalHeader()->setStretchLastSection(true);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    usersLayout->addWidget(usersTable);

    stackedWidget->addWidget(usersPage);

    // ===== 商家管理页面 =====
    sellersPage = new QWidget();
    QVBoxLayout *sellersLayout = new QVBoxLayout(sellersPage);
    sellersLayout->setContentsMargins(15, 15, 15, 15);
    sellersLayout->setSpacing(10);

    QLabel *sellersTitle = new QLabel("商家管理");
    sellersTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    sellersLayout->addWidget(sellersTitle);

    // 使用标签页区分所有商家和待审核商家认证申请
    sellersTabWidget = new QTabWidget();
    
    // 所有商家标签页
    QWidget *allSellersTab = new QWidget();
    QVBoxLayout *allSellersLayout = new QVBoxLayout(allSellersTab);

    QHBoxLayout *sellersButtonLayout = new QHBoxLayout();
    refreshSellersBtn = new QPushButton("刷新");
    banSellerBtn = new QPushButton("封禁商家");
    unbanSellerBtn = new QPushButton("解封商家");
    backFromSellersBtn = new QPushButton("返回");

    sellersButtonLayout->addWidget(refreshSellersBtn);
    sellersButtonLayout->addWidget(banSellerBtn);
    sellersButtonLayout->addWidget(unbanSellerBtn);
    sellersButtonLayout->addStretch();
    sellersButtonLayout->addWidget(backFromSellersBtn);

    allSellersLayout->addLayout(sellersButtonLayout);

    sellersTable = new QTableWidget();
    sellersTable->setColumnCount(5);
    sellersTable->setHorizontalHeaderLabels({"商家ID", "商家名", "邮箱", "注册日期", "状态"});
    sellersTable->horizontalHeader()->setStretchLastSection(true);
    sellersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sellersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    allSellersLayout->addWidget(sellersTable);
    
    sellersTabWidget->addTab(allSellersTab, "所有商家");
    
    // 待审核商家认证申请标签页
    QWidget *pendingSellersTab = new QWidget();
    QVBoxLayout *pendingSellersLayout = new QVBoxLayout(pendingSellersTab);
    
    QHBoxLayout *pendingSellersButtonLayout = new QHBoxLayout();
    refreshPendingSellersBtn = new QPushButton("刷新");
    reviewSellerBtn = new QPushButton("查看详情");
    
    pendingSellersButtonLayout->addWidget(refreshPendingSellersBtn);
    pendingSellersButtonLayout->addWidget(reviewSellerBtn);
    pendingSellersButtonLayout->addStretch();
    
    pendingSellersLayout->addLayout(pendingSellersButtonLayout);
    
    pendingSellersTable = new QTableWidget();
    pendingSellersTable->setColumnCount(5);
    pendingSellersTable->setHorizontalHeaderLabels({"用户ID", "用户名", "邮箱", "申请时间", "状态"});
    pendingSellersTable->horizontalHeader()->setStretchLastSection(true);
    pendingSellersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pendingSellersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pendingSellersLayout->addWidget(pendingSellersTable);
    
    sellersTabWidget->addTab(pendingSellersTab, "待审核商家认证");
    
    sellersLayout->addWidget(sellersTabWidget);

    stackedWidget->addWidget(sellersPage);

    // ===== 图书管理页面 =====
    booksPage = new QWidget();
    QVBoxLayout *booksLayout = new QVBoxLayout(booksPage);
    booksLayout->setContentsMargins(15, 15, 15, 15);
    booksLayout->setSpacing(10);

    QLabel *booksTitle = new QLabel("图书管理");
    booksTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    booksLayout->addWidget(booksTitle);

    // 使用标签页区分所有书籍和待审核书籍
    QTabWidget *booksTabWidget = new QTabWidget();
    
    // 所有书籍标签页
    QWidget *allBooksTab = new QWidget();
    QVBoxLayout *allBooksLayout = new QVBoxLayout(allBooksTab);
    
    QHBoxLayout *booksButtonLayout = new QHBoxLayout();
    refreshBooksBtn = new QPushButton("刷新");
    editBookBtn = new QPushButton("编辑图书");
    deleteBookBtn = new QPushButton("删除图书");
    backFromBooksBtn = new QPushButton("返回");

    booksButtonLayout->addWidget(refreshBooksBtn);
    booksButtonLayout->addWidget(editBookBtn);
    booksButtonLayout->addWidget(deleteBookBtn);
    booksButtonLayout->addStretch();
    booksButtonLayout->addWidget(backFromBooksBtn);

    allBooksLayout->addLayout(booksButtonLayout);
    
    booksTable = new QTableWidget();
    booksTable->setColumnCount(7);
    booksTable->setHorizontalHeaderLabels({"ISBN", "书名", "作者", "分类", "价格", "库存", "状态"});
    booksTable->horizontalHeader()->setStretchLastSection(true);
    booksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    booksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    allBooksLayout->addWidget(booksTable);
    
    booksTabWidget->addTab(allBooksTab, "所有书籍");
    
    // 待审核书籍标签页
    QWidget *pendingBooksTab = new QWidget();
    QVBoxLayout *pendingBooksLayout = new QVBoxLayout(pendingBooksTab);
    
    QHBoxLayout *pendingBooksButtonLayout = new QHBoxLayout();
    refreshPendingBooksBtn = new QPushButton("刷新");
    approveBookBtn = new QPushButton("审核通过");
    rejectBookBtn = new QPushButton("审核拒绝");
    
    pendingBooksButtonLayout->addWidget(refreshPendingBooksBtn);
    pendingBooksButtonLayout->addWidget(approveBookBtn);
    pendingBooksButtonLayout->addWidget(rejectBookBtn);
    pendingBooksButtonLayout->addStretch();
    
    pendingBooksLayout->addLayout(pendingBooksButtonLayout);
    
    pendingBooksTable = new QTableWidget();
    pendingBooksTable->setColumnCount(7);
    pendingBooksTable->setHorizontalHeaderLabels({"ISBN", "书名", "作者", "分类", "价格", "库存", "商家ID"});
    pendingBooksTable->horizontalHeader()->setStretchLastSection(true);
    pendingBooksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pendingBooksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pendingBooksLayout->addWidget(pendingBooksTable);
    
    booksTabWidget->addTab(pendingBooksTab, "待审核书籍");
    
    booksLayout->addWidget(booksTabWidget);

    stackedWidget->addWidget(booksPage);

    // ===== 订单管理页面 =====
    ordersPage = new QWidget();
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersPage);
    ordersLayout->setContentsMargins(15, 15, 15, 15);
    ordersLayout->setSpacing(10);

    QLabel *ordersTitle = new QLabel("订单管理");
    ordersTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    ordersLayout->addWidget(ordersTitle);

    QHBoxLayout *ordersButtonLayout = new QHBoxLayout();
    refreshOrdersBtn = new QPushButton("刷新");
    viewOrderDetailsBtn = new QPushButton("查看详情");
    deleteOrderBtn = new QPushButton("删除订单");
    backFromOrdersBtn = new QPushButton("返回");

    ordersButtonLayout->addWidget(refreshOrdersBtn);
    ordersButtonLayout->addWidget(viewOrderDetailsBtn);
    ordersButtonLayout->addWidget(deleteOrderBtn);
    ordersButtonLayout->addStretch();
    ordersButtonLayout->addWidget(backFromOrdersBtn);

    ordersLayout->addLayout(ordersButtonLayout);

    ordersTable = new QTableWidget();
    ordersTable->setColumnCount(6);
    ordersTable->setHorizontalHeaderLabels({"订单ID", "用户ID", "总金额", "状态", "日期", "商品详情"});
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersLayout->addWidget(ordersTable);

    stackedWidget->addWidget(ordersPage);

    // ===== 统计页面 =====
    statsPage = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsPage);
    statsLayout->setContentsMargins(15, 15, 15, 15);
    statsLayout->setSpacing(10);

    QLabel *statsTitle = new QLabel("系统统计");
    statsTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    statsLayout->addWidget(statsTitle);

    QHBoxLayout *statsButtonLayout = new QHBoxLayout();
    refreshStatsBtn = new QPushButton("刷新统计");
    backFromStatsBtn = new QPushButton("返回");

    statsButtonLayout->addWidget(refreshStatsBtn);
    statsButtonLayout->addStretch();
    statsButtonLayout->addWidget(backFromStatsBtn);

    statsLayout->addLayout(statsButtonLayout);

    statsDisplay = new QTextEdit();
    statsDisplay->setReadOnly(true);
    statsLayout->addWidget(statsDisplay);

    stackedWidget->addWidget(statsPage);
    
    // ===== 申诉管理页面 =====
    appealsPage = new QWidget();
    QVBoxLayout *appealsLayout = new QVBoxLayout(appealsPage);
    appealsLayout->setContentsMargins(15, 15, 15, 15);
    appealsLayout->setSpacing(10);
    
    QLabel *appealsTitle = new QLabel("申诉审核管理");
    appealsTitle->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    appealsLayout->addWidget(appealsTitle);
    
    QHBoxLayout *appealsButtonLayout = new QHBoxLayout();
    refreshAppealsBtn = new QPushButton("刷新");
    approveAppealBtn = new QPushButton("通过申诉");
    approveAppealBtn->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold;");
    rejectAppealBtn = new QPushButton("拒绝申诉");
    rejectAppealBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
    backFromAppealsBtn = new QPushButton("返回");
    
    appealsButtonLayout->addWidget(refreshAppealsBtn);
    appealsButtonLayout->addWidget(approveAppealBtn);
    appealsButtonLayout->addWidget(rejectAppealBtn);
    appealsButtonLayout->addStretch();
    appealsButtonLayout->addWidget(backFromAppealsBtn);
    
    appealsLayout->addLayout(appealsButtonLayout);
    
    // 审核意见输入
    QLabel *reviewCommentLabel = new QLabel("审核意见:");
    appealsLayout->addWidget(reviewCommentLabel);
    appealReviewCommentEdit = new QTextEdit();
    appealReviewCommentEdit->setPlaceholderText("请输入审核意见（可选）...");
    appealReviewCommentEdit->setMaximumHeight(80);
    appealsLayout->addWidget(appealReviewCommentEdit);
    
    appealsTable = new QTableWidget();
    appealsTable->setColumnCount(7);
    appealsTable->setHorizontalHeaderLabels({"申诉ID", "商家ID", "商家名", "申诉理由", "提交时间", "状态", "审核意见"});
    appealsTable->horizontalHeader()->setStretchLastSection(true);
    appealsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    appealsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    appealsLayout->addWidget(appealsTable);
    
    selectedAppealRow = -1;
    
    stackedWidget->addWidget(appealsPage);
    
    // ===== 聊天管理页面 =====
    chatPage = new QWidget();
    QHBoxLayout *chatMainLayout = new QHBoxLayout(chatPage);
    chatMainLayout->setContentsMargins(15, 15, 15, 15);
    chatMainLayout->setSpacing(10);
    
    // 左侧：用户列表
    QWidget *chatUserListWidget = new QWidget();
    chatUserListWidget->setMaximumWidth(300);
    QVBoxLayout *chatUserListLayout = new QVBoxLayout(chatUserListWidget);
    
    QLabel *userListTitle = new QLabel("用户列表");
    userListTitle->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    chatUserListLayout->addWidget(userListTitle);
    
    refreshChatBtn = new QPushButton("刷新列表");
    chatUserListLayout->addWidget(refreshChatBtn);
    
    chatUserList = new QListWidget();
    chatUserList->setStyleSheet("border: 1px solid #ddd; border-radius: 5px;");
    chatUserListLayout->addWidget(chatUserList, 1);
    
    backFromChatBtn = new QPushButton("返回");
    chatUserListLayout->addWidget(backFromChatBtn);
    
    chatMainLayout->addWidget(chatUserListWidget);
    
    // 右侧：聊天区域
    QWidget *chatAreaWidget = new QWidget();
    QVBoxLayout *chatAreaLayout = new QVBoxLayout(chatAreaWidget);
    
    QLabel *chatTitle = new QLabel("聊天记录");
    chatTitle->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");
    chatAreaLayout->addWidget(chatTitle);
    
    chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);
    chatDisplay->setMinimumHeight(400);
    chatDisplay->setStyleSheet("background-color: white; border: 1px solid #ddd; border-radius: 5px; padding: 10px;");
    chatAreaLayout->addWidget(chatDisplay, 1);
    
    QLabel *inputLabel = new QLabel("输入消息:");
    chatAreaLayout->addWidget(inputLabel);
    
    chatInput = new QTextEdit();
    chatInput->setPlaceholderText("请输入回复消息...");
    chatInput->setMaximumHeight(100);
    chatInput->setStyleSheet("border: 1px solid #ddd; border-radius: 5px; padding: 5px;");
    chatAreaLayout->addWidget(chatInput);
    
    sendChatBtn = new QPushButton("发送");
    sendChatBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold; padding: 8px 20px;");
    chatAreaLayout->addWidget(sendChatBtn, 0, Qt::AlignRight);
    
    chatMainLayout->addWidget(chatAreaWidget, 1);
    
    currentChatUserId = -1;
    currentChatUserType = "";
    
    stackedWidget->addWidget(chatPage);
    
    // ===== 审核页面 =====
    reviewPage = new QWidget();
    QVBoxLayout *reviewLayout = new QVBoxLayout(reviewPage);
    reviewLayout->setContentsMargins(20, 20, 20, 20);
    reviewLayout->setSpacing(15);
    
    QLabel *reviewTitle = new QLabel("卖家认证审核");
    reviewTitle->setStyleSheet("font-size: 24px; font-weight: bold; padding: 10px;");
    reviewLayout->addWidget(reviewTitle);
    
    // 用户信息区域
    QGroupBox *userInfoGroup = new QGroupBox("用户信息");
    QFormLayout *userInfoLayout = new QFormLayout(userInfoGroup);
    
    reviewUserIdLabel = new QLabel();
    reviewUsernameLabel = new QLabel();
    reviewEmailLabel = new QLabel();
    reviewApplyTimeLabel = new QLabel();
    
    userInfoLayout->addRow("用户ID:", reviewUserIdLabel);
    userInfoLayout->addRow("用户名:", reviewUsernameLabel);
    userInfoLayout->addRow("邮箱:", reviewEmailLabel);
    userInfoLayout->addRow("申请时间:", reviewApplyTimeLabel);
    
    reviewLayout->addWidget(userInfoGroup);
    
    // 营业执照图片区域
    QGroupBox *licenseGroup = new QGroupBox("营业执照");
    QVBoxLayout *licenseLayout = new QVBoxLayout(licenseGroup);
    
    reviewLicenseImageLabel = new QLabel();
    reviewLicenseImageLabel->setMinimumHeight(300);
    reviewLicenseImageLabel->setMaximumHeight(500);
    reviewLicenseImageLabel->setAlignment(Qt::AlignCenter);
    reviewLicenseImageLabel->setStyleSheet("border: 2px solid #bdc3c7; border-radius: 8px; background-color: #ecf0f1;");
    reviewLicenseImageLabel->setText("加载中...");
    reviewLicenseImageLabel->setScaledContents(true);
    licenseLayout->addWidget(reviewLicenseImageLabel);
    
    reviewLayout->addWidget(licenseGroup);
    
    // 操作按钮
    QHBoxLayout *reviewButtonLayout = new QHBoxLayout();
    approveBtn = new QPushButton("✓ 通过审核");
    approveBtn->setStyleSheet("background-color: #27ae60; color: white; padding: 12px 30px; font-size: 16px; font-weight: bold;");
    rejectBtn = new QPushButton("✗ 拒绝审核");
    rejectBtn->setStyleSheet("background-color: #e74c3c; color: white; padding: 12px 30px; font-size: 16px; font-weight: bold;");
    backFromReviewBtn = new QPushButton("返回");
    backFromReviewBtn->setStyleSheet("background-color: #95a5a6; color: white; padding: 12px 30px; font-size: 16px;");
    
    reviewButtonLayout->addWidget(approveBtn);
    reviewButtonLayout->addWidget(rejectBtn);
    reviewButtonLayout->addStretch();
    reviewButtonLayout->addWidget(backFromReviewBtn);
    
    reviewLayout->addLayout(reviewButtonLayout);
    reviewLayout->addStretch();
    
    stackedWidget->addWidget(reviewPage);
}

void BookAdmin::initConnections()
{
    // 登录
    connect(loginButton, &QPushButton::clicked, this, &BookAdmin::onLoginClicked);

    // 仪表盘导航
    connect(usersButton, &QPushButton::clicked, this, &BookAdmin::showUsersPage);
    connect(buyersButton, &QPushButton::clicked, this, &BookAdmin::showBuyersPage);
    connect(sellersButton, &QPushButton::clicked, this, &BookAdmin::showSellersPage);
    connect(reviewPendingSellersButton, &QPushButton::clicked, this, &BookAdmin::showReviewPendingSellersPage);
    connect(booksButton, &QPushButton::clicked, this, &BookAdmin::showBooksPage);
    connect(ordersButton, &QPushButton::clicked, this, &BookAdmin::showOrdersPage);
    connect(statsButton, &QPushButton::clicked, this, &BookAdmin::showStatsPage);
    connect(chatButton, &QPushButton::clicked, this, &BookAdmin::showChatPage);
    connect(appealsButton, &QPushButton::clicked, this, &BookAdmin::showAppealsPage);
    connect(logoutButton, &QPushButton::clicked, this, &BookAdmin::onLogoutClicked);
    
    // 仪表盘刷新按钮
    connect(dashboardRefreshButton, &QPushButton::clicked, this, &BookAdmin::loadStats);

    // 聊天管理
    connect(refreshChatBtn, &QPushButton::clicked, this, &BookAdmin::loadChatUsers);
    connect(chatUserList, &QListWidget::itemClicked, this, &BookAdmin::onChatUserSelected);
    connect(sendChatBtn, &QPushButton::clicked, this, &BookAdmin::onSendChatClicked);
    connect(backFromChatBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    
    // 申诉管理
    connect(refreshAppealsBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshAppealsClicked);
    connect(approveAppealBtn, &QPushButton::clicked, this, &BookAdmin::onApproveAppealClicked);
    connect(rejectAppealBtn, &QPushButton::clicked, this, &BookAdmin::onRejectAppealClicked);
    connect(backFromAppealsBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    connect(appealsTable, &QTableWidget::cellClicked, this, &BookAdmin::onAppealTableCellClicked);

    // 用户管理
    connect(refreshUsersBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshUsersClicked);
    connect(banUserBtn, &QPushButton::clicked, this, &BookAdmin::onBanUserClicked);
    connect(unbanUserBtn, &QPushButton::clicked, this, &BookAdmin::onUnbanUserClicked);
    connect(backFromUsersBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    connect(usersTable, &QTableWidget::cellClicked, this, &BookAdmin::onUserTableCellClicked);

    // 商家管理
    connect(refreshSellersBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshSellersClicked);
    connect(banSellerBtn, &QPushButton::clicked, this, &BookAdmin::onBanSellerClicked);
    connect(unbanSellerBtn, &QPushButton::clicked, this, &BookAdmin::onUnbanSellerClicked);
    connect(backFromSellersBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    connect(sellersTable, &QTableWidget::cellClicked, this, &BookAdmin::onSellerTableCellClicked);
    // 待审核商家认证申请
    connect(refreshPendingSellersBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshPendingSellersClicked);
    connect(reviewSellerBtn, &QPushButton::clicked, this, &BookAdmin::onReviewSellerClicked);
    connect(pendingSellersTable, &QTableWidget::cellClicked, this, [this](int row, int column) {
        Q_UNUSED(column);
        selectedPendingSellerRow = row;
    });

    // 图书管理
    connect(refreshBooksBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshBooksClicked);
    connect(editBookBtn, &QPushButton::clicked, this, &BookAdmin::onEditBookClicked);
    connect(deleteBookBtn, &QPushButton::clicked, this, &BookAdmin::onDeleteBookClicked);
    connect(backFromBooksBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    connect(booksTable, &QTableWidget::cellClicked, this, &BookAdmin::onBookTableCellClicked);
    // 待审核书籍
    connect(refreshPendingBooksBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshPendingBooksClicked);
    connect(approveBookBtn, &QPushButton::clicked, this, &BookAdmin::onApproveBookClicked);
    connect(rejectBookBtn, &QPushButton::clicked, this, &BookAdmin::onRejectBookClicked);

    // 订单管理
    connect(refreshOrdersBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshOrdersClicked);
    connect(viewOrderDetailsBtn, &QPushButton::clicked, this, &BookAdmin::onViewOrderDetailsClicked);
    connect(deleteOrderBtn, &QPushButton::clicked, this, &BookAdmin::onDeleteOrderClicked);
    connect(backFromOrdersBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    connect(ordersTable, &QTableWidget::cellClicked, this, &BookAdmin::onOrderTableCellClicked);

    // 统计
    connect(refreshStatsBtn, &QPushButton::clicked, this, &BookAdmin::onRefreshStatsClicked);
    connect(backFromStatsBtn, &QPushButton::clicked, this, &BookAdmin::showDashboardPage);
    
    // 审核
    connect(approveBtn, &QPushButton::clicked, this, &BookAdmin::onApproveCertificationClicked);
    connect(rejectBtn, &QPushButton::clicked, this, &BookAdmin::onRejectCertificationClicked);
    connect(backFromReviewBtn, &QPushButton::clicked, this, &BookAdmin::onBackFromReviewBtnClicked);
}

void BookAdmin::applyStyle()
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

// 登录功能
void BookAdmin::onLoginClicked()
{
    QString username = loginUsername->text().trimmed();
    QString password = loginPassword->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入账号和密码！");
        return;
    }

    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器！");
            return;
        }
        QThread::msleep(300);
    }

    QJsonObject response = apiService->login(username, password);
    
    if (response["success"].toBool())
    {
        currentAdminId = response["adminId"].toString();
        currentAdminName = username;
        isLoggedIn = true;
        
        welcomeLabel->setText("欢迎回来，" + currentAdminName + " 管理员");
        showDashboardPage();
        loadStats();  // 加载统计数据
        QMessageBox::information(this, "登录成功", "欢迎使用管理员系统！");
    } else {
        QString error = response["error"].toString();
        if (error.isEmpty()) error = response["message"].toString();
        QMessageBox::warning(this, "登录失败", "登录失败：" + error);
    }
}

void BookAdmin::onLogoutClicked()
{
    isLoggedIn = false;
    currentAdminId.clear();
    currentAdminName.clear();
    showLoginPage();
    QMessageBox::information(this, "提示", "已退出登录");
}

// 用户管理功能
void BookAdmin::onRefreshUsersClicked() { loadUsers(); }

void BookAdmin::onBanUserClicked()
{
    if (selectedUserRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要封禁的用户！");
        return;
    }
    
    QString userId = usersTable->item(selectedUserRow, 0)->text();
    QString username = usersTable->item(selectedUserRow, 1)->text();
    QString currentStatus = usersTable->item(selectedUserRow, 4)->text();  // 状态列
    
    if (currentStatus == "封禁") {
        QMessageBox::information(this, "提示", "该用户已被封禁，无需重复封禁");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认封禁", 
        QString("确定要封禁用户 \"%1\" 吗？\n\n封禁后该用户将无法登录和进行交易。").arg(username),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->banUser(currentAdminId, userId, true);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "用户已封禁");
            loadUsers();
        } else {
            QMessageBox::warning(this, "错误", "封禁失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onUnbanUserClicked()
{
    if (selectedUserRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要解封的用户！");
        return;
    }
    
    QString userId = usersTable->item(selectedUserRow, 0)->text();
    QString username = usersTable->item(selectedUserRow, 1)->text();
    QString currentStatus = usersTable->item(selectedUserRow, 4)->text();  // 状态列
    
    if (currentStatus != "封禁") {
        QMessageBox::information(this, "提示", "该用户未被封禁，无需解封");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认解封", 
        QString("确定要解封用户 \"%1\" 吗？\n\n解封后该用户可以正常登录和进行交易。").arg(username),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->banUser(currentAdminId, userId, false);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "用户已解封");
            loadUsers();
        } else {
            QMessageBox::warning(this, "错误", "解封失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onUserTableCellClicked(int row, int column)
{
    selectedUserRow = row;
    
    // 如果点击的是身份列（第8列，索引7）且身份为"审核中"，进入审核界面
    if (column == 7) {
        QTableWidgetItem *roleItem = usersTable->item(row, 7); // 身份列在第8列（索引7）
        if (roleItem && roleItem->text() == "审核中") {
            // 获取用户ID
            QTableWidgetItem *userIdItem = usersTable->item(row, 0);
            if (userIdItem) {
                QString userId = userIdItem->text();
                showReviewPage(userId.toInt());
                return;
            }
        }
    }
}

// 商家管理功能
void BookAdmin::onRefreshSellersClicked() { loadSellers(); }
void BookAdmin::onBanSellerClicked()
{
    if (selectedSellerRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要封禁的商家！");
        return;
    }
    
    QString sellerId = sellersTable->item(selectedSellerRow, 0)->text();
    QString sellerName = sellersTable->item(selectedSellerRow, 1)->text();
    QString currentStatus = sellersTable->item(selectedSellerRow, 4)->text();  // 状态列
    
    if (currentStatus == "封禁") {
        QMessageBox::information(this, "提示", "该商家已被封禁，无需重复封禁");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认封禁", 
        QString("确定要封禁商家 \"%1\" 吗？\n\n封禁后该商家将无法添加图书，其所有上架图书将自动下架。").arg(sellerName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->banSeller(currentAdminId, sellerId, true);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "商家已封禁，其所有图书已下架");
            loadSellers();
        } else {
            QMessageBox::warning(this, "错误", "封禁失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onUnbanSellerClicked()
{
    if (selectedSellerRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要解封的商家！");
        return;
    }
    
    QString sellerId = sellersTable->item(selectedSellerRow, 0)->text();
    QString sellerName = sellersTable->item(selectedSellerRow, 1)->text();
    QString currentStatus = sellersTable->item(selectedSellerRow, 4)->text();  // 状态列
    
    if (currentStatus != "封禁") {
        QMessageBox::information(this, "提示", "该商家未被封禁，无需解封");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认解封", 
        QString("确定要解封商家 \"%1\" 吗？\n\n解封后该商家可以正常添加图书。").arg(sellerName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->banSeller(currentAdminId, sellerId, false);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "商家已解封，其所有图书已恢复上架");
            loadSellers();
        } else {
            QMessageBox::warning(this, "错误", "解封失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onSellerTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedSellerRow = row;
}

// 图书管理功能
void BookAdmin::onRefreshBooksClicked() { loadBooks(); }
void BookAdmin::onEditBookClicked()
{
    if (selectedBookRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的图书！");
        return;
    }
    
    QString bookId = booksTable->item(selectedBookRow, 0)->text();
    
    // 获取当前图书信息
    QJsonObject response = apiService->getAllBooksGlobal(currentAdminId);
    QJsonObject currentBook;
    if (response["success"].toBool()) {
        QJsonArray books = response["books"].toArray();
        for (const QJsonValue &bookVal : books) {
            QJsonObject book = bookVal.toObject();
            if (book["isbn"].toString() == bookId) {
                currentBook = book;
                break;
            }
        }
    }
    
    if (currentBook.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该图书信息");
        return;
    }
    
    // 创建编辑对话框
    QDialog dialog(this);
    dialog.setWindowTitle("编辑图书");
    dialog.setMinimumWidth(500);
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    // ISBN（只读）
    QLineEdit *isbnEdit = new QLineEdit(currentBook["isbn"].toString());
    isbnEdit->setReadOnly(true);
    
    QLineEdit *titleEdit = new QLineEdit(currentBook["title"].toString());
    QLineEdit *authorEdit = new QLineEdit(currentBook["author"].toString());
    
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
    
    // 二级分类下拉框
    QComboBox *subCategoryCombo = new QComboBox();
    
    // 定义分类映射
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
    categoryMap["其他"] = QStringList();
    
    // 设置当前分类
    QString currentCategory1 = currentBook["category1"].toString();
    QString currentCategory2 = currentBook["category2"].toString();
    int categoryIndex = categoryCombo->findText(currentCategory1);
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
    
    // 初始化二级分类
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
    priceEdit->setValue(currentBook["price"].toDouble());
    
    QSpinBox *stockEdit = new QSpinBox();
    stockEdit->setRange(0, 999999);
    stockEdit->setValue(currentBook["stock"].toInt());
    
    QComboBox *statusCombo = new QComboBox();
    statusCombo->addItem("正常");
    statusCombo->addItem("下架");
    statusCombo->addItem("缺货");
    int statusIndex = statusCombo->findText(currentBook["status"].toString());
    if (statusIndex >= 0) {
        statusCombo->setCurrentIndex(statusIndex);
    }
    
    form->addRow("ISBN:", isbnEdit);
    form->addRow("书名:", titleEdit);
    form->addRow("作者:", authorEdit);
    form->addRow("一级分类:", categoryCombo);
    form->addRow("二级分类:", subCategoryCombo);
    form->addRow("价格:", priceEdit);
    form->addRow("库存:", stockEdit);
    form->addRow("状态:", statusCombo);
    
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
        bookData["status"] = statusCombo->currentText();
        
        QJsonObject updateResponse = apiService->updateBookGlobal(currentAdminId, bookId, bookData);
        
        if (updateResponse["success"].toBool()) {
            QMessageBox::information(this, "成功", "图书更新成功！");
            loadBooks();
        } else {
            QMessageBox::warning(this, "错误", "更新图书失败：" + updateResponse["message"].toString());
        }
    }
}

void BookAdmin::onDeleteBookClicked()
{
    if (selectedBookRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的图书！");
        return;
    }
    QString bookId = booksTable->item(selectedBookRow, 0)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要删除该图书吗？", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->deleteBookGlobal(currentAdminId, bookId);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "图书已删除");
            loadBooks();
        } else {
            QMessageBox::warning(this, "错误", "删除失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onBookTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedBookRow = row;
}

// 订单管理功能
void BookAdmin::onRefreshOrdersClicked() { loadOrders(); }
void BookAdmin::onViewOrderDetailsClicked()
{
    QMessageBox::information(this, "提示", "订单详情：显示完整的订单信息和商品列表");
}

void BookAdmin::onDeleteOrderClicked()
{
    if (selectedOrderRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的订单！");
        return;
    }
    QString orderId = ordersTable->item(selectedOrderRow, 0)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要删除该订单吗？", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->deleteOrderGlobal(currentAdminId, orderId);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "订单已删除");
            loadOrders();
        } else {
            QMessageBox::warning(this, "错误", "删除失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onOrderTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    selectedOrderRow = row;
}

// 统计功能
void BookAdmin::onRefreshStatsClicked() { loadStats(); }

// 页面切换
void BookAdmin::showLoginPage() { stackedWidget->setCurrentWidget(loginPage); }
void BookAdmin::showDashboardPage() 
{ 
    // 停止聊天刷新定时器
    if (chatRefreshTimer && chatRefreshTimer->isActive()) {
        chatRefreshTimer->stop();
    }
    
    loadStats();  // 立即加载一次统计数据
    
    // 启动仪表盘自动刷新定时器
    if (dashboardRefreshTimer && !dashboardRefreshTimer->isActive()) {
        dashboardRefreshTimer->start();
    }
    
    stackedWidget->setCurrentWidget(dashboardPage); 
}
void BookAdmin::showUsersPage() { 
    if (usersTitleLabel) {
        usersTitleLabel->setText("用户管理");
    }
    loadUsers(); 
    stackedWidget->setCurrentWidget(usersPage); 
}
void BookAdmin::showBuyersPage() { 
    if (usersTitleLabel) {
        usersTitleLabel->setText("买家管理");
    }
    loadBuyers(); 
    stackedWidget->setCurrentWidget(usersPage); 
}
void BookAdmin::showSellersPage() { 
    loadSellers(); 
    loadPendingSellers();  // 同时加载待审核商家认证申请
    stackedWidget->setCurrentWidget(sellersPage);
    // 默认显示"所有商家"标签页
    if (sellersTabWidget) {
        sellersTabWidget->setCurrentIndex(0);
    }
}
void BookAdmin::showReviewPendingSellersPage() {
    loadSellers();
    loadPendingSellers();  // 加载待审核商家认证申请
    stackedWidget->setCurrentWidget(sellersPage);
    // 切换到"待审核商家认证"标签页（索引1）
    if (sellersTabWidget) {
        sellersTabWidget->setCurrentIndex(1);
    }
}
void BookAdmin::showBooksPage() { 
    loadBooks(); 
    loadPendingBooks();  // 同时加载待审核书籍
    stackedWidget->setCurrentWidget(booksPage); 
}
void BookAdmin::showOrdersPage() { loadOrders(); stackedWidget->setCurrentWidget(ordersPage); }
void BookAdmin::showStatsPage() { loadStats(); stackedWidget->setCurrentWidget(statsPage); }

void BookAdmin::showAppealsPage()
{
    onRefreshAppealsClicked();
    stackedWidget->setCurrentWidget(appealsPage);
}

void BookAdmin::showChatPage()
{
    if (!isLoggedIn || currentAdminId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    loadChatUsers();
    stackedWidget->setCurrentWidget(chatPage);
    
    // 启动聊天刷新定时器
    if (!chatRefreshTimer->isActive()) {
        chatRefreshTimer->start();
    }
}

void BookAdmin::loadChatUsers()
{
    if (!isLoggedIn || currentAdminId.isEmpty()) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 获取所有用户和卖家，显示在列表中
    chatUserList->clear();
    
    // 获取所有买家
    QJsonObject usersResponse = apiService->getAllUsers(currentAdminId);
    if (usersResponse["success"].toBool()) {
        QJsonArray users = usersResponse["users"].toArray();
        for (const QJsonValue &userVal : users) {
            QJsonObject user = userVal.toObject();
            QString itemText = QString("买家: %1 (ID: %2)").arg(user["username"].toString()).arg(user["userId"].toInt());
            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, user["userId"].toInt());
            item->setData(Qt::UserRole + 1, "buyer");
            chatUserList->addItem(item);
        }
    }
    
    // 获取所有卖家
    QJsonObject sellersResponse = apiService->getAllSellers(currentAdminId);
    if (sellersResponse["success"].toBool()) {
        QJsonArray sellers = sellersResponse["sellers"].toArray();
        for (const QJsonValue &sellerVal : sellers) {
            QJsonObject seller = sellerVal.toObject();
            QString itemText = QString("卖家: %1 (ID: %2)").arg(seller["sellerName"].toString()).arg(seller["sellerId"].toInt());
            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, seller["sellerId"].toInt());
            item->setData(Qt::UserRole + 1, "seller");
            chatUserList->addItem(item);
        }
    }
}

void BookAdmin::onChatUserSelected()
{
    QListWidgetItem *item = chatUserList->currentItem();
    if (!item) {
        return;
    }
    
    currentChatUserId = item->data(Qt::UserRole).toInt();
    currentChatUserType = item->data(Qt::UserRole + 1).toString();
    
    // 加载与该用户的聊天历史
    loadChatHistory();
}

void BookAdmin::loadChatHistory()
{
    if (!isLoggedIn || currentAdminId.isEmpty()) {
        return;
    }
    
    if (currentChatUserId <= 0 || currentChatUserType.isEmpty()) {
        chatDisplay->clear();
        chatDisplay->append("请从左侧列表选择一个用户查看聊天记录");
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 获取与当前用户的聊天历史（管理员视角：查看与特定用户的聊天）
    // 注意：这里需要交换参数，因为管理员要查看与用户的聊天，所以管理员是"otherUser"
    QJsonObject response = apiService->getChatHistory(
        currentAdminId,  // 管理员ID
        "admin",         // 管理员类型
        QString::number(currentChatUserId),  // 对方用户ID
        currentChatUserType  // 对方用户类型
    );
    
    if (response["success"].toBool()) {
        chatDisplay->clear();
        QJsonArray messages = response["messages"].toArray();
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            QString content = msg["content"].toString();
            QString sendTime = msg["sendTime"].toString();
            
            // 格式化时间
            QDateTime dateTime = QDateTime::fromString(sendTime, "yyyy-MM-dd hh:mm:ss");
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(sendTime, Qt::ISODate);
            }
            QString timeStr = dateTime.isValid() ? dateTime.toString("yyyy-MM-dd hh:mm") : sendTime;
            
            // 显示消息
            QString senderName;
            if (senderType == "admin") {
                senderName = "我（管理员）";
            } else if (senderType == "buyer") {
                senderName = "买家";
            } else if (senderType == "seller") {
                senderName = "卖家";
            } else {
                senderName = "未知";
            }
            
            chatDisplay->append(QString("[%1] %2: %3").arg(timeStr).arg(senderName).arg(content));
        }
        
        // 滚动到底部
        QTextCursor cursor = chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        chatDisplay->setTextCursor(cursor);
    }
}

void BookAdmin::onSendChatClicked()
{
    if (!isLoggedIn || currentAdminId.isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        return;
    }
    
    if (currentChatUserId <= 0 || currentChatUserType.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请先选择一个用户");
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
    
    // 发送消息给选中的用户
    QJsonObject response = apiService->sendChatMessage(
        currentAdminId,
        "admin",
        QString::number(currentChatUserId),  // 接收者ID
        currentChatUserType,  // 接收者类型
        message  // 消息内容
    );
    
    if (response["success"].toBool()) {
        chatInput->clear();
        
        // 重新加载聊天历史以显示最新消息
        loadChatHistory();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}

void BookAdmin::onRefreshAppealsClicked()
{
    if (currentAdminId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }
    
    QJsonObject response = apiService->getAllAppeals(currentAdminId);
    
    if (response["success"].toBool()) {
        QJsonArray appeals = response["appeals"].toArray();
        appealsTable->setRowCount(appeals.size());
        
        for (int i = 0; i < appeals.size(); ++i) {
            QJsonObject appeal = appeals[i].toObject();
            appealsTable->setItem(i, 0, new QTableWidgetItem(QString::number(appeal["appealId"].toInt())));
            appealsTable->setItem(i, 1, new QTableWidgetItem(QString::number(appeal["sellerId"].toInt())));
            appealsTable->setItem(i, 2, new QTableWidgetItem(appeal["sellerName"].toString()));
            appealsTable->setItem(i, 3, new QTableWidgetItem(appeal["appealReason"].toString()));
            appealsTable->setItem(i, 4, new QTableWidgetItem(appeal["submitTime"].toString()));
            
            QString status = appeal["status"].toString();
            QTableWidgetItem *statusItem = new QTableWidgetItem(status);
            if (status == "待审核") {
                statusItem->setForeground(QBrush(QColor("#f39c12")));
            } else if (status == "已通过") {
                statusItem->setForeground(QBrush(QColor("#27ae60")));
            } else if (status == "未通过") {
                statusItem->setForeground(QBrush(QColor("#e74c3c")));
            }
            appealsTable->setItem(i, 5, statusItem);
            
            QString reviewComment = appeal["reviewComment"].toString();
            appealsTable->setItem(i, 6, new QTableWidgetItem(reviewComment.isEmpty() ? "-" : reviewComment));
        }
    } else {
        QMessageBox::warning(this, "错误", "获取申诉列表失败：" + response["message"].toString());
    }
}

void BookAdmin::onAppealTableCellClicked(int row, int column)
{
    selectedAppealRow = row;
}

void BookAdmin::onApproveAppealClicked()
{
    if (selectedAppealRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要审核的申诉");
        return;
    }
    
    int appealId = appealsTable->item(selectedAppealRow, 0)->text().toInt();
    QString currentStatus = appealsTable->item(selectedAppealRow, 5)->text();
    
    if (currentStatus != "待审核") {
        QMessageBox::warning(this, "提示", "只能审核待审核状态的申诉");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认通过", 
        "确定要通过此申诉吗？通过后卖家将自动解封。", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    QString reviewComment = appealReviewCommentEdit->toPlainText().trimmed();
    QJsonObject response = apiService->reviewAppeal(currentAdminId, appealId, "已通过", reviewComment);
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", response["message"].toString());
        appealReviewCommentEdit->clear();
        onRefreshAppealsClicked();
    } else {
        QMessageBox::warning(this, "错误", "审核失败：" + response["message"].toString());
    }
}

void BookAdmin::onRejectAppealClicked()
{
    if (selectedAppealRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要审核的申诉");
        return;
    }
    
    int appealId = appealsTable->item(selectedAppealRow, 0)->text().toInt();
    QString currentStatus = appealsTable->item(selectedAppealRow, 5)->text();
    
    if (currentStatus != "待审核") {
        QMessageBox::warning(this, "提示", "只能审核待审核状态的申诉");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认拒绝", 
        "确定要拒绝此申诉吗？", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    QString reviewComment = appealReviewCommentEdit->toPlainText().trimmed();
    if (reviewComment.isEmpty()) {
        QMessageBox::warning(this, "提示", "拒绝申诉时建议填写审核意见");
        return;
    }
    
    QJsonObject response = apiService->reviewAppeal(currentAdminId, appealId, "未通过", reviewComment);
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", response["message"].toString());
        appealReviewCommentEdit->clear();
        onRefreshAppealsClicked();
    } else {
        QMessageBox::warning(this, "错误", "审核失败：" + response["message"].toString());
    }
}

void BookAdmin::showReviewPage(int userId)
{
    currentReviewUserId = userId;
    
    // 获取认证信息
    QJsonObject response = apiService->getSellerCertification(currentAdminId, QString::number(userId));
    
    if (response["success"].toBool()) {
        QJsonObject cert = response["certification"].toObject();
        
        // 显示用户信息
        reviewUserIdLabel->setText(QString::number(cert["userId"].toInt()));
        reviewUsernameLabel->setText(cert["username"].toString());
        reviewEmailLabel->setText(cert["email"].toString());
        reviewApplyTimeLabel->setText(cert["applyTime"].toString());
        
        // 显示营业执照图片
        QString licenseImageBase64 = cert["licenseImage"].toString();
        if (!licenseImageBase64.isEmpty()) {
            QByteArray imageData = QByteArray::fromBase64(licenseImageBase64.toUtf8());
            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                // 缩放图片以适应标签大小
                QSize labelSize = reviewLicenseImageLabel->size();
                if (labelSize.width() > 0 && labelSize.height() > 0) {
                    pixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                reviewLicenseImageLabel->setPixmap(pixmap);
            } else {
                reviewLicenseImageLabel->setText("图片加载失败");
            }
        } else {
            reviewLicenseImageLabel->setText("未上传营业执照图片");
        }
        
        stackedWidget->setCurrentWidget(reviewPage);
    } else {
        QMessageBox::warning(this, "错误", "获取认证信息失败：" + response["message"].toString());
    }
}

void BookAdmin::onApproveCertificationClicked()
{
    if (currentReviewUserId < 0) {
        QMessageBox::warning(this, "错误", "无效的用户ID");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要通过该用户的卖家认证申请吗？", 
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->approveSellerCertification(
            currentAdminId, QString::number(currentReviewUserId));
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "审核通过，用户已成功成为卖家");
            // 返回用户管理页面并刷新
            showUsersPage();
            loadUsers();
        } else {
            QMessageBox::warning(this, "错误", "审核失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onRejectCertificationClicked()
{
    if (currentReviewUserId < 0) {
        QMessageBox::warning(this, "错误", "无效的用户ID");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要拒绝该用户的卖家认证申请吗？", 
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QJsonObject response = apiService->rejectSellerCertification(
            currentAdminId, QString::number(currentReviewUserId));
        
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "审核已拒绝");
            // 返回用户管理页面并刷新
            showUsersPage();
            loadUsers();
        } else {
            QMessageBox::warning(this, "错误", "拒绝失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onBackFromReviewBtnClicked()
{
    showUsersPage();
}

// 数据加载
void BookAdmin::loadUsers()
{
    QJsonObject response = apiService->getAllUsers(currentAdminId);
    if (response["success"].toBool()) {
        QJsonArray users = response["users"].toArray();
        usersTable->setRowCount(0);
        for (const QJsonValue &userVal : users) {
            QJsonObject user = userVal.toObject();
            int row = usersTable->rowCount();
            usersTable->insertRow(row);
            usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["userId"].toInt())));
            usersTable->setItem(row, 1, new QTableWidgetItem(user["username"].toString()));
            usersTable->setItem(row, 2, new QTableWidgetItem(user["email"].toString()));
            usersTable->setItem(row, 3, new QTableWidgetItem(user["registerDate"].toString()));
            usersTable->setItem(row, 4, new QTableWidgetItem(user["status"].toString()));
            usersTable->setItem(row, 5, new QTableWidgetItem(QString::number(user["balance"].toDouble(), 'f', 2)));
            
            // 显示会员等级（放在身份列前面），用数字表示（1-5）
            int membershipLevelInt = 1;
            if (user.contains("membershipLevel")) {
                QJsonValue levelVal = user["membershipLevel"];
                if (levelVal.isDouble()) {
                    membershipLevelInt = levelVal.toInt();
                } else {
                    QString levelStr = levelVal.toString();
                    // 兼容旧数据：如果是文本，转换为整数
                    if (levelStr == "普通") membershipLevelInt = 1;
                    else if (levelStr == "银卡") membershipLevelInt = 2;
                    else if (levelStr == "金卡") membershipLevelInt = 3;
                    else if (levelStr == "白金") membershipLevelInt = 4;
                    else if (levelStr == "钻石") membershipLevelInt = 5;
                    else membershipLevelInt = levelStr.toInt();
                }
            }
            if (membershipLevelInt < 1 || membershipLevelInt > 5) {
                membershipLevelInt = 1;
            }
            usersTable->setItem(row, 6, new QTableWidgetItem(QString::number(membershipLevelInt)));
            
            // 显示身份：0-审核中，1-买家，2-卖家
            int role = user["role"].toInt();
            QString roleText;
            switch (role) {
                case 0:
                    roleText = "审核中";
                    break;
                case 1:
                    roleText = "买家";
                    break;
                case 2:
                    roleText = "卖家";
                    break;
                default:
                    roleText = "未知";
                    break;
            }
            QTableWidgetItem *roleItem = new QTableWidgetItem(roleText);
            usersTable->setItem(row, 7, roleItem);
            
            // 如果身份是"审核中"，设置为可点击样式，提示管理员可以点击审核
            if (roleText == "审核中") {
                roleItem->setForeground(QBrush(QColor(255, 140, 0))); // 橙色
                roleItem->setToolTip("点击此处进入审核界面");
                roleItem->setFlags(roleItem->flags() | Qt::ItemIsEnabled);
            }
        }
    }
}

void BookAdmin::loadBuyers()
{
    // 加载买家（只显示role=1的用户）
    QJsonObject response = apiService->getAllUsers(currentAdminId);
    if (response["success"].toBool()) {
        QJsonArray users = response["users"].toArray();
        usersTable->setRowCount(0);
        for (const QJsonValue &userVal : users) {
            QJsonObject user = userVal.toObject();
            
            // 只显示买家（role=1）
            int role = user["role"].toInt();
            if (role != 1) {
                continue;  // 跳过非买家用户
            }
            
            int row = usersTable->rowCount();
            usersTable->insertRow(row);
            usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["userId"].toInt())));
            usersTable->setItem(row, 1, new QTableWidgetItem(user["username"].toString()));
            usersTable->setItem(row, 2, new QTableWidgetItem(user["email"].toString()));
            usersTable->setItem(row, 3, new QTableWidgetItem(user["registerDate"].toString()));
            usersTable->setItem(row, 4, new QTableWidgetItem(user["status"].toString()));
            usersTable->setItem(row, 5, new QTableWidgetItem(QString::number(user["balance"].toDouble(), 'f', 2)));
            
            // 显示会员等级（放在身份列前面），用数字表示（1-5）
            int membershipLevelInt = 1;
            if (user.contains("membershipLevel")) {
                QJsonValue levelVal = user["membershipLevel"];
                if (levelVal.isDouble()) {
                    membershipLevelInt = levelVal.toInt();
                } else {
                    QString levelStr = levelVal.toString();
                    // 兼容旧数据：如果是文本，转换为整数
                    if (levelStr == "普通") membershipLevelInt = 1;
                    else if (levelStr == "银卡") membershipLevelInt = 2;
                    else if (levelStr == "金卡") membershipLevelInt = 3;
                    else if (levelStr == "白金") membershipLevelInt = 4;
                    else if (levelStr == "钻石") membershipLevelInt = 5;
                    else membershipLevelInt = levelStr.toInt();
                }
            }
            if (membershipLevelInt < 1 || membershipLevelInt > 5) {
                membershipLevelInt = 1;
            }
            usersTable->setItem(row, 6, new QTableWidgetItem(QString::number(membershipLevelInt)));
            
            // 显示身份（买家）
            QTableWidgetItem *roleItem = new QTableWidgetItem("买家");
            usersTable->setItem(row, 7, roleItem);
        }
    }
}

void BookAdmin::loadSellers()
{
    QJsonObject response = apiService->getAllSellers(currentAdminId);
    if (response["success"].toBool()) {
        QJsonArray sellers = response["sellers"].toArray();
        sellersTable->setRowCount(0);
        for (const QJsonValue &sellerVal : sellers) {
            QJsonObject seller = sellerVal.toObject();
            int row = sellersTable->rowCount();
            sellersTable->insertRow(row);
            sellersTable->setItem(row, 0, new QTableWidgetItem(QString::number(seller["sellerId"].toInt())));
            sellersTable->setItem(row, 1, new QTableWidgetItem(seller["sellerName"].toString()));
            sellersTable->setItem(row, 2, new QTableWidgetItem(seller["email"].toString()));
            sellersTable->setItem(row, 3, new QTableWidgetItem(seller["registerDate"].toString()));
            sellersTable->setItem(row, 4, new QTableWidgetItem(seller["status"].toString()));
        }
    }
}

void BookAdmin::loadPendingSellers()
{
    // 从users表获取role=0的用户（待审核商家）
    QJsonObject response = apiService->getAllUsers(currentAdminId);
    if (response["success"].toBool()) {
        QJsonArray users = response["users"].toArray();
        pendingSellersTable->setRowCount(0);
        for (const QJsonValue &userVal : users) {
            QJsonObject user = userVal.toObject();
            
            // 只显示role=0的用户（审核中）
            int role = user["role"].toInt();
            if (role != 0) {
                continue;  // 跳过非待审核用户
            }
            
            int row = pendingSellersTable->rowCount();
            pendingSellersTable->insertRow(row);
            pendingSellersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["userId"].toInt())));
            pendingSellersTable->setItem(row, 1, new QTableWidgetItem(user["username"].toString()));
            pendingSellersTable->setItem(row, 2, new QTableWidgetItem(user["email"].toString()));
            
            // 申请时间：优先使用applyTime，如果没有则使用registerDate
            QString applyTime = user["applyTime"].toString();
            if (applyTime.isEmpty()) {
                applyTime = user["registerDate"].toString();
            }
            pendingSellersTable->setItem(row, 3, new QTableWidgetItem(applyTime));
            
            // 状态固定为"审核中"
            pendingSellersTable->setItem(row, 4, new QTableWidgetItem("审核中"));
        }
    } else {
        QString errorMsg = response["message"].toString();
        qDebug() << "加载待审核商家申请失败:" << errorMsg;
    }
}

void BookAdmin::onRefreshPendingSellersClicked()
{
    loadPendingSellers();
}

void BookAdmin::onReviewSellerClicked()
{
    if (selectedPendingSellerRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要查看的商家认证申请");
        return;
    }
    
    int userId = pendingSellersTable->item(selectedPendingSellerRow, 0)->text().toInt();
    showReviewPage(userId);
}

void BookAdmin::loadBooks()
{
    qDebug() << "开始加载图书列表...";
    QJsonObject response = apiService->getAllBooksGlobal(currentAdminId);
    
    if (response["success"].toBool()) {
        QJsonArray books = response["books"].toArray();
        qDebug() << "获取到" << books.size() << "本图书";
        
        booksTable->setRowCount(0);
        
        if (books.isEmpty()) {
            qDebug() << "图书列表为空";
            QMessageBox::information(this, "提示", "当前没有图书数据");
            return;
        }
        
        for (const QJsonValue &bookVal : books) {
            QJsonObject book = bookVal.toObject();
            int row = booksTable->rowCount();
            booksTable->insertRow(row);
            
            // 获取分类信息（优先使用category字段，如果没有则组合category1和category2）
            QString category = book["category"].toString();
            if (category.isEmpty()) {
                QString category1 = book["category1"].toString();
                QString category2 = book["category2"].toString();
                category = category1;
                if (!category2.isEmpty()) {
                    category += " / " + category2;
                }
            }
            
            booksTable->setItem(row, 0, new QTableWidgetItem(book["isbn"].toString()));
            booksTable->setItem(row, 1, new QTableWidgetItem(book["title"].toString()));
            booksTable->setItem(row, 2, new QTableWidgetItem(book["author"].toString()));
            booksTable->setItem(row, 3, new QTableWidgetItem(category));
            booksTable->setItem(row, 4, new QTableWidgetItem(QString::number(book["price"].toDouble(), 'f', 2)));
            booksTable->setItem(row, 5, new QTableWidgetItem(QString::number(book["stock"].toInt())));
            booksTable->setItem(row, 6, new QTableWidgetItem(book["status"].toString()));
        }
        
        qDebug() << "图书列表加载完成，共" << booksTable->rowCount() << "行";
    } else {
        QString errorMsg = response["message"].toString();
        qDebug() << "加载图书失败:" << errorMsg;
        QMessageBox::warning(this, "错误", "加载图书列表失败：" + errorMsg);
        booksTable->setRowCount(0);
    }
}

void BookAdmin::loadPendingBooks()
{
    qDebug() << "开始加载待审核书籍列表...";
    QJsonObject response = apiService->getPendingBooks(currentAdminId);
    
    if (response["success"].toBool()) {
        QJsonArray books = response["books"].toArray();
        qDebug() << "获取到" << books.size() << "本待审核书籍";
        
        pendingBooksTable->setRowCount(0);
        
        if (books.isEmpty()) {
            qDebug() << "待审核书籍列表为空";
            return;
        }
        
        for (const QJsonValue &bookVal : books) {
            QJsonObject book = bookVal.toObject();
            int row = pendingBooksTable->rowCount();
            pendingBooksTable->insertRow(row);
            
            // 获取分类信息
            QString category = book["category"].toString();
            if (category.isEmpty()) {
                QString category1 = book["category1"].toString();
                QString category2 = book["category2"].toString();
                category = category1;
                if (!category2.isEmpty()) {
                    category += " / " + category2;
                }
            }
            
            pendingBooksTable->setItem(row, 0, new QTableWidgetItem(book["isbn"].toString()));
            pendingBooksTable->setItem(row, 1, new QTableWidgetItem(book["title"].toString()));
            pendingBooksTable->setItem(row, 2, new QTableWidgetItem(book["author"].toString()));
            pendingBooksTable->setItem(row, 3, new QTableWidgetItem(category));
            pendingBooksTable->setItem(row, 4, new QTableWidgetItem(QString::number(book["price"].toDouble(), 'f', 2)));
            pendingBooksTable->setItem(row, 5, new QTableWidgetItem(QString::number(book["stock"].toInt())));
            pendingBooksTable->setItem(row, 6, new QTableWidgetItem(QString::number(book["merchantId"].toInt())));
        }
        
        qDebug() << "待审核书籍列表加载完成，共" << pendingBooksTable->rowCount() << "行";
    } else {
        QString errorMsg = response["message"].toString();
        qDebug() << "加载待审核书籍失败:" << errorMsg;
        QMessageBox::warning(this, "错误", "加载待审核书籍列表失败：" + errorMsg);
        pendingBooksTable->setRowCount(0);
    }
}

void BookAdmin::onRefreshPendingBooksClicked()
{
    loadPendingBooks();
}

void BookAdmin::onApproveBookClicked()
{
    int currentRow = pendingBooksTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要审核的书籍");
        return;
    }
    
    QString isbn = pendingBooksTable->item(currentRow, 0)->text();
    QString title = pendingBooksTable->item(currentRow, 1)->text();
    
    int ret = QMessageBox::question(this, "确认", 
                                     QString("确定要审核通过书籍《%1》吗？").arg(title),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QJsonObject response = apiService->approveBook(currentAdminId, isbn);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "审核通过成功");
            loadPendingBooks();  // 刷新列表
        } else {
            QMessageBox::warning(this, "错误", "审核通过失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::onRejectBookClicked()
{
    int currentRow = pendingBooksTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要审核的书籍");
        return;
    }
    
    QString isbn = pendingBooksTable->item(currentRow, 0)->text();
    QString title = pendingBooksTable->item(currentRow, 1)->text();
    
    int ret = QMessageBox::question(this, "确认", 
                                     QString("确定要拒绝书籍《%1》吗？").arg(title),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QJsonObject response = apiService->rejectBook(currentAdminId, isbn);
        if (response["success"].toBool()) {
            QMessageBox::information(this, "成功", "审核拒绝成功");
            loadPendingBooks();  // 刷新列表
        } else {
            QMessageBox::warning(this, "错误", "审核拒绝失败：" + response["message"].toString());
        }
    }
}

void BookAdmin::loadOrders()
{
    QJsonObject response = apiService->getAllOrdersGlobal(currentAdminId);
    if (response["success"].toBool()) {
        QJsonArray orders = response["orders"].toArray();
        ordersTable->setRowCount(0);
        for (const QJsonValue &orderVal : orders) {
            QJsonObject order = orderVal.toObject();
            int row = ordersTable->rowCount();
            ordersTable->insertRow(row);
            ordersTable->setItem(row, 0, new QTableWidgetItem(order["orderId"].toString()));
            ordersTable->setItem(row, 1, new QTableWidgetItem(order["userId"].toString()));
            ordersTable->setItem(row, 2, new QTableWidgetItem(QString::number(order["totalAmount"].toDouble(), 'f', 2)));
            ordersTable->setItem(row, 3, new QTableWidgetItem(order["status"].toString()));
            ordersTable->setItem(row, 4, new QTableWidgetItem(order["orderDate"].toString()));
            ordersTable->setItem(row, 5, new QTableWidgetItem(order["items"].toString()));
        }
    }
}

void BookAdmin::loadStats()
{
    QJsonObject response = apiService->getSystemStats(currentAdminId);
    if (response["success"].toBool()) {
        QJsonObject stats = response["stats"].toObject();
        
        // 更新仪表盘统计卡片
        totalUsersLabel->setText(QString::number(stats["totalUsers"].toInt()));
        totalBuyersLabel->setText(QString::number(stats["totalBuyers"].toInt()));
        totalSellersLabel->setText(QString::number(stats["totalSellers"].toInt()));
        pendingSellersLabel->setText(QString::number(stats["pendingSellers"].toInt()));
        totalBooksLabel->setText(QString::number(stats["totalBooks"].toInt()));
        totalOrdersLabel->setText(QString::number(stats["totalOrders"].toInt()));
        totalAppealsLabel->setText(QString::number(stats["totalAppeals"].toInt()));
        
        // 验证：总买家数 + 总商家数 + 待审核商家数 = 总用户数
        int totalBuyers = stats["totalBuyers"].toInt();
        int totalSellers = stats["totalSellers"].toInt();
        int pendingSellers = stats["pendingSellers"].toInt();
        int totalUsers = stats["totalUsers"].toInt();
        int calculatedTotal = totalBuyers + totalSellers + pendingSellers;
        
        if (calculatedTotal != totalUsers) {
            qDebug() << "警告：统计数据不一致！总买家数(" << totalBuyers 
                     << ") + 总商家数(" << totalSellers 
                     << ") + 待审核商家数(" << pendingSellers 
                     << ") = " << calculatedTotal 
                     << "，但总用户数 = " << totalUsers;
        }
        
        // 更新统计页面文本
        QString statsText = QString("=== 系统统计信息 ===\n\n");
        statsText += QString("总用户数: %1\n").arg(stats["totalUsers"].toInt());
        statsText += QString("总买家数: %1\n").arg(stats["totalBuyers"].toInt());
        statsText += QString("总商家数: %1\n").arg(stats["totalSellers"].toInt());
        statsText += QString("待审核商家数: %1\n").arg(stats["pendingSellers"].toInt());
        statsText += QString("总图书数: %1\n").arg(stats["totalBooks"].toInt());
        statsText += QString("总订单数: %1\n").arg(stats["totalOrders"].toInt());
        statsText += QString("\n验证：总买家数 + 总商家数 + 待审核商家数 = %1 + %2 + %3 = %4")
                     .arg(totalBuyers).arg(totalSellers).arg(pendingSellers).arg(calculatedTotal);
        if (calculatedTotal == totalUsers) {
            statsText += QString(" ✓ (与总用户数一致)");
        } else {
            statsText += QString(" ✗ (与总用户数 %1 不一致)").arg(totalUsers);
        }
        statsText += QString("\n\n系统运行正常");
        
        if (statsDisplay) {
            statsDisplay->setPlainText(statsText);
        }
    }
}

