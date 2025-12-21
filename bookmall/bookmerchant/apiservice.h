#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "tcpclient.h"

// 商家端API服务类 - 封装所有商家业务功能请求
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
    TcpClient* getTcpClient() { return tcpClient; }

    // 用户相关API（商家登录）
    QJsonObject login(const QString &username, const QString &password);
    
    // ===== 商家端专用API =====
    
    // 图书管理API
    QJsonObject getSellerBooks(const QString &sellerId);
    QJsonObject addBook(const QString &sellerId, const QJsonObject &bookData);
    QJsonObject updateBook(const QString &sellerId, const QString &bookId, const QJsonObject &bookData);
    QJsonObject deleteBook(const QString &sellerId, const QString &bookId);
    
    // 订单管理API
    QJsonObject getSellerOrders(const QString &sellerId);
    QJsonObject createOrder(const QString &sellerId, const QJsonObject &orderData);
    QJsonObject updateOrderStatus(const QString &sellerId, const QString &orderId, const QString &status);
    QJsonObject deleteOrder(const QString &sellerId, const QString &orderId);
    
    // 会员管理API
    QJsonObject getMembers(const QString &sellerId);
    QJsonObject addMember(const QString &sellerId, const QJsonObject &memberData);
    QJsonObject updateMember(const QString &sellerId, const QString &memberId, const QJsonObject &memberData);
    QJsonObject deleteMember(const QString &sellerId, const QString &memberId);
    QJsonObject rechargeMember(const QString &sellerId, const QString &memberId, double amount);
    
    // 统计报表API
    QJsonObject getDashboardStats(const QString &sellerId);
    QJsonObject getSalesReport(const QString &sellerId, const QString &startDate, const QString &endDate);
    QJsonObject getInventoryReport(const QString &sellerId, const QString &startDate, const QString &endDate);
    QJsonObject getMemberReport(const QString &sellerId, const QString &startDate, const QString &endDate);
    
    // 系统设置API
    QJsonObject getSystemSettings(const QString &sellerId);
    QJsonObject updateSystemSettings(const QString &sellerId, const QJsonObject &settings);
    
    // 个人中心和申诉API
    QJsonObject getSellerProfile(const QString &sellerId);
    QJsonObject updateSellerProfile(const QJsonObject &sellerData);
    QJsonObject submitAppeal(const QString &sellerId, const QString &appealReason);
    QJsonObject getAppeal(const QString &sellerId);
    
    // 聊天相关API
    QJsonObject sendChatMessage(const QString &senderId, const QString &senderType, 
                                const QString &receiverId, const QString &receiverType, 
                                const QString &message);
    QJsonObject getChatHistory(const QString &userId, const QString &userType, 
                               const QString &otherUserId = "", const QString &otherUserType = "");
    
    // 评论相关API
    QJsonObject getBookReviews(const QString &bookId);
    QJsonObject getBookRatingStats(const QString &bookId);
    QJsonObject getSellerReviews(const QString &sellerId);  // 获取该卖家的所有商品评论

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

