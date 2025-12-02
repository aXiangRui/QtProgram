#include "../include/OrderManager.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QUuid>

OrderManager::OrderManager(QObject *parent)
    : QObject(parent), m_isDataLoaded(false)
{
    // 加载订单数据
    loadOrders();
}

OrderManager::~OrderManager()
{
}

Order OrderManager::getOrder(const QString& orderId)
{
    if (!m_isDataLoaded) {
        loadOrders();
    }
    
    if (m_orderMap.contains(orderId)) {
        return m_orderMap[orderId];
    }
    
    // 返回空订单对象
    return Order();
}

QList<Order> OrderManager::getUserOrders(int userId)
{
    if (!m_isDataLoaded) {
        loadOrders();
    }
    
    if (m_userOrdersMap.contains(userId)) {
        return m_userOrdersMap[userId];
    }
    
    // 返回空列表
    return QList<Order>();
}

Order OrderManager::createOrder(const QString& userId, const QList<OrderItem>& items, const QString& couponCode, int membershipLevel)
{
    if (!m_isDataLoaded) {
        loadOrders();
    }
    
    // 创建新订单
    Order newOrder;
    newOrder.orderId = generateOrderId();
    newOrder.userId = userId;
    newOrder.createTime = QDateTime::currentDateTime();
    newOrder.status = "pending";
    newOrder.items = items;
    
    // 计算总金额
    double totalAmount = 0.0;
    for (const OrderItem& item : items) {
        totalAmount += item.price * item.quantity;
    }
    newOrder.totalAmount = totalAmount;
    
    // 应用优惠
    newOrder.actualAmount = applyDiscount(totalAmount, couponCode, membershipLevel);
    newOrder.couponCode = couponCode;
    
    // 添加到列表和映射
    m_orders.append(newOrder);
    m_orderMap[newOrder.orderId] = newOrder;
    
    int intUserId = userId.toInt();
    if (!m_userOrdersMap.contains(intUserId)) {
        m_userOrdersMap[intUserId] = QList<Order>();
    }
    m_userOrdersMap[intUserId].append(newOrder);
    
    // 更新缓存
    cacheOrders();
    
    return newOrder;
}

bool OrderManager::updateOrderStatus(const QString& orderId, const QString& status)
{
    if (!m_isDataLoaded) {
        loadOrders();
    }
    
    if (!m_orderMap.contains(orderId)) {
        return false;
    }
    
    Order order = m_orderMap[orderId];
    order.status = status;
    
    // 根据状态更新时间
    if (status == "paid") {
        order.payTime = QDateTime::currentDateTime();
    } else if (status == "delivered") {
        order.deliveryTime = QDateTime::currentDateTime();
    } else if (status == "completed") {
        order.receiveTime = QDateTime::currentDateTime();
    }
    
    // 更新订单
    m_orderMap[orderId] = order;
    
    // 更新用户订单列表中的订单
    int userId = order.userId.toInt();
    if (m_userOrdersMap.contains(userId)) {
        QList<Order>& userOrders = m_userOrdersMap[userId];
        for (int i = 0; i < userOrders.size(); ++i) {
            if (userOrders[i].orderId == orderId) {
                userOrders[i] = order;
                break;
            }
        }
    }
    
    // 更新全局订单列表
    for (int i = 0; i < m_orders.size(); ++i) {
        if (m_orders[i].orderId == orderId) {
            m_orders[i] = order;
            break;
        }
    }
    
    // 更新缓存
    cacheOrders();
    
    return true;
}

bool OrderManager::payOrder(const QString& orderId, const QString& paymentMethod)
{
    if (!m_orderMap.contains(orderId)) {
        return false;
    }
    
    Order order = m_orderMap[orderId];
    
    // 检查订单状态
    if (order.status != "pending") {
        return false;
    }
    
    // 更新订单
    order.status = "paid";
    order.payTime = QDateTime::currentDateTime();
    order.paymentMethod = paymentMethod;
    
    return updateOrderStatus(orderId, "paid");
}

bool OrderManager::cancelOrder(const QString& orderId)
{
    if (!m_orderMap.contains(orderId)) {
        return false;
    }
    
    Order order = m_orderMap[orderId];
    
    // 检查订单状态，只有待支付和已支付未发货的订单可以取消
    if (order.status != "pending" && order.status != "paid") {
        return false;
    }
    
    return updateOrderStatus(orderId, "cancelled");
}

bool OrderManager::confirmReceipt(const QString& orderId)
{
    if (!m_orderMap.contains(orderId)) {
        return false;
    }
    
    Order order = m_orderMap[orderId];
    
    // 检查订单状态，只有已发货的订单可以确认收货
    if (order.status != "delivered") {
        return false;
    }
    
    return updateOrderStatus(orderId, "completed");
}

void OrderManager::loadOrders()
{
    // 尝试从缓存加载
    loadOrdersFromCache();
    
    // 如果缓存不存在或加载失败，生成模拟数据
    if (!m_isDataLoaded) {
        generateMockOrders();
    }
}

void OrderManager::loadOrdersFromCache()
{
    QFile file(":/cache/orders.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return;
    }
    
    QJsonArray ordersArray = doc.array();
    for (const QJsonValue& value : ordersArray) {
        QJsonObject obj = value.toObject();
        Order order;
        order.orderId = obj["orderId"].toString();
        order.userId = obj["userId"].toString();
        order.createTime = QDateTime::fromString(obj["createTime"].toString(), Qt::ISODate);
        if (obj.contains("payTime") && !obj["payTime"].isNull()) {
            order.payTime = QDateTime::fromString(obj["payTime"].toString(), Qt::ISODate);
        }
        if (obj.contains("deliveryTime") && !obj["deliveryTime"].isNull()) {
            order.deliveryTime = QDateTime::fromString(obj["deliveryTime"].toString(), Qt::ISODate);
        }
        if (obj.contains("receiveTime") && !obj["receiveTime"].isNull()) {
            order.receiveTime = QDateTime::fromString(obj["receiveTime"].toString(), Qt::ISODate);
        }
        order.status = obj["status"].toString();
        order.totalAmount = obj["totalAmount"].toDouble();
        order.actualAmount = obj["actualAmount"].toDouble();
        order.couponCode = obj["couponCode"].toString();
        order.paymentMethod = obj["paymentMethod"].toString();
        order.address = obj["address"].toString();
        order.phone = obj["phone"].toString();
        order.receiver = obj["receiver"].toString();
        
        // 加载订单项
        QJsonArray itemsArray = obj["items"].toArray();
        for (const QJsonValue& itemValue : itemsArray) {
            QJsonObject itemObj = itemValue.toObject();
            OrderItem item;
            item.bookId = itemObj["bookId"].toString();
            item.title = itemObj["title"].toString();
            item.quantity = itemObj["quantity"].toInt();
            item.price = itemObj["price"].toDouble();
            order.items.append(item);
        }
        
        m_orders.append(order);
        m_orderMap[order.orderId] = order;
        
        int userId = order.userId.toInt();
        if (!m_userOrdersMap.contains(userId)) {
            m_userOrdersMap[userId] = QList<Order>();
        }
        m_userOrdersMap[userId].append(order);
    }
    
    m_isDataLoaded = true;
}

void OrderManager::loadOrdersFromServer()
{
    // 这里应该从服务器加载订单数据
    // 由于是模拟环境，我们使用模拟数据
    generateMockOrders();
}

void OrderManager::cacheOrders()
{
    // 将订单数据缓存到本地
    QJsonArray ordersArray;
    for (const Order& order : m_orders) {
        QJsonObject obj;
        obj["orderId"] = order.orderId;
        obj["userId"] = order.userId;
        obj["createTime"] = order.createTime.toString(Qt::ISODate);
        if (order.payTime.isValid()) {
            obj["payTime"] = order.payTime.toString(Qt::ISODate);
        }
        if (order.deliveryTime.isValid()) {
            obj["deliveryTime"] = order.deliveryTime.toString(Qt::ISODate);
        }
        if (order.receiveTime.isValid()) {
            obj["receiveTime"] = order.receiveTime.toString(Qt::ISODate);
        }
        obj["status"] = order.status;
        obj["totalAmount"] = order.totalAmount;
        obj["actualAmount"] = order.actualAmount;
        obj["couponCode"] = order.couponCode;
        obj["paymentMethod"] = order.paymentMethod;
        obj["address"] = order.address;
        obj["phone"] = order.phone;
        obj["receiver"] = order.receiver;
        
        // 保存订单项
        QJsonArray itemsArray;
        for (const OrderItem& item : order.items) {
            QJsonObject itemObj;
            itemObj["bookId"] = item.bookId;
            itemObj["title"] = item.title;
            itemObj["quantity"] = item.quantity;
            itemObj["price"] = item.price;
            itemsArray.append(itemObj);
        }
        obj["items"] = itemsArray;
        
        ordersArray.append(obj);
    }
    
    QJsonDocument doc(ordersArray);
    QFile file(":/cache/orders.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void OrderManager::generateMockOrders()
{
    // 为前5个用户生成模拟订单数据
    QStringList statuses = {"pending", "paid", "delivered", "completed", "cancelled"};
    QStringList paymentMethods = {"alipay", "wechat", "creditcard", "banktransfer"};
    
    for (int userId = 1; userId <= 5; ++userId) {
        // 每个用户生成2-5个订单
        int orderCount = 2 + (userId % 4);
        for (int i = 1; i <= orderCount; ++i) {
            Order order;
            order.orderId = generateOrderId();
            order.userId = QString::number(userId);
            
            // 生成订单时间
            QDateTime createTime = QDateTime::currentDateTime().addDays(-(i * 3));
            order.createTime = createTime;
            
            // 根据状态设置其他时间
            QString status = statuses[(userId + i) % statuses.size()];
            order.status = status;
            
            if (status != "pending" && status != "cancelled") {
                order.payTime = createTime.addSecs((1 + (i % 3)) * 3600);
            }
            
            if (status == "delivered" || status == "completed") {
                order.deliveryTime = order.payTime.addSecs((1 + (i % 2)) * 24 * 3600);
            }
            
            if (status == "completed") {
                order.receiveTime = order.deliveryTime.addDays(2 + (i % 3));
            }
            
            // 添加订单项
            int itemCount = 1 + (i % 3);
            double totalAmount = 0.0;
            for (int j = 1; j <= itemCount; ++j) {
                OrderItem item;
                item.bookId = QString("B%1").arg((userId * 10 + j), 6, 10, QChar('0'));
                item.title = QString("图书标题%1").arg((userId * 10 + j));
                item.quantity = 1 + (j % 2);
                item.price = 20.0 + (j * userId) * 1.5;
                order.items.append(item);
                totalAmount += item.price * item.quantity;
            }
            
            order.totalAmount = totalAmount;
            
            // 应用优惠
            order.couponCode = (i % 3 == 0) ? QString("COUPON%1").arg(i) : "";
            order.actualAmount = applyDiscount(totalAmount, order.couponCode, userId % 5 + 1);
            
            // 支付方式
            order.paymentMethod = (status != "pending" && status != "cancelled") ? 
                paymentMethods[(userId + i) % paymentMethods.size()] : "";
            
            // 收货信息
            order.address = QString("收货地址%1").arg(userId);
            order.phone = QString("1380000%1").arg(userId, 4, 10, QChar('0'));
            order.receiver = QString("收件人%1").arg(userId);
            
            // 添加到列表和映射
            m_orders.append(order);
            m_orderMap[order.orderId] = order;
            
            if (!m_userOrdersMap.contains(userId)) {
                m_userOrdersMap[userId] = QList<Order>();
            }
            m_userOrdersMap[userId].append(order);
        }
    }
    
    m_isDataLoaded = true;
    
    // 缓存模拟数据
    cacheOrders();
}

QString OrderManager::generateOrderId()
{
    // 生成唯一订单ID
    QUuid uuid = QUuid::createUuid();
    QString uuidString = uuid.toString().remove("{").remove("}").remove("-");
    return "ORDER" + uuidString.left(16).toUpper();
}

double OrderManager::applyDiscount(double totalAmount, const QString& couponCode, int membershipLevel)
{
    double discount = 0.0;
    
    // 会员折扣
    if (membershipLevel >= 2) {
        discount += totalAmount * 0.05 * (membershipLevel - 1);
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
    discount = qMin(discount, totalAmount * 0.5);
    
    return totalAmount - discount;
}