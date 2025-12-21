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

// 管理员登录
QJsonObject ApiService::login(const QString &username, const QString &password)
{
    QJsonObject request;
    request["action"] = "adminLogin";
    request["username"] = username;
    request["password"] = password;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 用户管理API =====

QJsonObject ApiService::getAllUsers(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetAllUsers";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getUser(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminGetUser";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::addUser(const QString &adminId, const QJsonObject &userData)
{
    QJsonObject request;
    request["action"] = "adminAddUser";
    request["adminId"] = adminId;
    for (auto it = userData.begin(); it != userData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateUser(const QString &adminId, const QString &userId, const QJsonObject &userData)
{
    QJsonObject request;
    request["action"] = "adminUpdateUser";
    request["adminId"] = adminId;
    request["userId"] = userId;
    for (auto it = userData.begin(); it != userData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteUser(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminDeleteUser";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::banUser(const QString &adminId, const QString &userId, bool banned)
{
    QJsonObject request;
    request["action"] = "adminBanUser";
    request["adminId"] = adminId;
    request["userId"] = userId;
    request["banned"] = banned;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 审核相关API =====
QJsonObject ApiService::getSellerCertification(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminGetSellerCertification";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::approveSellerCertification(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminApproveSellerCertification";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::rejectSellerCertification(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminRejectSellerCertification";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 商家管理API =====

QJsonObject ApiService::getAllSellers(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetAllSellers";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getSeller(const QString &adminId, const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "adminGetSeller";
    request["adminId"] = adminId;
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::addSeller(const QString &adminId, const QJsonObject &sellerData)
{
    QJsonObject request;
    request["action"] = "adminAddSeller";
    request["adminId"] = adminId;
    for (auto it = sellerData.begin(); it != sellerData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateSeller(const QString &adminId, const QString &sellerId, const QJsonObject &sellerData)
{
    QJsonObject request;
    request["action"] = "adminUpdateSeller";
    request["adminId"] = adminId;
    request["sellerId"] = sellerId;
    for (auto it = sellerData.begin(); it != sellerData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteSeller(const QString &adminId, const QString &sellerId)
{
    QJsonObject request;
    request["action"] = "adminDeleteSeller";
    request["adminId"] = adminId;
    request["sellerId"] = sellerId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::banSeller(const QString &adminId, const QString &sellerId, bool banned)
{
    QJsonObject request;
    request["action"] = "adminBanSeller";
    request["adminId"] = adminId;
    request["sellerId"] = sellerId;
    request["banned"] = banned;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 图书全局管理API =====

QJsonObject ApiService::getAllBooksGlobal(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetAllBooks";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getPendingBooks(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetPendingBooks";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getPendingSellerCertifications(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetPendingSellerCertifications";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::approveBook(const QString &adminId, const QString &isbn)
{
    QJsonObject request;
    request["action"] = "adminApproveBook";
    request["adminId"] = adminId;
    request["isbn"] = isbn;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::rejectBook(const QString &adminId, const QString &isbn)
{
    QJsonObject request;
    request["action"] = "adminRejectBook";
    request["adminId"] = adminId;
    request["isbn"] = isbn;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteBookGlobal(const QString &adminId, const QString &bookId)
{
    QJsonObject request;
    request["action"] = "adminDeleteBook";
    request["adminId"] = adminId;
    request["bookId"] = bookId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateBookGlobal(const QString &adminId, const QString &bookId, const QJsonObject &bookData)
{
    QJsonObject request;
    request["action"] = "adminUpdateBook";
    request["adminId"] = adminId;
    request["bookId"] = bookId;
    for (auto it = bookData.begin(); it != bookData.end(); ++it) {
        request[it.key()] = it.value();
    }
    return tcpClient->sendRequest(request, 10000);
}

// ===== 订单全局管理API =====

QJsonObject ApiService::getAllOrdersGlobal(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetAllOrders";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getOrderDetails(const QString &adminId, const QString &orderId)
{
    QJsonObject request;
    request["action"] = "adminGetOrderDetails";
    request["adminId"] = adminId;
    request["orderId"] = orderId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::deleteOrderGlobal(const QString &adminId, const QString &orderId)
{
    QJsonObject request;
    request["action"] = "adminDeleteOrder";
    request["adminId"] = adminId;
    request["orderId"] = orderId;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 系统统计API =====

QJsonObject ApiService::getSystemStats(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetSystemStats";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getDashboardData(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetDashboard";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getUserStats(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetUserStats";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getSellerStats(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetSellerStats";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getBookStats(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetBookStats";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getOrderStats(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetOrderStats";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 系统日志API =====

QJsonObject ApiService::getSystemLogs(const QString &adminId, const QString &startDate, const QString &endDate)
{
    QJsonObject request;
    request["action"] = "adminGetSystemLogs";
    request["adminId"] = adminId;
    request["startDate"] = startDate;
    request["endDate"] = endDate;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getOperationLogs(const QString &adminId, const QString &userId)
{
    QJsonObject request;
    request["action"] = "adminGetOperationLogs";
    request["adminId"] = adminId;
    request["userId"] = userId;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 系统设置API =====

QJsonObject ApiService::getSystemSettings(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetSystemSettings";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::updateSystemSettings(const QString &adminId, const QJsonObject &settings)
{
    QJsonObject request;
    request["action"] = "adminUpdateSystemSettings";
    request["adminId"] = adminId;
    request["settings"] = settings;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 数据分析API =====

QJsonObject ApiService::getSalesAnalysis(const QString &adminId, const QString &startDate, const QString &endDate)
{
    QJsonObject request;
    request["action"] = "adminGetSalesAnalysis";
    request["adminId"] = adminId;
    request["startDate"] = startDate;
    request["endDate"] = endDate;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getUserAnalysis(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetUserAnalysis";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::getSellerAnalysis(const QString &adminId)
{
    QJsonObject request;
    request["action"] = "adminGetSellerAnalysis";
    request["adminId"] = adminId;
    return tcpClient->sendRequest(request, 10000);
}

// ===== 申诉管理API =====

QJsonObject ApiService::getAllAppeals(const QString &adminId, const QString &status)
{
    QJsonObject request;
    request["action"] = "adminGetAllAppeals";
    request["adminId"] = adminId;
    if (!status.isEmpty()) {
        request["status"] = status;
    }
    return tcpClient->sendRequest(request, 10000);
}

QJsonObject ApiService::reviewAppeal(const QString &adminId, int appealId, const QString &status, const QString &reviewComment)
{
    QJsonObject request;
    request["action"] = "adminReviewAppeal";
    request["adminId"] = adminId;
    request["appealId"] = appealId;
    request["reviewerId"] = adminId.toInt();
    request["status"] = status;
    request["reviewComment"] = reviewComment;
    // 通过审核时需要解封卖家并恢复图书上架，可能需要更长时间，增加超时时间到30秒
    int timeout = (status == "已通过") ? 30000 : 10000;
    return tcpClient->sendRequest(request, timeout);
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

