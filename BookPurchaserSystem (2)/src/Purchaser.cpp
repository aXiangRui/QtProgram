#include "../include/Purchaser.h"
#include "../include/NetworkController.h"
#include "../include/BookManager.h"
#include "../include/UserManager.h"
#include "../include/CartManager.h"
#include "../include/OrderManager.h"
#include "../include/CategoryManager.h"
#include <QDebug>
#include <QDateTime>
#include <QUuid>

Purchaser::Purchaser(QObject *parent)
    : QObject(parent)
{
    // 初始化各个管理器
    m_networkController = new NetworkController(this);
    m_bookManager = new BookManager(this);
    m_userManager = new UserManager(this);
    m_cartManager = new CartManager(this);
    m_orderManager = new OrderManager(this);
    m_categoryManager = new CategoryManager(this);
    
    // 设置默认连接类型为HTTP
    m_networkController->setConnectionType(NetworkConnectionFactory::HTTP);
}

Purchaser::~Purchaser()
{
    // 清理资源
    delete m_networkController;
    delete m_bookManager;
    delete m_userManager;
    delete m_cartManager;
    delete m_orderManager;
    delete m_categoryManager;
}

QList<Book> Purchaser::PopularRecommend(int quantity)
{
    // 获取用户偏好
    QMap<QString, double> preferences = m_user.preferences;
    
    // 获取热门图书
    QList<Book> popularBooks = m_bookManager->getPopularBooks();
    
    // 根据用户偏好和图书热度进行排序
    std::sort(popularBooks.begin(), popularBooks.end(), 
              [&preferences](const Book& a, const Book& b) {
                  double scoreA = a.score * 0.7;
                  double scoreB = b.score * 0.7;
                  
                  // 添加用户偏好权重
                  if (preferences.contains(a.category)) {
                      scoreA += preferences[a.category] * 0.3;
                  }
                  if (preferences.contains(b.category)) {
                      scoreB += preferences[b.category] * 0.3;
                  }
                  
                  return scoreA > scoreB;
              });
    
    // 返回指定数量的图书
    return popularBooks.mid(0, quantity);
}

QList<Book> Purchaser::GetBooksByCategory(const QString& categoryId1, const QString& categoryId2)
{
    // 获取分类树
    Category rootCategory = m_categoryManager->getCategoryTree();
    
    // 查找一级分类
    Category* category1 = findCategory(&rootCategory, categoryId1);
    if (!category1) return QList<Book>();
    
    // 查找二级分类
    Category* category2 = nullptr;
    if (!categoryId2.isEmpty()) {
        category2 = findCategory(category1, categoryId2);
        if (!category2) return QList<Book>();
    }
    
    // 获取该分类下的所有图书
    return m_bookManager->getBooksByCategory(categoryId2.isEmpty() ? categoryId1 : categoryId2);
}

QList<Book> Purchaser::SearchBooks(const QString& keyword)
{
    return m_bookManager->searchBooks(keyword);
}

Book Purchaser::ViewBookDetail(const QString& bookId)
{
    return m_bookManager->getBookById(bookId);
}

bool Purchaser::AddToCart(const QString& bookId, int quantity)
{
    // 检查图书是否存在
    Book book = m_bookManager->getBookById(bookId);
    if (book.bookId.isEmpty()) return false;
    
    // 检查库存
    if (book.stock < quantity) return false;
    
    // 添加到购物车
    return m_cartManager->addItem(m_user.userId, bookId, quantity);
}

Order Purchaser::CheckoutByBook(const QString& bookId, int quantity, const QString& couponCode, int membershipLevel)
{
    // 检查图书是否存在
    Book book = m_bookManager->getBookById(bookId);
    if (book.bookId.isEmpty()) {
        // 返回空订单
        return Order();
    }
    
    // 检查库存
    if (book.stock < quantity) {
        // 返回空订单
        return Order();
    }
    
    // 创建订单项
    OrderItem item;
    item.bookId = book.bookId;
    item.title = book.title;
    item.quantity = quantity;
    item.price = book.price;
    
    QList<OrderItem> items;
    items.append(item);
    
    // 创建订单
    return m_orderManager->createOrder(QString::number(m_user.userId), items, couponCode, membershipLevel);
}

Order Purchaser::CheckoutBycart(const QString& cartId, const QString& couponCode, int membershipLevel)
{
    // 获取购物车
    ShoppingCart cart = m_cartManager->getCart(m_user.userId);
    
    // 检查购物车是否为空
    if (cart.items.isEmpty()) {
        // 返回空订单
        return Order();
    }
    
    // 创建订单项
    QList<OrderItem> items;
    for (const CartItem& cartItem : cart.items) {
        // 获取图书信息
        Book book = m_bookManager->getBookById(cartItem.bookId);
        if (book.bookId.isEmpty()) continue;
        
        // 检查库存
        if (book.stock < cartItem.quantity) continue;
        
        OrderItem item;
        item.bookId = book.bookId;
        item.title = book.title;
        item.quantity = cartItem.quantity;
        item.price = book.price;
        
        items.append(item);
    }
    
    // 检查是否有有效的订单项
    if (items.isEmpty()) {
        // 返回空订单
        return Order();
    }
    
    // 创建订单
    Order order = m_orderManager->createOrder(QString::number(m_user.userId), items, couponCode, membershipLevel);
    
    // 清空购物车
    m_cartManager->clearCart(m_user.userId);
    
    return order;
}

Order Purchaser::ViewOrder(const QString& orderId)
{
    return m_orderManager->getOrder(orderId);
}

bool Purchaser::RemoveFromCart(const QString& bookId)
{
    return m_cartManager->removeItem(m_user.userId, bookId);
}

bool Purchaser::Login(const QString& username, const QString& password)
{
    // 调用用户管理器进行登录验证
    return m_userManager->loginUser(username, password, m_user);
}

bool Purchaser::Regis(const QString& username, const QString& password)
{
    return m_userManager->registerUser(username, password);
}

bool Purchaser::ChangeImformation(const QString& field, const QString& value)
{
    // 更新用户信息
    if (field == "username") {
        m_user.username = value;
    } else if (field == "password") {
        m_user.password = value;
    } else if (field == "email") {
        m_user.email = value;
    } else if (field == "phone") {
        m_user.phone = value;
    } else {
        return false;
    }
    
    // 保存到用户管理器
    return m_userManager->updateUser(m_user);
}

int Purchaser::Level(int userId)
{
    return m_userManager->getMembershipLevel(userId);
}

QList<Order> Purchaser::ViewMyOrder(int userId)
{
    return m_orderManager->getUserOrders(userId);
}

ShoppingCart Purchaser::ViewShoppingCart(int userId)
{
    return m_cartManager->getCart(userId);
}

bool Purchaser::AddToLove(const QString& bookId)
{
    // 检查图书是否存在
    Book book = m_bookManager->getBookById(bookId);
    if (book.bookId.isEmpty()) return false;
    
    // 检查是否已在收藏中
    for (const Book& favBook : m_user.favorites) {
        if (favBook.bookId == bookId) {
            return false;
        }
    }
    
    // 添加到收藏
    m_user.favorites.append(book);
    
    // 更新用户信息
    return m_userManager->updateUser(m_user);
}

void Purchaser::SaleChat()
{
    // 这里应该打开售前售后聊天窗口
    qDebug() << "Open sale chat window";
}

void Purchaser::ProductFeedback(const QString& feedback)
{
    // 这里应该提交商品反馈
    qDebug() << "Submit product feedback:" << feedback;
}

Category* Purchaser::findCategory(Category* parent, const QString& categoryId)
{
    if (parent->categoryId == categoryId)
        return parent;
    
    for (auto& subCategory : parent->subCategories) {
        Category* result = findCategory(&subCategory, categoryId);
        if (result) return result;
    }
    
    return nullptr;
}

void Purchaser::applyDiscount(Order& order, const QString& couponCode, int membershipLevel)
{
    // 这里应该应用优惠计算
    // 由于是模拟环境，我们使用简单的折扣计算
    double discount = 0.0;
    
    // 会员折扣
    if (membershipLevel >= 2) {
        discount += order.totalAmount * 0.05 * (membershipLevel - 1);
    }
    
    // 优惠券折扣
    if (!couponCode.isEmpty()) {
        // 模拟优惠券折扣
        if (couponCode.startsWith("COUPON")) {
            int couponValue = couponCode.mid(6).toInt();
            discount += couponValue * 5.0;
        }
    }
    
    // 确保折扣不超过总金额的50%
    discount = qMin(discount, order.totalAmount * 0.5);
    
    order.actualAmount = order.totalAmount - discount;
}

QString Purchaser::generateOrderId()
{
    // 生成唯一订单ID
    QUuid uuid = QUuid::createUuid();
    QString uuidString = uuid.toString().remove("{").remove("}").remove("-");
    return "ORDER" + uuidString.left(16).toUpper();
}
