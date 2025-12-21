#ifndef PURCHASER_H
#define PURCHASER_H

#include <QMainWindow>
#include <QListWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QSpinBox>
#include <QTimer>
#include "user.h"
#include "book.h"
#include "apiservice.h"
#include <QComboBox>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class Purchaser; }
QT_END_NAMESPACE

class Purchaser : public QMainWindow
{
    Q_OBJECT

public:
    Purchaser(QWidget *parent = nullptr);
    ~Purchaser();

private slots:
    // 登录注册相关
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onRegisterConfirmed();
    void onConnectClicked();  // 连接按钮（如果存在）

    // 浏览相关
    void onSearchClicked();
    void onCategoryItemClicked(QTreeWidgetItem *item, int column);
    void onBookItemClicked(QListWidgetItem *item);
    void onRefreshClicked();  // 手动刷新图书列表
    void onAutoRefresh();     // 自动刷新图书列表

    // 购物车相关
    void onAddToCartClicked();
    void onRemoveFromCartClicked();
    void onCheckoutClicked();
    void onDirectBuyClicked();
    void onViewCartClicked();

    // 收藏相关
    void onAddToFavoriteClicked();

    // 订单相关
    void onViewOrderClicked();
    void onCancelOrderClicked();
    void onConfirmReceiveClicked();  // 确认收货
    void onOrderTableCellClicked(int row, int column);

    // 支付相关
    void onPaymentClicked();
    void onConfirmPaymentClicked();
    void onCancelPaymentClicked();
    void onRechargeClicked();  // 充值
    void performRecharge(double amount);  // 执行实际充值操作
    void onLevelInfoClicked();  // 显示会员等级说明

    // 个人信息相关
    void onUpdateProfileClicked();
    void onViewProfileClicked();
    void onApplySellerClicked();  // 申请成为卖家
    void onSelectLicenseImageClicked();  // 选择营业执照图片
    void onChangePasswordClicked();  // 修改密码
    void onLotteryClicked();  // 抽奖按钮点击

    // 客服相关
    void onCustomerServiceClicked();
    void onSendFeedbackClicked();
    void loadChatHistory();  // 加载聊天历史
    
    // 卖家聊天相关
    void onContactSellerClicked();  // 联系卖家按钮点击
    void onSendSellerMessageClicked();  // 发送消息给卖家
    void loadSellerChatHistory();  // 加载与卖家的聊天历史
    
    // 评论相关
    void onAddReviewClicked();  // 添加评论按钮点击
    void loadBookReviews();  // 加载商品评论
    void loadBookRatingStats();  // 加载商品评分统计
    
    // 工具函数
    QString truncateBookTitle(const QString &title, int maxLength = 12);  // 截断过长的书名

    // 界面切换
    void showLoginPage();
    void showMainPage();
    void showBookDetailPage();
    void showCartPage();
    void showOrdersPage();
    void showProfilePage();
    void showServicePage();
    void showPaymentPage();

    void onBooksLoaded(const QList<Book> &books);  // 处理服务器返回的图书列表
    void onLoginResult(bool success, const User &user);  // 处理登录结果
    void onOrderResult(bool success, const QString &orderId);  // 处理下单结果
    void onNetworkError(const QString &errMsg);  // 处理网络错误

private:
    // 核心功能实现
    QList<Book> PopularRecommend(int quantity);
    QList<Book> GetBooksByCategory(const QString &categoryId1, const QString &categoryId2);
    QList<Book> SearchBooks(const QString &keyword);
    Book ViewBookDetail(const QString &bookId);
    bool AddToCart(const QString &bookId, int quantity);
    Order CheckoutByBook(const QString &cartId, const QString &couponCode, int membershipLevel);
    Order CheckoutByCart(const QString &cartId, const QString &couponCode, int membershipLevel);
    Order ViewOrder(const QString &orderId);
    User* Login(const QString &username, const QString &password);
    bool Register(const QString &username, const QString &password);
    bool ChangeInformation(const QString &field, const QString &value);
    int LevelUp();
    QList<Order> ViewMyOrder(int userId);
    QList<CartItem> ViewShoppingCart(int userId);
    bool RemoveFromCart(const QString &bookId);
    bool AddToFavorite(const QString &bookId);
    void SaleChat();
    void ProductFeedback(const QString &feedback);

    // 初始化函数
    void initUI();
    void initData();
    void initConnections();
    void loadBooks();  // 从服务器加载图书
    void loadLocalBooks();  // 加载本地预设图书数据
    void loadCategories();
    void updateRecommendations();
    void updateCartDisplay();
    void updateCartTotal();  // 更新购物车总金额（只计算选中的商品）
    void onCartQuantityChanged(int newQuantity);  // 处理购物车数量改变
    void updateOrderDisplay();
    void updateProfileDisplay();
    void updateBooksToCategories();
    CategoryNode* findCategoryNode(CategoryNode *node, const QString &categoryId);
    CategoryNode* findCategoryNodeByName(CategoryNode *node, const QString &categoryName);
    void clearCategoryBooks(CategoryNode *node);
    void applyStyle();  // 应用统一样式

    // UI组件
    QStackedWidget *stackedWidget;

    // 登录页面
    QWidget *loginPage;
    QLineEdit *loginUsername;
    QLineEdit *loginPassword;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QLabel *loginStatusLabel;  // 添加状态标签

    // 注册页面
    QWidget *registerPage;
    QLineEdit *regUsername;
    QLineEdit *regPassword;
    QLineEdit *regConfirmPassword;
    QPushButton *confirmRegisterBtn;
    QPushButton *backToLoginBtn;

    // 主页面
    QWidget *mainPage;
    QListWidget *recommendList;
    QTreeWidget *categoryTree;
    QLineEdit *searchInput;
    QPushButton *searchButton;
    QPushButton *refreshButton;
    QPushButton *cartButton;
    QPushButton *ordersButton;
    QPushButton *profileButton;
    QPushButton *serviceButton;
    QPushButton *logoutButton;

    // 图书详情页面
    QWidget *bookDetailPage;
    QLabel *bookCoverLabel;  // 封面图片标签
    QLabel *bookTitleLabel;
    QLabel *bookAuthorLabel;
    QLabel *bookPriceLabel;
    QLabel *bookScoreLabel;
    QLabel *bookFavoriteCountLabel;
    QTextEdit *bookDescription;
    QSpinBox *quantitySpinBox;
    QPushButton *addToCartBtn;
    QPushButton *buyNowBtn;
    QPushButton *addToFavoriteBtn;
    QPushButton *contactSellerBtn;  // 联系卖家按钮
    QPushButton *addReviewBtn;  // 添加评论按钮
    QPushButton *backToMainBtn;
    QTextEdit *reviewsDisplay;  // 评论显示区域
    QLabel *ratingStatsLabel;  // 评分统计标签

    // 购物车页面
    QWidget *cartPage;
    QTableWidget *cartTable;
    QLabel *cartTotalLabel;
    QPushButton *checkoutBtn;
    QPushButton *removeFromCartBtn;
    QPushButton *backFromCartBtn;

    // 订单页面
    QWidget *ordersPage;
    QTableWidget *ordersTable;
    QPushButton *backFromOrdersBtn;
    QPushButton *refreshOrdersBtn;
    QPushButton *cancelOrderBtn;
    QPushButton *confirmReceiveBtn;  // 确认收货按钮
    int selectedOrderRow;

    // 支付页面
    QWidget *paymentPage;
    QLabel *paymentOrderIdLabel;
    QLabel *paymentAmountLabel;
    QLabel *paymentBalanceLabel;
    QLabel *paymentCoupon30Label;  // 30元优惠券数量标签
    QLabel *paymentCoupon50Label;  // 50元优惠券数量标签
    QComboBox *paymentMethodCombo;
    QComboBox *paymentCouponCombo;  // 优惠券选择下拉框
    QPushButton *confirmPaymentBtn;
    QPushButton *cancelPaymentBtn;
    QTextEdit *paymentOrderItems;
    QString pendingOrderId;  // 待支付的订单ID
    double pendingAmount;    // 待支付金额
    QList<CartItem> pendingCartItems; // 待支付订单的商品快照（用于展示/生成订单）

    // 个人资料页面
    QWidget *profilePage;
    QWidget *profileBanner;  // Banner区域
    QLabel *welcomeLabel;  // 欢迎语标签
    QWidget *memberCard;  // 会员卡片
    QLabel *memberCardLabel;  // 会员卡片文字标签
    QLabel *memberCardRechargeLabel;  // 会员卡片累计充值标签
    QLineEdit *profileUsername;
    QLineEdit *profilePhone;
    QLineEdit *profileEmail;
    QLineEdit *profileAddress;
    QLabel *profileLevelLabel;
    QPushButton *levelInfoBtn;  // 会员等级说明按钮（圆形）
    QLabel *profileBalanceLabel;
    QLabel *profilePointsLabel;  // 积分显示标签
    QPushButton *lotteryBtn;  // 抽奖按钮
    QListWidget *favoriteList;
    QPushButton *updateProfileBtn;
    QPushButton *rechargeBtn;
    QPushButton *backFromProfileBtn;
    
    // 修改密码相关
    QLineEdit *oldPasswordEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *changePasswordBtn;
    
    // 卖家认证相关
    QLabel *sellerStatusLabel;  // 卖家认证状态标签
    QLabel *licenseImageLabel;  // 营业执照图片预览标签
    QPushButton *selectLicenseBtn;  // 选择营业执照按钮
    QPushButton *applySellerBtn;  // 申请成为卖家按钮
    QString licenseImagePath;  // 营业执照图片路径
    QString licenseImageBase64;  // 营业执照图片Base64编码

    // 客服页面
    QWidget *servicePage;
    QTextEdit *chatDisplay;
    QTextEdit *feedbackInput;
    QPushButton *sendFeedbackBtn;
    
    // 卖家聊天对话框
    QDialog *sellerChatDialog;
    QTextEdit *sellerChatDisplay;
    QTextEdit *sellerMessageInput;
    QPushButton *sendSellerMessageBtn;
    int currentSellerId;  // 当前聊天的卖家ID
    QTimer *sellerChatRefreshTimer;  // 卖家聊天刷新定时器
    QDateTime lastMessageTime;  // 最后一条消息的时间，用于增量更新
    QPushButton *backFromServiceBtn;
    QTimer *chatRefreshTimer;  // 聊天刷新定时器

    // 数据
    User* currentUser;  // 改为指针
    UserManager userManager;  // 用户管理器
    QList<Book> allBooks;
    QMap<QString, Book> bookMap;
    CategoryNode *categoryRoot;
    QList<Order> allOrders;
    bool isLoggedIn;

    // 当前查看的图书
    Book currentBook;

    ApiService *apiService;  // API服务（TCP协议）
    QString serverIp;  // 服务器IP（可从配置或UI输入）
    int serverPort;    // 服务器端口（例如：8888）
    
    // 自动刷新定时器
    QTimer *autoRefreshTimer;
    static const int AUTO_REFRESH_INTERVAL = 30000;  // 30秒自动刷新一次
};

#endif // PURCHASER_H
