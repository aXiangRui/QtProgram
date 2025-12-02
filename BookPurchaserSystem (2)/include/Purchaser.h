#ifndef PURCHASER_H
#define PURCHASER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QDateTime>
#include <QObject>
#include <QJsonObject>

// 图书信息
struct Book {
    QString bookId;
    QString title;
    QString author;
    QString category;
    QString subCategory;
    double price;
    double originalPrice;
    int stock;
    int sales;
    double score;
    QString description;
    QString coverImage;
    QList<QString> detailImages;
    QList<QString> tags;
};

// 购物车项
struct CartItem {
    QString bookId;
    int quantity;
    double price;
};

// 购物车
struct ShoppingCart {
    QString cartId;
    QList<CartItem> items;
    double totalPrice;
};

// 订单项
struct OrderItem {
    QString bookId;
    QString title;
    int quantity;
    double price;
};

// 订单信息
struct Order {
    QString orderId;
    QString userId;
    QDateTime createTime;
    QDateTime payTime;
    QDateTime deliveryTime;
    QDateTime receiveTime;
    QList<OrderItem> items;
    double totalAmount;
    double actualAmount;
    QString couponCode;
    QString status; // pending, paid, delivered, completed, cancelled
    QString paymentMethod;
    QString address;
    QString phone;
    QString receiver;
};

// 分类信息
struct Category {
    QString categoryId;
    QString name;
    QString parentId;
    QList<Category> subCategories;
};

// 为Category结构体添加operator==运算符重载
inline bool operator==(const Category& lhs, const Category& rhs) {
    return lhs.categoryId == rhs.categoryId && 
           lhs.name == rhs.name && 
           lhs.parentId == rhs.parentId;
}

// 用户信息
struct User {
    int userId;
    QString username;
    QString password;
    QString email;
    QString phone;
    int membershipLevel;
    QList<Order> orders;
    ShoppingCart cart;
    QList<Book> favorites;
    QMap<QString, double> preferences; // 图书类别偏好及权重
};

// 前置声明
class NetworkController;
class BookManager;
class UserManager;
class CartManager;
class OrderManager;
class CategoryManager;

class Purchaser : public QObject
{
    Q_OBJECT
public:
    Purchaser(QObject *parent = nullptr);
    ~Purchaser();

    // 图书浏览相关接口
    QList<Book> PopularRecommend(int quantity);
    QList<Book> GetBooksByCategory(const QString& categoryId1, const QString& categoryId2);
    QList<Book> SearchBooks(const QString& keyword);
    Book ViewBookDetail(const QString& bookId);

    // 购买相关接口
    bool AddToCart(const QString& bookId, int quantity);
    Order CheckoutByBook(const QString& bookId, int quantity, const QString& couponCode, int membershipLevel);
    Order CheckoutBycart(const QString& cartId, const QString& couponCode, int membershipLevel);
    Order ViewOrder(const QString& orderId);
    bool RemoveFromCart(const QString& bookId);

    // 用户管理相关接口
    bool Login(const QString& username, const QString& password);
    bool Regis(const QString& username, const QString& password);
    bool ChangeImformation(const QString& field, const QString& value);
    int Level(int userId);
    QList<Order> ViewMyOrder(int userId);
    ShoppingCart ViewShoppingCart(int userId);

    // 收藏相关接口
    bool AddToLove(const QString& bookId);

    // 客户服务相关接口
    void SaleChat();
    void ProductFeedback(const QString& feedback);

    // 获取当前用户
    User getCurrentUser() const { return m_user; }

private:
    // 辅助方法
    Category* findCategory(Category* parent, const QString& categoryId);
    void applyDiscount(Order& order, const QString& couponCode, int membershipLevel);
    QString generateOrderId();

    // 成员变量
    User m_user;
    NetworkController* m_networkController;
    BookManager* m_bookManager;
    UserManager* m_userManager;
    CartManager* m_cartManager;
    OrderManager* m_orderManager;
    CategoryManager* m_categoryManager;
};

#endif // PURCHASER_H
