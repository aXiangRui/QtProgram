#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QDateTime>

// 图书结构体
struct Book
{
    QString bookId;
    QString title;
    QString author;
    QString publisher;
    QDateTime publishDate;
    QString category;
    QString subCategory;
    double price;
    double originalPrice;
    int stock;
    int sales;
    double score;
    QString coverImage;
    QString description;
};

// 分类结构体
struct Category
{
    QString categoryId;
    QString name;
    QString parentId;

    bool operator==(const Category &other) const
    {
        return categoryId == other.categoryId;
    }
};

// 购物车项结构体
struct CartItem
{
    int cartItemId;
    int userId;
    QString bookId;
    QString bookName;
    QString author;
    double price;
    int quantity;
};

// 订单项结构体
struct OrderItem
{
    QString orderItemId;
    QString orderId;
    QString bookId;
    QString bookName;
    QString author;
    double price;
    int quantity;
};

// 订单结构体
struct Order
{
    QString orderId;
    int userId;
    double totalAmount;
    double actualAmount;
    QString couponCode;
    QString status;
    QDateTime createTime;
    QDateTime payTime;
    QDateTime shipTime;
    QDateTime receiveTime;
    QList<OrderItem> items;
};

// 用户结构体
struct User
{
    int userId;
    QString username;
    QString password;
    QString email;
    QString phone;
    int membershipLevel;
    QList<Book> favorites;
};

// 购物车结构体
struct ShoppingCart
{
    int cartId;
    int userId;
    QList<CartItem> items;
    double totalPrice;
};

#endif // BOOK_H
