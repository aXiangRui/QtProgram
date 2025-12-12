#ifndef USER_H
#define USER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QDate>
#include "book.h"

// 用户偏好
struct UserPreference
{
    QString category;
    double weight;  // 偏好权重
};

// 购物车项
struct CartItem
{
    QString bookId;
    QString bookTitle;
    int quantity;
    double price;

    double getTotal() const { return price * quantity; }
};

// 订单项
struct OrderItem
{
    QString bookId;
    QString bookTitle;
    int quantity;
    double price;
    QString status;  // 订单状态

    double getTotal() const { return price * quantity; }
};

// 订单类
class Order
{
public:
    Order();
    Order(const QString &id, int userId, const QDate &date);

    QString getOrderId() const { return orderId; }
    int getUserId() const { return userId; }
    QDate getOrderDate() const { return orderDate; }
    double getTotalAmount() const { return totalAmount; }
    QString getStatus() const { return status; }
    QList<OrderItem> getItems() const { return items; }

    void setTotalAmount(double amount) { totalAmount = amount; }
    void setStatus(const QString &s) { status = s; }
    void addItem(const OrderItem &item);

private:
    QString orderId;
    int userId;
    QDate orderDate;
    double totalAmount;
    QString status;
    QList<OrderItem> items;
};

// 用户类
class User
{
public:
    User();
    User(const QString &username, const QString &password);

    // Getter方法
    int getId() const { return userId; }
    QString getUsername() const { return username; }
    QString getPassword() const { return password; }
    QString getPhone() const { return phone; }
    QString getEmail() const { return email; }
    QString getAddress() const { return address; }
    int getMembershipLevel() const { return membershipLevel; }
    double getBalance() const { return balance; }
    QList<QString> getFavoriteBooks() const { return favoriteBooks; }
    QList<UserPreference> getPreferences() const { return preferences; }
    QList<CartItem> getCartItems() const { return cartItems; }

    // Setter方法
    void setId(int id) { userId = id; }
    void setPhone(const QString &p) { phone = p; }
    void setEmail(const QString &e) { email = e; }
    void setAddress(const QString &a) { address = a; }
    void setMembershipLevel(int level) { membershipLevel = level; }
    void setBalance(double b) { balance = b; }

    // 购物车操作
    bool addToCart(const QString &bookId, int quantity, const QString &title, double price);
    bool removeFromCart(const QString &bookId);
    void clearCart();
    double getCartTotal() const;

    // 收藏操作
    bool addToFavorite(const QString &bookId);
    bool removeFromFavorite(const QString &bookId);
    bool isFavorite(const QString &bookId) const;

    // 偏好操作
    void addPreference(const QString &category, double weight);
    void updatePreference(const QString &category, double weight);

    // 余额操作
    bool deductBalance(double amount);
    void addBalance(double amount);

private:
    int userId;
    QString username;
    QString password;
    QString phone;
    QString email;
    QString address;
    int membershipLevel;
    double balance;
    QList<QString> favoriteBooks;  // 收藏的图书ID列表
    QList<UserPreference> preferences;  // 用户偏好
    QList<CartItem> cartItems;  // 购物车项
};
// 用户管理器类
class UserManager
{
public:
    UserManager();

    bool registerUser(const QString &username, const QString &password);
    User* login(const QString &username, const QString &password);
    User* getUserById(int userId);
    User* getUserByUsername(const QString &username);
    bool userExists(const QString &username) const;

private:
    QVector<User> users;
    int nextUserId;
};

#endif // USER_H
