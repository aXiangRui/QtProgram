#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QJsonObject>
#include "tcpclient.h"

// 统一的API服务类 - 封装所有业务功能请求
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

    // 用户相关API
    QJsonObject login(const QString &username, const QString &password);
    QJsonObject registerUser(const QString &username, const QString &password, const QString &email = "");
    QJsonObject changePassword(const QString &userId, const QString &oldPassword, const QString &newPassword);
    QJsonObject updateUserInfo(const QString &userId, const QString &phone, const QString &email, const QString &address);  // 更新用户信息
    QJsonObject rechargeBalance(const QString &userId, double amount);  // 充值余额
    QJsonObject participateLottery(const QString &userId);  // 参与抽奖
    
    // 卖家认证相关API
    QJsonObject applySellerCertification(const QString &userId, const QString &username, 
                                         const QString &password, const QString &email, 
                                         const QString &licenseImageBase64);
    QJsonObject getSellerCertStatus(const QString &userId);
    
    // 图书相关API
    QJsonObject getAllBooks();
    QJsonObject getBook(const QString &bookId);
    QJsonObject searchBooks(const QString &keyword);
    
    // 购物车相关API
    QJsonObject addToCart(const QString &userId, const QString &bookId, int quantity);
    QJsonObject getCart(const QString &userId);
    QJsonObject updateCartQuantity(const QString &userId, const QString &bookId, int quantity);
    QJsonObject removeFromCart(const QString &userId, const QString &bookId);
    
    // 收藏相关API
    QJsonObject addFavorite(const QString &userId, const QString &bookId);
    QJsonObject removeFavorite(const QString &userId, const QString &bookId);
    
    // 订单相关API
    // 下单时需要传递用户ID、订单项、收货人信息（姓名/电话/地址）
    QJsonObject createOrder(const QString &userId,
                            const QJsonArray &items,
                            const QString &customer,
                            const QString &phone,
                            const QString &address,
                            const QString &useCoupon = "");
    QJsonObject getUserOrders(const QString &userId);
    QJsonObject getOrder(const QString &orderId);
    QJsonObject payOrder(const QString &orderId, const QString &paymentMethod, const QString &useCoupon = "");
    QJsonObject cancelOrder(const QString &orderId, const QString &userId, const QString &reason = "");
    QJsonObject confirmReceiveOrder(const QString &orderId, const QString &userId);  // 确认收货
    
    // 聊天相关API
    QJsonObject sendChatMessage(const QString &senderId, const QString &senderType, 
                                const QString &receiverId, const QString &receiverType, 
                                const QString &message);
    QJsonObject getChatHistory(const QString &userId, const QString &userType, 
                               const QString &otherUserId = "", const QString &otherUserType = "");
    
    // 评论相关API
    QJsonObject addReview(const QString &userId, const QString &bookId, int rating, const QString &comment);
    QJsonObject getBookReviews(const QString &bookId);
    QJsonObject getBookRatingStats(const QString &bookId);

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

