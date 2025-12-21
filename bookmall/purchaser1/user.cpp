#include "user.h"
#include <QDebug>

User::User() : userId(0), membershipLevel(1), memberLevel("普通会员"), 
               totalRecharge(0.0), memberDiscount(1.0), points(0), coupon30(0), coupon50(0), balance(0.0) {}

User::User(const QString &username, const QString &password)
    : userId(0), username(username), password(password),
      membershipLevel(1), memberLevel("普通会员"), 
      totalRecharge(0.0), memberDiscount(1.0), points(0), coupon30(0), coupon50(0), balance(0.0) {}

User::User(const User &other)
    : userId(other.userId), username(other.username), password(other.password),
      phone(other.phone), email(other.email), address(other.address),
      membershipLevel(other.membershipLevel), memberLevel(other.memberLevel),
      totalRecharge(other.totalRecharge), memberDiscount(other.memberDiscount),
      points(other.points), coupon30(other.coupon30), coupon50(other.coupon50), balance(other.balance),
      favoriteBooks(other.favoriteBooks), preferences(other.preferences),
      cartItems(other.cartItems) {}

bool User::addToCart(const QString &bookId, int quantity, const QString &title, double price)
{
    // 检查是否已存在
    for (auto &item : cartItems) {
        if (item.bookId == bookId) {
            item.quantity += quantity;
            return true;
        }
    }

    // 添加新项
    CartItem newItem;
    newItem.bookId = bookId;
    newItem.bookTitle = title;
    newItem.quantity = quantity;
    newItem.price = price;

    cartItems.append(newItem);
    return true;
}

bool User::removeFromCart(const QString &bookId)
{
    for (int i = 0; i < cartItems.size(); i++) {
        if (cartItems[i].bookId == bookId) {
            cartItems.removeAt(i);
            return true;
        }
    }
    return false;
}

void User::clearCart()
{
    cartItems.clear();
}

double User::getCartTotal() const
{
    double total = 0.0;
    for (const auto &item : cartItems) {
        total += item.getTotal();
    }
    return total;
}

bool User::addToFavorite(const QString &bookId)
{
    if (!favoriteBooks.contains(bookId)) {
        favoriteBooks.append(bookId);
        return true;
    }
    return false;
}

bool User::removeFromFavorite(const QString &bookId)
{
    return favoriteBooks.removeAll(bookId) > 0;
}

bool User::isFavorite(const QString &bookId) const
{
    return favoriteBooks.contains(bookId);
}

void User::addPreference(const QString &category, double weight)
{
    UserPreference pref;
    pref.category = category;
    pref.weight = weight;
    preferences.append(pref);
}

void User::updatePreference(const QString &category, double weight)
{
    for (auto &pref : preferences) {
        if (pref.category == category) {
            pref.weight = weight;
            return;
        }
    }
    addPreference(category, weight);
}

bool User::deductBalance(double amount)
{
    if (balance >= amount) {
        balance -= amount;
        return true;
    }
    return false;
}

void User::addBalance(double amount)
{
    balance += amount;
}

Order::Order() : userId(0), totalAmount(0.0), status("待付款") {}

Order::Order(const QString &id, int userId, const QDate &date)
    : orderId(id), userId(userId), orderDate(date), totalAmount(0.0), status("待付款") {}

void Order::addItem(const OrderItem &item)
{
    items.append(item);
    totalAmount += item.getTotal();
}
// 用户管理器实现
UserManager::UserManager() : nextUserId(100)
{
    // 添加测试用户：admin/123456
    User adminUser("admin", "123456");
    adminUser.setId(1);
    adminUser.setPhone("13800138000");
    adminUser.setEmail("admin@example.com");
    adminUser.setAddress("北京市海淀区");
    adminUser.setMembershipLevel(3);
    adminUser.setBalance(5000.0);
    adminUser.addPreference("tech", 0.9);
    adminUser.addPreference("fiction", 0.7);
    users.append(adminUser);
    
    // 添加测试用户：test/123456
    User testUser("test", "123456");
    testUser.setId(2);
    testUser.setPhone("13900139000");
    testUser.setEmail("test@example.com");
    testUser.setAddress("上海市浦东新区");
    testUser.setMembershipLevel(2);
    testUser.setBalance(1000.0);
    testUser.addPreference("tech", 0.8);
    testUser.addPreference("fiction", 0.6);
    users.append(testUser);
    
    qDebug() << "用户管理器初始化完成，已添加" << users.size() << "个测试用户";
}

bool UserManager::registerUser(const QString &username, const QString &password)
{
    // 检查用户名是否已存在
    if (userExists(username)) {
        return false;
    }

    // 创建新用户
    User newUser(username, password);
    newUser.setId(nextUserId++);
    newUser.setMembershipLevel(1); // 默认会员等级
    newUser.setBalance(100.0);     // 默认余额

    users.append(newUser);
    return true;
}

User* UserManager::login(const QString &username, const QString &password)
{
    for (auto &user : users) {
        if (user.getUsername() == username && user.getPassword() == password) {
            return &user;
        }
    }
    return nullptr;
}

User* UserManager::getUserById(int userId)
{
    for (auto &user : users) {
        if (user.getId() == userId) {
            return &user;
        }
    }
    return nullptr;
}

User* UserManager::getUserByUsername(const QString &username)
{
    for (auto &user : users) {
        if (user.getUsername() == username) {
            return &user;
        }
    }
    return nullptr;
}

bool UserManager::userExists(const QString &username) const
{
    for (const auto &user : users) {
        if (user.getUsername() == username) {
            return true;
        }
    }
    return false;
}
