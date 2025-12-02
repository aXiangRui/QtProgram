#ifndef CARTMANAGER_H
#define CARTMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "Purchaser.h"

// 购物车管理器类，负责购物车数据的加载、缓存和更新
class CartManager : public QObject
{
    Q_OBJECT
public:
    CartManager(QObject *parent = nullptr);
    ~CartManager();

    // 获取用户购物车
    ShoppingCart getCart(int userId);
    
    // 添加商品到购物车
    bool addItem(int userId, const QString& bookId, int quantity);
    
    // 从购物车移除商品
    bool removeItem(int userId, const QString& bookId);
    
    // 更新购物车商品数量
    bool updateItemQuantity(int userId, const QString& bookId, int quantity);
    
    // 清空购物车
    bool clearCart(int userId);
    
    // 计算购物车总价
    double calculateTotal(int userId);
    
    // 加载购物车数据
    void loadCarts();
    
    // 缓存购物车数据
    void cacheCarts();

private:
    // 从本地缓存加载购物车数据
    void loadCartsFromCache();
    
    // 从服务器加载购物车数据
    void loadCartsFromServer();
    
    // 模拟购物车数据（用于测试）
    void generateMockCarts();

private:
    QMap<int, ShoppingCart> m_carts; // 用户ID到购物车的映射
    bool m_isDataLoaded;
};

#endif // CARTMANAGER_H