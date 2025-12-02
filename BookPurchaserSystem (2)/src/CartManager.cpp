#include "../include/CartManager.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CartManager::CartManager(QObject *parent)
    : QObject(parent), m_isDataLoaded(false)
{
    // 加载购物车数据
    loadCarts();
}

CartManager::~CartManager()
{
}

ShoppingCart CartManager::getCart(int userId)
{
    if (!m_isDataLoaded) {
        loadCarts();
    }
    
    if (m_carts.contains(userId)) {
        return m_carts[userId];
    }
    
    // 创建空购物车
    ShoppingCart cart;
    cart.cartId = QString("CART%1").arg(userId);
    cart.totalPrice = 0.0;
    m_carts[userId] = cart;
    
    return cart;
}

bool CartManager::addItem(int userId, const QString& bookId, int quantity)
{
    if (!m_isDataLoaded) {
        loadCarts();
    }
    
    // 获取或创建购物车
    ShoppingCart cart = getCart(userId);
    
    // 检查是否已存在该商品
    bool itemExists = false;
    for (int i = 0; i < cart.items.size(); ++i) {
        if (cart.items[i].bookId == bookId) {
            // 更新数量
            cart.items[i].quantity += quantity;
            itemExists = true;
            break;
        }
    }
    
    // 如果不存在，添加新商品
    if (!itemExists) {
        CartItem newItem;
        newItem.bookId = bookId;
        newItem.quantity = quantity;
        // 这里应该从BookManager获取价格，但为了简化，我们使用模拟价格
        newItem.price = 20.0 + (qHash(bookId) % 50) * 1.5;
        cart.items.append(newItem);
    }
    
    // 重新计算总价
    cart.totalPrice = calculateTotal(userId);
    
    // 更新购物车
    m_carts[userId] = cart;
    
    // 更新缓存
    cacheCarts();
    
    return true;
}

bool CartManager::removeItem(int userId, const QString& bookId)
{
    if (!m_isDataLoaded) {
        loadCarts();
    }
    
    if (!m_carts.contains(userId)) {
        return false;
    }
    
    ShoppingCart cart = m_carts[userId];
    
    // 查找并移除商品
    bool removed = false;
    for (int i = 0; i < cart.items.size(); ++i) {
        if (cart.items[i].bookId == bookId) {
            cart.items.removeAt(i);
            removed = true;
            break;
        }
    }
    
    if (removed) {
        // 重新计算总价
        cart.totalPrice = calculateTotal(userId);
        
        // 更新购物车
        m_carts[userId] = cart;
        
        // 更新缓存
        cacheCarts();
    }
    
    return removed;
}

bool CartManager::updateItemQuantity(int userId, const QString& bookId, int quantity)
{
    if (!m_isDataLoaded) {
        loadCarts();
    }
    
    if (!m_carts.contains(userId)) {
        return false;
    }
    
    ShoppingCart cart = m_carts[userId];
    
    // 查找并更新商品数量
    bool updated = false;
    for (int i = 0; i < cart.items.size(); ++i) {
        if (cart.items[i].bookId == bookId) {
            cart.items[i].quantity = quantity;
            updated = true;
            break;
        }
    }
    
    if (updated) {
        // 重新计算总价
        cart.totalPrice = calculateTotal(userId);
        
        // 更新购物车
        m_carts[userId] = cart;
        
        // 更新缓存
        cacheCarts();
    }
    
    return updated;
}

bool CartManager::clearCart(int userId)
{
    if (!m_isDataLoaded) {
        loadCarts();
    }
    
    if (!m_carts.contains(userId)) {
        return false;
    }
    
    // 创建空购物车
    ShoppingCart cart;
    cart.cartId = QString("CART%1").arg(userId);
    cart.totalPrice = 0.0;
    
    // 更新购物车
    m_carts[userId] = cart;
    
    // 更新缓存
    cacheCarts();
    
    return true;
}

double CartManager::calculateTotal(int userId)
{
    if (!m_carts.contains(userId)) {
        return 0.0;
    }
    
    ShoppingCart cart = m_carts[userId];
    double total = 0.0;
    
    for (const CartItem& item : cart.items) {
        total += item.price * item.quantity;
    }
    
    return total;
}

void CartManager::loadCarts()
{
    // 尝试从缓存加载
    loadCartsFromCache();
    
    // 如果缓存不存在或加载失败，生成模拟数据
    if (!m_isDataLoaded) {
        generateMockCarts();
    }
}

void CartManager::loadCartsFromCache()
{
    QFile file(":/cache/carts.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return;
    }
    
    QJsonObject rootObj = doc.object();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
        int userId = it.key().toInt();
        QJsonObject cartObj = it.value().toObject();
        
        ShoppingCart cart;
        cart.cartId = cartObj["cartId"].toString();
        cart.totalPrice = cartObj["totalPrice"].toDouble();
        
        // 加载购物车项
        QJsonArray itemsArray = cartObj["items"].toArray();
        for (const QJsonValue& value : itemsArray) {
            QJsonObject itemObj = value.toObject();
            CartItem item;
            item.bookId = itemObj["bookId"].toString();
            item.quantity = itemObj["quantity"].toInt();
            item.price = itemObj["price"].toDouble();
            cart.items.append(item);
        }
        
        m_carts[userId] = cart;
    }
    
    m_isDataLoaded = true;
}

void CartManager::loadCartsFromServer()
{
    // 这里应该从服务器加载购物车数据
    // 由于是模拟环境，我们使用模拟数据
    generateMockCarts();
}

void CartManager::cacheCarts()
{
    // 将购物车数据缓存到本地
    QJsonObject rootObj;
    
    for (auto it = m_carts.begin(); it != m_carts.end(); ++it) {
        int userId = it.key();
        const ShoppingCart& cart = it.value();
        
        QJsonObject cartObj;
        cartObj["cartId"] = cart.cartId;
        cartObj["totalPrice"] = cart.totalPrice;
        
        // 保存购物车项
        QJsonArray itemsArray;
        for (const CartItem& item : cart.items) {
            QJsonObject itemObj;
            itemObj["bookId"] = item.bookId;
            itemObj["quantity"] = item.quantity;
            itemObj["price"] = item.price;
            itemsArray.append(itemObj);
        }
        cartObj["items"] = itemsArray;
        
        rootObj[QString::number(userId)] = cartObj;
    }
    
    QJsonDocument doc(rootObj);
    QFile file(":/cache/carts.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void CartManager::generateMockCarts()
{
    // 为前5个用户生成模拟购物车数据
    for (int userId = 1; userId <= 5; ++userId) {
        ShoppingCart cart;
        cart.cartId = QString("CART%1").arg(userId);
        
        // 为每个用户添加1-3个商品
        int itemCount = 1 + (userId % 3);
        for (int i = 1; i <= itemCount; ++i) {
            CartItem item;
            item.bookId = QString("B%1").arg((userId * 10 + i), 6, 10, QChar('0'));
            item.quantity = 1 + (i % 2);
            item.price = 20.0 + (i * userId) * 1.5;
            cart.items.append(item);
        }
        
        // 计算总价
        cart.totalPrice = calculateTotal(userId);
        
        m_carts[userId] = cart;
    }
    
    m_isDataLoaded = true;
    
    // 缓存模拟数据
    cacheCarts();
}