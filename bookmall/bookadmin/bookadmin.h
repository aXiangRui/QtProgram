#ifndef BOOKADMIN_H
#define BOOKADMIN_H

#include <QMainWindow>
#include <QString>

// 前向声明，避免循环依赖和头文件包含问题
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class QTableWidget;
class QTextEdit;
class QListWidget;
class QTimer;
class QWidget;
class ApiService;

class BookAdmin : public QMainWindow
{
    Q_OBJECT

public:
    BookAdmin(QWidget *parent = nullptr);
    ~BookAdmin();

private slots:
    // 登录相关
    void onLoginClicked();
    void onLogoutClicked();

    // 用户管理
    void onRefreshUsersClicked();
    void onBanUserClicked();      // 封禁用户
    void onUnbanUserClicked();    // 解封用户
    void onUserTableCellClicked(int row, int column);
    
    // 审核相关
    void onApproveCertificationClicked();
    void onRejectCertificationClicked();
    void onBackFromReviewBtnClicked();

    // 商家管理
    void onRefreshSellersClicked();
    void onBanSellerClicked();      // 封禁商家
    void onUnbanSellerClicked();    // 解封商家
    void onSellerTableCellClicked(int row, int column);
    void onRefreshPendingSellersClicked();  // 刷新待审核商家认证申请
    void onReviewSellerClicked();  // 查看待审核商家认证详情
    void loadPendingSellers();  // 加载待审核商家认证申请列表

    // 图书管理
    void onRefreshBooksClicked();
    void onDeleteBookClicked();
    void onEditBookClicked();
    void onBookTableCellClicked(int row, int column);
    void onRefreshPendingBooksClicked();  // 刷新待审核书籍
    void onApproveBookClicked();  // 审核通过书籍
    void onRejectBookClicked();  // 审核拒绝书籍
    void loadPendingBooks();  // 加载待审核书籍列表

    // 订单管理
    void onRefreshOrdersClicked();
    void onViewOrderDetailsClicked();
    void onDeleteOrderClicked();
    void onOrderTableCellClicked(int row, int column);

    // 统计和系统
    void onRefreshStatsClicked();

    // 页面切换
    void showLoginPage();
    void showDashboardPage();
    void showUsersPage();
    void showBuyersPage();  // 买家管理页面（只显示买家）
    void showSellersPage();
    void showReviewPendingSellersPage();  // 审核管理页面（显示待审核商家）
    void showBooksPage();
    void showOrdersPage();
    void showStatsPage();
    void showReviewPage(int userId);
    void showAppealsPage();
    void showChatPage();
    
    // 申诉审核相关
    void onRefreshAppealsClicked();
    void onApproveAppealClicked();
    void onRejectAppealClicked();
    void onAppealTableCellClicked(int row, int column);

private:
    // UI初始化
    void initUI();
    void initConnections();
    void applyStyle();

    // 数据加载
    void loadUsers();
    void loadBuyers();  // 加载买家（只显示role=1的用户）
    void loadSellers();
    void loadBooks();
    void loadOrders();
    void loadStats();

    // UI组件
    QStackedWidget *stackedWidget;

    // 登录页面
    QWidget *loginPage;
    QLineEdit *loginUsername;
    QLineEdit *loginPassword;
    QPushButton *loginButton;
    QLabel *loginStatusLabel;

    // 仪表盘页面
    QWidget *dashboardPage;
    QPushButton *dashboardRefreshButton;  // 仪表盘刷新按钮
    QLabel *welcomeLabel;
    QPushButton *usersButton;
    QPushButton *buyersButton;  // 买家管理按钮
    QPushButton *sellersButton;
    QPushButton *reviewPendingSellersButton;  // 审核管理按钮（待审核商家）
    QPushButton *booksButton;
    QPushButton *ordersButton;
    QPushButton *statsButton;
    QPushButton *appealsButton;  // 申诉管理按钮
    QPushButton *logoutButton;
    QLabel *totalUsersLabel;
    QLabel *totalBuyersLabel;      // 总买家数
    QLabel *totalSellersLabel;
    QLabel *pendingSellersLabel;   // 待审核商家数
    QLabel *totalBooksLabel;
    QLabel *totalOrdersLabel;
    QLabel *totalAppealsLabel;     // 总申诉数

    // 用户管理页面
    QWidget *usersPage;
    QLabel *usersTitleLabel;  // 用户管理页面标题（用于切换显示"用户管理"或"买家管理"）
    QTableWidget *usersTable;
    QPushButton *refreshUsersBtn;
    QPushButton *banUserBtn;      // 封禁用户按钮
    QPushButton *unbanUserBtn;    // 解封用户按钮
    QPushButton *backFromUsersBtn;
    
    // 审核页面
    QWidget *reviewPage;
    QLabel *reviewUserIdLabel;
    QLabel *reviewUsernameLabel;
    QLabel *reviewEmailLabel;
    QLabel *reviewApplyTimeLabel;
    QLabel *reviewLicenseImageLabel;
    QPushButton *approveBtn;
    QPushButton *rejectBtn;
    QPushButton *backFromReviewBtn;
    int currentReviewUserId;

    // 商家管理页面
    QWidget *sellersPage;
    QTabWidget *sellersTabWidget;  // 商家管理标签页控件（用于切换标签）
    QTableWidget *sellersTable;
    QPushButton *refreshSellersBtn;
    QPushButton *banSellerBtn;      // 封禁商家按钮
    QPushButton *unbanSellerBtn;    // 解封商家按钮
    QPushButton *backFromSellersBtn;
    QTableWidget *pendingSellersTable;  // 待审核商家认证申请表格
    QPushButton *refreshPendingSellersBtn;  // 刷新待审核商家认证申请按钮
    QPushButton *reviewSellerBtn;  // 查看待审核商家认证详情按钮
    int selectedPendingSellerRow;  // 选中的待审核商家认证申请行

    // 图书管理页面
    QWidget *booksPage;
    QTableWidget *booksTable;
    QPushButton *refreshBooksBtn;
    QPushButton *editBookBtn;
    QPushButton *deleteBookBtn;
    QPushButton *backFromBooksBtn;
    QTableWidget *pendingBooksTable;  // 待审核书籍表格
    QPushButton *refreshPendingBooksBtn;  // 刷新待审核书籍按钮
    QPushButton *approveBookBtn;  // 审核通过按钮
    QPushButton *rejectBookBtn;  // 审核拒绝按钮

    // 订单管理页面
    QWidget *ordersPage;
    QTableWidget *ordersTable;
    QPushButton *refreshOrdersBtn;
    QPushButton *viewOrderDetailsBtn;
    QPushButton *deleteOrderBtn;
    QPushButton *backFromOrdersBtn;

    // 统计页面
    QWidget *statsPage;
    QTextEdit *statsDisplay;
    QPushButton *refreshStatsBtn;
    QPushButton *backFromStatsBtn;
    
    // 申诉管理页面
    QWidget *appealsPage;
    QTableWidget *appealsTable;
    QPushButton *refreshAppealsBtn;
    QPushButton *approveAppealBtn;
    QPushButton *rejectAppealBtn;
    QPushButton *backFromAppealsBtn;
    QTextEdit *appealReviewCommentEdit;
    int selectedAppealRow;
    
    // 聊天管理页面
    QPushButton *chatButton;  // 聊天管理按钮
    QWidget *chatPage;
    QListWidget *chatUserList;  // 用户列表（买家/卖家）
    QTextEdit *chatDisplay;
    QTextEdit *chatInput;
    QPushButton *sendChatBtn;
    QPushButton *refreshChatBtn;
    QPushButton *backFromChatBtn;
    int currentChatUserId;
    QString currentChatUserType;
    QTimer *chatRefreshTimer;  // 聊天刷新定时器
    QTimer *dashboardRefreshTimer;  // 仪表盘自动刷新定时器
    // showChatPage() 已在第69行声明，删除此重复声明
    void loadChatUsers();
    void onChatUserSelected();
    void loadChatHistory();
    void onSendChatClicked();

    // 数据
    ApiService *apiService;
    QString currentAdminId;
    QString currentAdminName;
    int selectedUserRow;
    int selectedSellerRow;
    int selectedBookRow;
    int selectedOrderRow;
    bool isLoggedIn;
    
    // 服务器配置
    QString serverIp;
    int serverPort;
};

#endif // BOOKADMIN_H

