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
    request["userType"] = "seller";  // 商家端登录，明确指定为seller
    return tcpClient->sendRequest(request, 10000);
}

// ===== 商家端专用API =====

// 图书管理API
QJsonObject ApiService::getSellerBooks(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetBooks";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::addBook(const QString &sellerId, const QJsonObject &bookData)
{
    QJsonObject request;
    request["action"] = "sellerAddBook";
    request["sellerId"] = sellerId;
    // 直接将bookData的字段复制到request中
    for (auto it = bookData.begin(); it != bookData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateBook(const QString &sellerId, const QString &bookId, const QJsonObject &bookData)
{
    QJsonObject request;
    request["action"] = "sellerUpdateBook";
    request["sellerId"] = sellerId;
    request["isbn"] = bookId;
    // 直接将bookData的字段复制到request中
    for (auto it = bookData.begin(); it != bookData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteBook(const QString &sellerId, const QString &bookId)
{
    QJsonObject request;
    request["action"] = "sellerDeleteBook";
    request["sellerId"] = sellerId;
    request["isbn"] = bookId;
    return tcpClient->sendRequest(request, 10000);
}

// 订单管理API
QJsonObject ApiService::getSellerOrders(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetOrders";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::createOrder(const QString &sellerId, const QJsonObject &orderData)
{
    QJsonObject request;
    request["action"] = "sellerCreateOrder";
    request["sellerId"] = sellerId;
    for (auto it = orderData.begin(); it != orderData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateOrderStatus(const QString &sellerId, const QString &orderId, const QString &status)
{
    QJsonObject request;
    request["action"] = "sellerUpdateOrderStatus";
    request["sellerId"] = sellerId;
    request["orderId"] = orderId;
    request["status"] = status;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteOrder(const QString &sellerId, const QString &orderId)
{
    QJsonObject request;
    request["action"] = "sellerDeleteOrder";
    request["sellerId"] = sellerId;
    request["orderId"] = orderId;
    return tcpClient->sendRequest(request, 10000);
}

// 会员管理API
QJsonObject ApiService::getMembers(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetMembers";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::addMember(const QString &sellerId, const QJsonObject &memberData)
{
    QJsonObject request;
    request["action"] = "sellerAddMember";
    request["sellerId"] = sellerId;
    for (auto it = memberData.begin(); it != memberData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateMember(const QString &sellerId, const QString &memberId, const QJsonObject &memberData)
{
    QJsonObject request;
    request["action"] = "sellerUpdateMember";
    request["sellerId"] = sellerId;
    request["userId"] = memberId;  // 使用userId而不是cardNo
    for (auto it = memberData.begin(); it != memberData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteMember(const QString &sellerId, const QString &memberId)
{
    QJsonObject request;
    request["action"] = "sellerDeleteMember";
    request["sellerId"] = sellerId;
    request["userId"] = memberId;  // 使用userId而不是cardNo
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::rechargeMember(const QString &sellerId, const QString &memberId, double amount)
{
    QJsonObject request;
    request["action"] = "sellerRechargeMember";
    request["sellerId"] = sellerId;
    request["cardNo"] = memberId;
    request["amount"] = amount;
    return tcpClient->sendRequest(request, 10000);
}

// 统计报表API
QJsonObject ApiService::getDashboardStats(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerDashboardStats";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getSalesReport(const QString &sellerId, const QString &startDate, const QString &endDate)
{
    QJsonObject request;
    request["action"] = "sellerGetReportSales";
    request["sellerId"] = sellerId;
    request["startDate"] = startDate;
    request["endDate"] = endDate;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getInventoryReport(const QString &sellerId, const QString &startDate, const QString &endDate)
{
    QJsonObject request;
    request["action"] = "sellerGetReportInventory";
    request["sellerId"] = sellerId;
    request["startDate"] = startDate;
    request["endDate"] = endDate;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getMemberReport(const QString &sellerId, const QString &startDate, const QString &endDate)
{
    QJsonObject request;
    request["action"] = "sellerGetReportMember";
    request["sellerId"] = sellerId;
    request["startDate"] = startDate;
    request["endDate"] = endDate;
    return tcpClient->sendRequest(request, 10000);
}

// 系统设置API
QJsonObject ApiService::getSystemSettings(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetSystemSettings";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateSystemSettings(const QString &sellerId, const QJsonObject &settings)
{
    QJsonObject request;
    request["action"] = "sellerUpdateSystemSettings";
    request["sellerId"] = sellerId;
    request["settings"] = settings;
    return tcpClient->sendRequest(request, 10000);
}

// 个人中心和申诉API
QJsonObject ApiService::getSellerProfile(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetProfile";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateSellerProfile(const QJsonObject &sellerData)
{
    QJsonObject request;
    request["action"] = "sellerUpdateProfile";
    request["sellerId"] = sellerData["sellerId"].toString();
    if (sellerData.contains("email")) {
        request["email"] = sellerData["email"];
    }
    if (sellerData.contains("phoneNumber")) {
        request["phoneNumber"] = sellerData["phoneNumber"];
    }
    if (sellerData.contains("address")) {
        request["address"] = sellerData["address"];
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::submitAppeal(const QString &sellerId, const QString &appealReason)
{
    QJsonObject request;
    request["action"] = "sellerSubmitAppeal";
    request["sellerId"] = sellerId;
    request["appealReason"] = appealReason;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getAppeal(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "sellerGetAppeal";
    request["sellerId"] = sellerId;
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

QJsonObject ApiService::getSellerReviews(const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "getSellerReviews";
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

