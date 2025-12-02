#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "Purchaser.h"

// 订单管理器类，负责订单数据的加载、缓存和更新
class OrderManager : public QObject
{
    Q_OBJECT
public:
    OrderManager(QObject *parent = nullptr);
    ~OrderManager();

    // 获取订单
    Order getOrder(const QString& orderId);
    
    // 获取用户的所有订单
    QList<Order> getUserOrders(int userId);
    
    // 创建订单
    Order createOrder(const QString& userId, const QList<OrderItem>& items, const QString& couponCode, int membershipLevel);
    
    // 更新订单状态
    bool updateOrderStatus(const QString& orderId, const QString& status);
    
    // 支付订单
    bool payOrder(const QString& orderId, const QString& paymentMethod);
    
    // 取消订单
    bool cancelOrder(const QString& orderId);
    
    // 确认收货
    bool confirmReceipt(const QString& orderId);
    
    // 加载订单数据
    void loadOrders();
    
    // 缓存订单数据
    void cacheOrders();

private:
    // 从本地缓存加载订单数据
    void loadOrdersFromCache();
    
    // 从服务器加载订单数据
    void loadOrdersFromServer();
    
    // 模拟订单数据（用于测试）
    void generateMockOrders();
    
    // 生成订单ID
    QString generateOrderId();
    
    // 应用优惠
    double applyDiscount(double totalAmount, const QString& couponCode, int membershipLevel);

private:
    QList<Order> m_orders;
    QMap<QString, Order> m_orderMap; // 用于快速查找
    QMap<int, QList<Order>> m_userOrdersMap; // 用户ID到订单列表的映射
    bool m_isDataLoaded;
};

#endif // ORDERMANAGER_H