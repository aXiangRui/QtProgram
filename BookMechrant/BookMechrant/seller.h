#ifndef SELLER_H
#define SELLER_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

// 前置声明
class LoginWidget;
class DashboardWidget;
class BookManageWidget;
class OrderManageWidget;
class MemberWidget;
class ReportWidget;
class NetworkClient;
class SystemManageWidget;

class Seller : public QWidget
{
    Q_OBJECT

public:
    Seller(QWidget *parent = 0);
    ~Seller();

private slots:
    void showLogin();
    void showDashboard();
    void showBookManage();
    void showOrderManage();
    void showMemberManage();
    void showReport();
    void onLoginSuccess();
    void onLogout();
    void showSystemManage();

private:
    void createNavigation();
    void applyStyle();
    QPushButton* createNavButton(const QString &text);
    void updateNavButtons(QPushButton *activeButton);

    // 导航区域
    QWidget *navWidget;
    QVBoxLayout *navLayout;
    QPushButton *navDashboard;
    QPushButton *navBooks;
    QPushButton *navOrders;
    QPushButton *navMembers;
    QPushButton *navReports;
    QPushButton *navLogout;
    QPushButton *navSystem;
    QLabel *userInfoLabel;

    // 内容区域
    QStackedWidget *contentStack;
    LoginWidget *loginPage;
    DashboardWidget *dashboardPage;
    BookManageWidget *bookManagePage;
    OrderManageWidget *orderManagePage;
    MemberWidget *memberPage;
    ReportWidget *reportPage;
    SystemManageWidget*systemPage;

    // 网络客户端
    NetworkClient *networkClient;

    // 状态
    bool isLoggedIn;
    QString currentUsername;
};

#endif // SELLER_H
