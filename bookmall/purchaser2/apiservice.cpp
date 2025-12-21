#include "apiservice.h"
#include <QDebug>

ApiService::ApiService(QObject *parent)
    : QObject(parent), serverHost("localhost"), serverPort(8888)
{
    tcpClient = new TcpClient(this);
    connect(tcpClient, &TcpClient::connected, this, &ApiService::connected);
    connect(tcpClient, &TcpClient::disconnected, this, &ApiService::disconnected);
    connect(tcpClient, &TcpClient::errorOccurred, this, &ApiService::errorOccurred);
}

ApiService::~ApiService()
{
    disconnectFromServer();
}

bool ApiService::connectToServer(const QString &host, quint16 port)
{
    serverHost = host;
    serverPort = port;
    return tcpClient->connectToServer(host, port);
}

void ApiService::disconnectFromServer()
{
    tcpClient->disconnectFromServer();
}

bool ApiService::isConnected() const
{
    return tcpClient->isConnected();
}

// 用户相关API
QJsonObject ApiService::login(const QString &username, const QString &password)
{
    QJsonObject request;
    request["action"] = "login";
    request["username"] = username;
    request["password"] = password;
    return tcpClient->sendRequest(request, 10000);  // 增加超时时间到10秒
}

QJsonObject ApiService::changePassword(const QString &userId, const QString &oldPassword, const QString &newPassword)
{
    QJsonObject request;
    request["action"] = "changePassword";
    request["userId"] = userId;
    request["oldPassword"] = oldPassword;
    request["newPassword"] = newPassword;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateUserInfo(const QString &userId, const QString &phone, const QString &email, const QString &address)
{
    QJsonObject request;
    request["action"] = "updateUserInfo";
    request["userId"] = userId;
    request["phone"] = phone;
    request["email"] = email;
    request["address"] = address;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::registerUser(const QString &username, const QString &password, const QString &email)
{
    QJsonObject request;
    request["action"] = "register";
    request["username"] = username;
    request["password"] = password;
    if (!email.isEmpty()) {
        request["email"] = email;
    }
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::rechargeBalance(const QString &userId, double amount)
{
    QJsonObject request;
    request["action"] = "rechargeBalance";
    request["userId"] = userId;
    request["amount"] = amount;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::participateLottery(const QString &userId)
{
    QJsonObject request;
    request["action"] = "participateLottery";
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::applySellerCertification(const QString &userId, const QString &username, 
                                                  const QString &password, const QString &email, 
                                                  const QString &licenseImageBase64)
{
    QJsonObject request;
    request["action"] = "applySellerCertification";
    request["userId"] = userId;
    request["username"] = username;
    request["password"] = password;
    request["email"] = email;
    request["licenseImage"] = licenseImageBase64;
    return tcpClient->sendRequest(request, 30000);  // 图片上传需要更长的超时时间
}

QJsonObject ApiService::getSellerCertStatus(const QString &userId)
{
    QJsonObject request;
    request["action"] = "getSellerCertStatus";
    request["userId"] = userId;
    return tcpClient->sendRequest(request);
}

// 图书相关API
QJsonObject ApiService::getAllBooks()
{
    QJsonObject request;
    request["action"] = "getAllBooks";
    return tcpClient->sendRequest(request, 10000);  // 增加超时时间到10秒
}

QJsonObject ApiService::getBook(const QString &bookId)
{
    QJsonObject request;
    request["action"] = "getBook";
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::searchBooks(const QString &keyword)
{
    QJsonObject request;
    request["action"] = "searchBooks";
    request["keyword"] = keyword;
    return tcpClient->sendRequest(request, 10000);  // 增加超时时间到10秒
}

// 购物车相关API
QJsonObject ApiService::addToCart(const QString &userId, const QString &bookId, int quantity)
{
    QJsonObject request;
    request["action"] = "addToCart";
    request["userId"] = userId;
    request["bookId"] = bookId;
    request["quantity"] = quantity;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::getCart(const QString &userId)
{
    QJsonObject request;
    request["action"] = "getCart";
    request["userId"] = userId;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::updateCartQuantity(const QString &userId, const QString &bookId, int quantity)
{
    QJsonObject request;
    request["action"] = "updateCartQuantity";
    request["userId"] = userId;
    request["bookId"] = bookId;
    request["quantity"] = quantity;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::removeFromCart(const QString &userId, const QString &bookId)
{
    QJsonObject request;
    request["action"] = "removeFromCart";
    request["userId"] = userId;
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request);
}

// 收藏相关API
QJsonObject ApiService::addFavorite(const QString &userId, const QString &bookId)
{
    QJsonObject request;
    request["action"] = "addFavorite";
    request["userId"] = userId;
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::removeFavorite(const QString &userId, const QString &bookId)
{
    QJsonObject request;
    request["action"] = "removeFavorite";
    request["userId"] = userId;
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request);
}

// 订单相关API
QJsonObject ApiService::createOrder(const QString &userId,
                                    const QJsonArray &items,
                                    const QString &customer,
                                    const QString &phone,
                                    const QString &address,
                                    const QString &useCoupon)
{
    QJsonObject request;
    request["action"] = "createOrder";
    request["userId"] = userId;
    request["items"] = items;
    request["customer"] = customer;
    request["phone"] = phone;
    request["address"] = address;
    if (!useCoupon.isEmpty()) {
        request["useCoupon"] = useCoupon;
    }
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::getUserOrders(const QString &userId)
{
    QJsonObject request;
    request["action"] = "getUserOrders";
    request["userId"] = userId;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::getOrder(const QString &orderId)
{
    QJsonObject request;
    request["action"] = "getOrder";
    request["orderId"] = orderId;
    return tcpClient->sendRequest(request);
}

QJsonObject ApiService::payOrder(const QString &orderId, const QString &paymentMethod, const QString &useCoupon)
{
    QJsonObject request;
    request["action"] = "payOrder";
    request["orderId"] = orderId;
    request["paymentMethod"] = paymentMethod;
    if (!useCoupon.isEmpty()) {
        request["useCoupon"] = useCoupon;
    }
    return tcpClient->sendRequest(request, 10000);  // 支付操作增加超时时间
}

QJsonObject ApiService::cancelOrder(const QString &orderId, const QString &userId, const QString &reason)
{
    QJsonObject request;
    request["action"] = "cancelOrder";
    request["orderId"] = orderId;
    request["userId"] = userId;
    request["reason"] = reason.isEmpty() ? "用户申请取消" : reason;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::confirmReceiveOrder(const QString &orderId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "confirmReceiveOrder";
    request["orderId"] = orderId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

// 聊天相关API
QJsonObject ApiService::sendChatMessage(const QString &senderId, const QString &senderType, 
                                       const QString &receiverId, const QString &receiverType, 
                                       const QString &message)
{
    QJsonObject request;
    request["action"] = "sendChatMessage";
    request["senderId"] = senderId;
    request["senderType"] = senderType;
    if (!receiverId.isEmpty()) {
        request["receiverId"] = receiverId.toInt();
    }
    if (!receiverType.isEmpty()) {
        request["receiverType"] = receiverType;
    }
    request["message"] = message;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getChatHistory(const QString &userId, const QString &userType, 
                                      const QString &otherUserId, const QString &otherUserType)
{
    QJsonObject request;
    request["action"] = "getChatHistory";
    request["userId"] = userId;
    request["userType"] = userType;
    if (!otherUserId.isEmpty()) {
        request["otherUserId"] = otherUserId.toInt();
    }
    if (!otherUserType.isEmpty()) {
        request["otherUserType"] = otherUserType;
    }
    return tcpClient->sendRequest(request, 10000);
}

// 评论相关API
QJsonObject ApiService::addReview(const QString &userId, const QString &bookId, int rating, const QString &comment)
{
    QJsonObject request;
    request["action"] = "addReview";
    request["userId"] = userId;
    request["bookId"] = bookId;
    request["rating"] = rating;
    request["comment"] = comment;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getBookReviews(const QString &bookId)
{
    QJsonObject request;
    request["action"] = "getBookReviews";
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getBookRatingStats(const QString &bookId)
{
    QJsonObject request;
    request["action"] = "getBookRatingStats";
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request, 10000);
}

