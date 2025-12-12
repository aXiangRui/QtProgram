#include "Seller.h"
#include "LoginWidget.h"
#include "DashboardWidget.h"
#include "BookManageWidget.h"
#include "OrderManageWidget.h"
#include "MemberWidget.h"
#include "ReportWidget.h"
#include "NetworkClient.h"
#include"SystemManageWidget.h"
#include <QStackedWidget>
#include <QMessageBox>
#include <QTimer>

// 本地配置常量
namespace {
    const QColor PRIMARY_COLOR(41, 128, 185);
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 700;
    const int NAV_WIDTH = 200;
}

Seller::Seller(QWidget *parent)
    : QWidget(parent), isLoggedIn(false)
{
    setWindowTitle("图书商家管理系统");
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // 创建网络客户端
    networkClient = new NetworkClient(this);

    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建导航区（初始隐藏）
    navWidget = new QWidget;
    navWidget->setFixedWidth(NAV_WIDTH);
    navWidget->setObjectName("navWidget");
    navWidget->hide();

    // 创建内容区
    contentStack = new QStackedWidget;
    contentStack->setObjectName("contentStack");

    // 创建各个页面
    loginPage = new LoginWidget;
    dashboardPage = new DashboardWidget;
    bookManagePage = new BookManageWidget;
    orderManagePage = new OrderManageWidget;
    memberPage = new MemberWidget;
    reportPage = new ReportWidget;
    systemPage=new SystemManageWidget;

    // 添加到堆栈
    contentStack->addWidget(loginPage);
    contentStack->addWidget(dashboardPage);
    contentStack->addWidget(bookManagePage);
    contentStack->addWidget(orderManagePage);
    contentStack->addWidget(memberPage);
    contentStack->addWidget(reportPage);
    contentStack->addWidget(systemPage);

    // 布局
    mainLayout->addWidget(navWidget);
    mainLayout->addWidget(contentStack, 1);

    // 设置当前页面为登录页
    contentStack->setCurrentWidget(loginPage);

    // 创建导航栏
    createNavigation();

    // 应用样式
    applyStyle();

    // 连接信号
    connect(loginPage, SIGNAL(loginSuccess()), this, SLOT(onLoginSuccess()));
    connect(navDashboard, SIGNAL(clicked()), this, SLOT(showDashboard()));
    connect(navBooks, SIGNAL(clicked()), this, SLOT(showBookManage()));
    connect(navOrders, SIGNAL(clicked()), this, SLOT(showOrderManage()));
    connect(navMembers, SIGNAL(clicked()), this, SLOT(showMemberManage()));
    connect(navReports, SIGNAL(clicked()), this, SLOT(showReport()));
    connect(navLogout, SIGNAL(clicked()), this, SLOT(onLogout()));
    connect(navSystem,SIGNAL(clicked()),this,SLOT(showSystemManage()));
}

Seller::~Seller()
{
}

void Seller::createNavigation()
{
    // 导航栏主布局
    navLayout = new QVBoxLayout(navWidget);
    navLayout->setContentsMargins(0, 20, 0, 20);
    navLayout->setSpacing(10);
    navLayout->addWidget(navSystem);

    // 用户信息区域
    QWidget *userWidget = new QWidget;
    userWidget->setObjectName("userWidget");
    QVBoxLayout *userLayout = new QVBoxLayout(userWidget);
    userLayout->setContentsMargins(10, 10, 10, 20);

    QLabel *avatarLabel = new QLabel("👤");
    avatarLabel->setFixedSize(60, 60);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAlignment(Qt::AlignCenter);

    userInfoLabel = new QLabel("未登录");
    userInfoLabel->setObjectName("userInfoLabel");
    userInfoLabel->setAlignment(Qt::AlignCenter);

    userLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
    userLayout->addWidget(userInfoLabel);
    navLayout->addWidget(userWidget);

    // 导航按钮
    navDashboard = createNavButton("仪表盘");
    navBooks = createNavButton("图书管理");
    navOrders = createNavButton("订单管理");
    navMembers = createNavButton("会员管理");
    navReports = createNavButton("报表分析");
    navSystem= createNavButton("系统管理");

    navLayout->addWidget(navDashboard);
    navLayout->addWidget(navBooks);
    navLayout->addWidget(navOrders);
    navLayout->addWidget(navMembers);
    navLayout->addWidget(navReports);
    navLayout->addWidget(navSystem);

    // 弹簧
    navLayout->addStretch();

    // 退出按钮
    navLogout = createNavButton("退出登录");
    navLogout->setObjectName("logoutButton");
    navLayout->addWidget(navLogout);
}

QPushButton* Seller::createNavButton(const QString &text)
{
    QPushButton *button = new QPushButton(text);
    button->setObjectName("navButton");
    button->setFixedHeight(40);
    button->setCheckable(true);
    return button;
}

void Seller::applyStyle()
{
    // 设置窗口背景
    setStyleSheet(QString(
        "#navWidget { background-color: %1; border-right: 1px solid #dddddd; }"
        "#navButton { background-color: transparent; border: none; color: white; text-align: left; padding-left: 20px; font-size: 14px; }"
        "#navButton:hover { background-color: rgba(255,255,255,0.1); }"
        "#navButton:checked { background-color: rgba(255,255,255,0.2); }"
        "#logoutButton { background-color: rgba(231,76,60,0.8); margin: 0 10px; border-radius: 4px; }"
        "#logoutButton:hover { background-color: rgba(231,76,60,1); }"
        "#userWidget { border-bottom: 1px solid rgba(255,255,255,0.2); }"
        "#userInfoLabel { color: white; font-size: 12px; }"
        "#avatarLabel { border-radius: 30px; background-color: rgba(255,255,255,0.2); color: white; font-size: 24px; }"
    ).arg(PRIMARY_COLOR.name()));
}

void Seller::showLogin()
{
    contentStack->setCurrentWidget(loginPage);
    navWidget->hide();
    isLoggedIn = false;
}

void Seller::showDashboard()
{
    contentStack->setCurrentWidget(dashboardPage);
    updateNavButtons(navDashboard);
}

void Seller::showBookManage()
{
    contentStack->setCurrentWidget(bookManagePage);
    updateNavButtons(navBooks);
}

void Seller::showOrderManage()
{
    contentStack->setCurrentWidget(orderManagePage);
    updateNavButtons(navOrders);
}

void Seller::showMemberManage()
{
    contentStack->setCurrentWidget(memberPage);
    updateNavButtons(navMembers);
}

void Seller::showReport()
{
    contentStack->setCurrentWidget(reportPage);
    updateNavButtons(navReports);
}

void Seller::onLoginSuccess()
{
    isLoggedIn = true;
    currentUsername = "商家用户";
    userInfoLabel->setText(currentUsername + "\n在线");
    navWidget->show();
    showDashboard();
}

void Seller::onLogout()
{
    if (QMessageBox::question(this, "确认退出", "确定要退出登录吗？") == QMessageBox::Yes) {
        isLoggedIn = false;
        currentUsername.clear();
        userInfoLabel->setText("未登录");
        navWidget->hide();
        showLogin();
    }
}

void Seller::showSystemManage()
{
    contentStack->setCurrentWidget(systemPage);
    updateNavButtons(navSystem);
}

void Seller::updateNavButtons(QPushButton *activeButton)
{
    // 重置所有按钮状态
    QList<QPushButton*> buttons = {navDashboard, navBooks, navOrders, navMembers, navReports};
    foreach (QPushButton *btn, buttons) {
        btn->setChecked(false);
    }
    // 设置当前活动按钮
    if (activeButton) {
        activeButton->setChecked(true);
    }
}
