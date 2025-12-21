#ifndef BOOKMERCHANT_H
#define BOOKMERCHANT_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTimer>
#include <QListWidget>
#include <QDateTime>
#include <QPainter>
#include <QVector>
#include "apiservice.h"

// 自定义折线图组件
// 这是一个自定义的Qt组件类，用于绘制销售趋势折线图
class SalesChartWidget : public QWidget
{
    Q_OBJECT
    // Qt元对象系统宏，必须放在类定义的第一个位置
    // 作用：启用Qt的信号槽机制、属性系统、反射等元对象功能
    
public:
    explicit SalesChartWidget(QWidget *parent = nullptr);
    
    void setSalesData(const QVector<double> &sales, const QVector<QString> &dates);
    // 参数1：const QVector<double> &sales：Qt的向量容器，存储double类型的销售额数据
    // 参数2：const QVector<QString> &dates：存储QString类型的日期标签
    // 作用：接收销售额数据和对应的日期标签，更新图表显示
    
protected:
    void paintEvent(QPaintEvent *event) override;
    // 重写的绘制事件处理函数
    
private:
    
    QVector<double> salesData;//存储销售额数据
    QVector<QString> dateLabels;//保存X轴上显示的日期标签，与salesData一一对应
    double maxValue;//Y轴的最大值,由calculateScale()方法根据数据自动计算
    double minValue;//Y轴的最小值,存储Y轴刻度的最小值，通常为0（销售额不能为负）

    void calculateScale();// 计算Y轴刻度范围,使用成员变量salesData进行计算
    // 作用：根据salesData中的最大值和最小值，智能计算合适的Y轴范围
    //    - 自动选择合适的分度值（10, 20, 50, 100等）
    //    - 计算maxValue和minValue，确保图表显示清晰
    // 调用时机：在setSalesData()中调用，数据更新时重新计算
};

class BookMerchant : public QMainWindow
{
    Q_OBJECT

public:
    BookMerchant(QWidget *parent = nullptr);
    ~BookMerchant();

private slots:
    // 登录相关
    void onLoginClicked();
    void onLogoutClicked();

    // 图书管理
    void onRefreshBooksClicked();
    void onAddBookClicked();
    void onEditBookClicked();
    void onDeleteBookClicked();
    void onBookTableCellClicked(int row, int column);

    // 订单管理
    void onRefreshOrdersClicked();
    void onUpdateOrderStatusClicked();
    void onDeleteOrderClicked();
    void onOrderTableCellClicked(int row, int column);

    // 会员管理
    void onEditMemberClicked();
    void onDeleteMemberClicked();
    void onMemberTableCellClicked(int row, int column);

    // 统计报表
    void onRefreshStatsClicked();
    void onGenerateSalesReportClicked();
    void onGenerateInventoryReportClicked();
    void onGenerateMemberReportClicked();

    // 页面切换
    void showLoginPage();
    void showMainPage();
    void showBooksPage();
    void showOrdersPage();
    void showMembersPage();
    void showStatsPage();
    void showProfilePage();
    void showChatPage();
    void showReviewsPage();  // 显示评论管理页面
    
    // 个人中心相关
    void onProfileClicked();
    void onRefreshProfileClicked();
    void onUpdateProfileClicked();
    void onLevelInfoClicked();
    void onSubmitAppealClicked();
    void onRefreshAppealClicked();

private:
    // UI初始化
    void initUI();
    void initConnections();
    void applyStyle();
    
    // 主界面组件创建
    QWidget* createStatCard(const QString &icon, const QString &title, const QString &value, const QString &subtitle, const QString &color, QLabel **valueLabelPtr = nullptr);
    QWidget* createChartWidget(const QString &title, const QString &subtitle);
    QWidget* createOrderStatsWidget();
    QWidget* createStatusItem(const QString &label, const QString &value, const QString &color, QLabel **valueLabelPtr = nullptr);
    void updateDashboardData();  // 更新仪表板数据
    void updateOrderStatusStats();  // 更新订单状态统计
    void updateSalesChart();  // 更新销量趋势图

    // 数据加载
    void loadBooks();
    void loadOrders(bool showEmptyMessage = false, bool updateDashboard = true);  // 加载订单，showEmptyMessage: 是否显示空订单提示，updateDashboard: 是否更新仪表板
    void loadMembers();
    void loadStats();
    void loadReviews();  // 加载评论数据

    // UI组件
    QStackedWidget *stackedWidget;

    // 登录页面
    QWidget *loginPage;
    QLineEdit *loginUsername;
    QLineEdit *loginPassword;
    QPushButton *loginButton;
    QLabel *loginStatusLabel;

    // 主页面（导航）
    QWidget *mainPage;
    QPushButton *mainPageRefreshButton;  // 主页面刷新按钮
    QPushButton *booksButton;
    QPushButton *ordersButton;
    QPushButton *membersButton;
    QPushButton *statsButton;
    QPushButton *buyerChatButton;  // 客户消息按钮
    QPushButton *reviewsButton;  // 评论管理按钮
    QPushButton *profileButton;  // 个人中心按钮
    QPushButton *logoutButton;
    QLabel *welcomeLabel;
    
    // 主界面数据卡片（用于更新数据）
    QLabel *orderValueLabel;
    QLabel *salesValueLabel;
    QLabel *revenueValueLabel;
    QLabel *booksValueLabel;
    
    // 订单统计标签（用于更新订单状态统计）
    QLabel *pendingOrdersLabel;
    QLabel *shippedOrdersLabel;
    QLabel *completedOrdersLabel;
    QLabel *cancelledOrdersLabel;
    
    // 销量趋势图组件
    SalesChartWidget *salesChartWidget;
    
    // 图书管理页面
    QWidget *booksPage;
    QTableWidget *booksTable;
    QPushButton *refreshBooksBtn;
    QPushButton *addBookBtn;
    QPushButton *editBookBtn;
    QPushButton *deleteBookBtn;
    QPushButton *backFromBooksBtn;
    
    // 图书编辑对话框控件
    QLineEdit *bookIdEdit;
    QLineEdit *bookTitleEdit;
    QLineEdit *bookAuthorEdit;
    QLineEdit *bookCategoryEdit;
    QLineEdit *bookSubCategoryEdit;
    QDoubleSpinBox *bookPriceEdit;
    QSpinBox *bookStockEdit;
    QTextEdit *bookDescriptionEdit;

    // 订单管理页面
    QWidget *ordersPage;
    QTableWidget *ordersTable;
    QPushButton *refreshOrdersBtn;
    QPushButton *updateOrderStatusBtn;
    QPushButton *deleteOrderBtn;
    QPushButton *backFromOrdersBtn;
    QComboBox *orderStatusCombo;

    // 会员管理页面
    QWidget *membersPage;
    QTableWidget *membersTable;
    QPushButton *editMemberBtn;
    QPushButton *deleteMemberBtn;
    QPushButton *backFromMembersBtn;

    // 统计报表页面
    QWidget *statsPage;
    QLabel *totalSalesLabel;
    QLabel *totalOrdersLabel;
    QLabel *totalMembersLabel;
    QLabel *totalBooksLabel;
    QTextEdit *reportDisplay;
    QPushButton *refreshStatsBtn;
    QPushButton *generateSalesReportBtn;
    QPushButton *generateInventoryReportBtn;
    QPushButton *generateMemberReportBtn;
    QPushButton *backFromStatsBtn;
    QDateEdit *reportStartDate;
    QDateEdit *reportEndDate;
    
    // 个人中心页面
    QWidget *profilePage;
    QWidget *profileBanner;  // 顶部Banner区域
    QLabel *profileWelcomeLabel;  // 个人中心欢迎语标签
    QWidget *memberCard;  // 会员卡片
    QLabel *memberCardLabel;  // 会员卡片标签
    QLabel *memberCardRechargeLabel;  // 会员卡片累计充值标签
    QPushButton *levelInfoBtn;  // 查看会员等级规则按钮
    QLineEdit *profileSellerNameEdit;  // 商家名称输入框
    QLineEdit *profileEmailEdit;  // 邮箱输入框
    QLineEdit *profilePhoneEdit;  // 电话输入框
    QLineEdit *profileAddressEdit;  // 地址输入框
    QLabel *profileBalanceLabel;  // 账户余额标签
    QLabel *profilePointsLabel;  // 积分标签
    QLabel *profileStatusLabel;  // 账户状态标签
    QPushButton *refreshProfileBtn;
    QPushButton *backFromProfileBtn;
    QPushButton *updateProfileBtn;  // 更新信息按钮
    
    // 申诉相关
    QTextEdit *appealReasonEdit;
    QLabel *appealStatusLabel;
    QLabel *appealReviewCommentLabel;
    QPushButton *submitAppealBtn;
    QPushButton *refreshAppealBtn;
    
    // 客服聊天相关（与管理员聊天）
    QPushButton *chatButton;  // 客服聊天按钮
    QWidget *chatPage;
    QTextEdit *chatDisplay;
    QTextEdit *chatInput;
    QPushButton *sendChatBtn;
    QPushButton *backFromChatBtn;
    QTimer *chatRefreshTimer;  // 客服聊天刷新定时器
    QDateTime lastChatMessageTime;  // 最后一条客服聊天消息的时间
    
    // 客户消息相关（与买家聊天）
    QWidget *buyerChatPage;  // 客户消息页面
    QListWidget *buyerListWidget;  // 买家列表
    QTextEdit *buyerChatDisplay;  // 买家聊天显示区域
    QTextEdit *buyerChatInput;  // 买家聊天输入区域
    QPushButton *sendBuyerChatBtn;  // 发送给买家按钮
    QPushButton *backFromBuyerChatBtn;  // 返回按钮
    QLabel *currentBuyerLabel;  // 当前选中的买家标签
    QTimer *buyerChatRefreshTimer;  // 买家聊天刷新定时器
    int currentChatBuyerId;  // 当前聊天的买家ID
    QDateTime lastBuyerChatMessageTime;  // 最后一条买家聊天消息的时间
    
    // 评论管理页面
    QWidget *reviewsPage;
    QTableWidget *reviewsTable;  // 评论表格
    QPushButton *refreshReviewsBtn;  // 刷新评论按钮
    QPushButton *backFromReviewsBtn;  // 返回按钮
    
    // showChatPage() 已在第65行声明，删除此重复声明
    void showBuyerChatPage();  // 显示客户消息页面
    void loadChatHistory();  // 加载客服聊天历史
    void onSendChatClicked();  // 发送客服消息
    void loadBuyerChatHistory();  // 加载买家聊天历史
    void onSendBuyerChatClicked();  // 发送买家消息
    void onBuyerListItemClicked(QListWidgetItem *item);  // 选择买家
    void loadBuyerList();  // 加载买家列表

    // 数据
    ApiService *apiService;
    QString currentSellerId;
    QString currentSellerName;
    int selectedBookRow;
    int selectedOrderRow;
    int selectedMemberRow;
    bool isLoggedIn;
    
    // 服务器配置
    QString serverIp;
    int serverPort;
    
    // 仪表板刷新定时器
    QTimer *dashboardRefreshTimer;
};

#endif // BOOKMERCHANT_H

