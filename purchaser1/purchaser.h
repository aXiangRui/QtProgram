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
#include "user.h"
#include "book.h"

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

    // 浏览相关
    void onSearchClicked();
    void onCategoryItemClicked(QTreeWidgetItem *item, int column);
    void onBookItemClicked(QListWidgetItem *item);

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

    // 个人信息相关
    void onUpdateProfileClicked();
    void onViewProfileClicked();

    // 客服相关
    void onCustomerServiceClicked();
    void onSendFeedbackClicked();

    // 界面切换
    void showLoginPage();
    void showMainPage();
    void showBookDetailPage();
    void showCartPage();
    void showOrdersPage();
    void showProfilePage();
    void showServicePage();

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
    void loadBooks();
    void loadCategories();
    void updateRecommendations();
    void updateCartDisplay();
    void updateOrderDisplay();
    void updateProfileDisplay();

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
    QLabel *bookTitleLabel;
    QLabel *bookAuthorLabel;
    QLabel *bookPriceLabel;
    QLabel *bookScoreLabel;
    QTextEdit *bookDescription;
    QSpinBox *quantitySpinBox;
    QPushButton *addToCartBtn;
    QPushButton *buyNowBtn;
    QPushButton *addToFavoriteBtn;
    QPushButton *backToMainBtn;

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

    // 个人资料页面
    QWidget *profilePage;
    QLineEdit *profileUsername;
    QLineEdit *profilePhone;
    QLineEdit *profileEmail;
    QLineEdit *profileAddress;
    QLabel *profileLevelLabel;
    QLabel *profileBalanceLabel;
    QListWidget *favoriteList;
    QPushButton *updateProfileBtn;
    QPushButton *backFromProfileBtn;

    // 客服页面
    QWidget *servicePage;
    QTextEdit *chatDisplay;
    QTextEdit *feedbackInput;
    QPushButton *sendFeedbackBtn;
    QPushButton *backFromServiceBtn;

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
};

#endif // PURCHASER_H
