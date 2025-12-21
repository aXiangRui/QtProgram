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
    const QList<OrderItem>& getItems() const { return items; }

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
    User(const User &other);  // 拷贝构造函数

    // Getter方法
    int getId() const { return userId; }
    QString getUsername() const { return username; }
    QString getPassword() const { return password; }
    QString getPhone() const { return phone; }
    QString getEmail() const { return email; }
    QString getAddress() const { return address; }
    int getMembershipLevel() const { return membershipLevel; }
    QString getMemberLevel() const { return memberLevel; }  // 会员等级字符串（普通会员、银卡会员等）
    double getBalance() const { return balance; }
    double getTotalRecharge() const { return totalRecharge; }  // 累计充值总额
    double getMemberDiscount() const { return memberDiscount; }  // 会员折扣率
    int getPoints() const { return points; }  // 积分
    bool canParticipateLottery() const { return points >= 3; }  // 是否可以参与抽奖（累计满3积分）
    int getCoupon30() const { return coupon30; }  // 30元优惠券数量
    int getCoupon50() const { return coupon50; }  // 50元优惠券数量
    QList<QString> getFavoriteBooks() const { return favoriteBooks; }
    QList<UserPreference> getPreferences() const { return preferences; }
    QList<CartItem> getCartItems() const { return cartItems; }

    // Setter方法
    void setId(int id) { userId = id; }
    void setPhone(const QString &p) { phone = p; }
    void setEmail(const QString &e) { email = e; }
    void setAddress(const QString &a) { address = a; }
    void setMembershipLevel(int level) { membershipLevel = level; }
    void setMemberLevel(const QString &level) { memberLevel = level; }  // 设置会员等级字符串
    void setTotalRecharge(double amount) { totalRecharge = amount; }  // 设置累计充值总额
    void setMemberDiscount(double discount) { memberDiscount = discount; }  // 设置会员折扣率
    void setPoints(int p) { points = p; }  // 设置积分
    void setBalance(double b) { balance = b; }
    void setCoupon30(int count) { coupon30 = count; }  // 设置30元优惠券数量
    void setCoupon50(int count) { coupon50 = count; }  // 设置50元优惠券数量

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

public:
    int userId;
    QString username;
    QString password;
    QString phone;
    QString email;
    QString address;
    int membershipLevel;  // 保留向后兼容（1-5）
    QString memberLevel;  // 会员等级字符串（普通会员、银卡会员、金卡会员、铂金会员、钻石会员、黑钻会员）
    double totalRecharge;  // 累计充值总额
    double memberDiscount;  // 会员折扣率（0.75-1.0）
    int points;  // 积分（每充值100元获得1积分）
    int coupon30;  // 30元优惠券数量
    int coupon50;  // 50元优惠券数量
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
