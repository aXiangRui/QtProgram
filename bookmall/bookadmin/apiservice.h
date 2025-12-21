#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "tcpclient.h"

// 管理员端API服务类 - 封装所有管理员功能请求
class ApiService : public QObject
{
    Q_OBJECT

public:
    explicit ApiService(QObject *parent = nullptr);
    ~ApiService();

    // 连接服务器
    bool connectToServer(const QString &host = "localhost", quint16 port = 8888);
    void disconnectFromServer();
    bool isConnected() const;

    // 管理员登录
    QJsonObject login(const QString &username, const QString &password);
    
    // ===== 管理员专用API =====
    
    // 用户管理API（管理所有买家）
    QJsonObject getAllUsers(const QString &adminId);
    QJsonObject getUser(const QString &adminId, const QString &userId);
    QJsonObject addUser(const QString &adminId, const QJsonObject &userData);
    QJsonObject updateUser(const QString &adminId, const QString &userId, const QJsonObject &userData);
    QJsonObject deleteUser(const QString &adminId, const QString &userId);
    QJsonObject banUser(const QString &adminId, const QString &userId, bool banned);
    
    // 审核相关API
    QJsonObject getSellerCertification(const QString &adminId, const QString &userId);
    QJsonObject approveSellerCertification(const QString &adminId, const QString &userId);
    QJsonObject rejectSellerCertification(const QString &adminId, const QString &userId);
    
    // 商家管理API（管理所有商家）
    QJsonObject getAllSellers(const QString &adminId);
    QJsonObject getSeller(const QString &adminId, const QString &sellerId);
    QJsonObject addSeller(const QString &adminId, const QJsonObject &sellerData);
    QJsonObject updateSeller(const QString &adminId, const QString &sellerId, const QJsonObject &sellerData);
    QJsonObject deleteSeller(const QString &adminId, const QString &sellerId);
    QJsonObject banSeller(const QString &adminId, const QString &sellerId, bool banned);
    
    // 图书全局管理API
    QJsonObject getAllBooksGlobal(const QString &adminId);
    QJsonObject getPendingBooks(const QString &adminId);  // 获取待审核书籍列表
    QJsonObject getPendingSellerCertifications(const QString &adminId);  // 获取待审核商家认证申请列表
    QJsonObject approveBook(const QString &adminId, const QString &isbn);  // 审核通过书籍
    QJsonObject rejectBook(const QString &adminId, const QString &isbn);  // 审核拒绝书籍
    QJsonObject deleteBookGlobal(const QString &adminId, const QString &bookId);
    QJsonObject updateBookGlobal(const QString &adminId, const QString &bookId, const QJsonObject &bookData);
    
    // 订单全局管理API
    QJsonObject getAllOrdersGlobal(const QString &adminId);
    QJsonObject getOrderDetails(const QString &adminId, const QString &orderId);
    QJsonObject deleteOrderGlobal(const QString &adminId, const QString &orderId);
    
    // 系统统计API
    QJsonObject getSystemStats(const QString &adminId);
    QJsonObject getDashboardData(const QString &adminId);
    QJsonObject getUserStats(const QString &adminId);
    QJsonObject getSellerStats(const QString &adminId);
    QJsonObject getBookStats(const QString &adminId);
    QJsonObject getOrderStats(const QString &adminId);
    
    // 系统日志API
    QJsonObject getSystemLogs(const QString &adminId, const QString &startDate, const QString &endDate);
    QJsonObject getOperationLogs(const QString &adminId, const QString &userId);
    
    // 系统设置API
    QJsonObject getSystemSettings(const QString &adminId);
    QJsonObject updateSystemSettings(const QString &adminId, const QJsonObject &settings);
    
    // 数据分析API
    QJsonObject getSalesAnalysis(const QString &adminId, const QString &startDate, const QString &endDate);
    QJsonObject getUserAnalysis(const QString &adminId);
    QJsonObject getSellerAnalysis(const QString &adminId);
    
    // 申诉管理API
    QJsonObject getAllAppeals(const QString &adminId, const QString &status = "");
    QJsonObject reviewAppeal(const QString &adminId, int appealId, const QString &status, const QString &reviewComment);
    
    // 聊天相关API
    QJsonObject sendChatMessage(const QString &senderId, const QString &senderType, 
                                const QString &receiverId, const QString &receiverType, 
                                const QString &message);
    QJsonObject getChatHistory(const QString &userId, const QString &userType, 
                               const QString &otherUserId = "", const QString &otherUserType = "");

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private:
    TcpClient *tcpClient;
    QString serverHost;
    quint16 serverPort;
};

#endif // APISERVICE_H

