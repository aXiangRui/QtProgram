#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QMutex>
#include <QDateTime>
#include "threadpool.h"

struct BookInfo {
    QString bookId;      // 图书ID
    QString bookName;    // 图书名称
    QString category;    // 分类
    QString subCategory; // 子分类
    double price;        // 价格
    double score;        // 评分
    int sales;           // 销量
    int stock;           // 库存
};

// TCP文件传输任务：处理单个客户端的大文件传输请求（每个客户端一个任务）
class TcpFileTask : public QObject, public Task
{
    Q_OBJECT
public:
    // 构造函数：接收客户端套接字描述符
    explicit TcpFileTask(qintptr socketDescriptor, QObject *parent = nullptr);
    // 任务执行函数：实现文件接收逻辑
    void run() override;

signals:
    // 新增信号：传递客户端IP、端口和接收的数据
    void dataReceived(const QString& clientIp, quint16 clientPort, const QString& data);
    // 新增信号：传递日志信息
    void logGenerated(const QString& log);

private:
    qintptr m_socketDescriptor;  // 客户端套接字描述符（用于创建通信套接字）
    QString m_clientIp;          // 客户端IP地址
    quint16 m_clientPort;        // 客户端端口
    int m_currentSellerId;       // 当前登录的商家ID（-1表示未登录或非商家）
    QString m_currentUserType;   // 当前用户类型（"buyer"或"seller"）
    QList<BookInfo> getPresetBooks();
    
    // 处理JSON格式的请求
    QJsonObject processJsonRequest(const QJsonObject &request);
    // 发送JSON格式的响应（长度前缀协议）
    void sendJsonResponse(QTcpSocket &socket, const QJsonObject &response);
    // 处理登录请求
    QJsonObject handleLogin(const QJsonObject &request);
    // 处理注册请求
    QJsonObject handleRegister(const QJsonObject &request);
    // 处理修改密码请求
    QJsonObject handleChangePassword(const QJsonObject &request);
    // 处理更新用户信息请求
    QJsonObject handleUpdateUserInfo(const QJsonObject &request);
    // 处理获取图书列表请求
    QJsonObject handleGetAllBooks(const QJsonObject &request);
    // 处理获取图书详情请求
    QJsonObject handleGetBook(const QJsonObject &request);
    // 处理搜索图书请求
    QJsonObject handleSearchBooks(const QJsonObject &request);
    // 处理添加到购物车请求
    QJsonObject handleAddToCart(const QJsonObject &request);
    // 处理获取购物车请求
    QJsonObject handleGetCart(const QJsonObject &request);
    // 处理更新购物车数量请求
    QJsonObject handleUpdateCartQuantity(const QJsonObject &request);
    // 处理从购物车移除书籍请求
    QJsonObject handleRemoveFromCart(const QJsonObject &request);
    // 处理添加到收藏请求
    QJsonObject handleAddFavorite(const QJsonObject &request);
    // 处理从收藏移除请求
    QJsonObject handleRemoveFavorite(const QJsonObject &request);
    // 处理创建订单请求
    QJsonObject handleCreateOrder(const QJsonObject &request);
    // 处理获取订单列表请求
    QJsonObject handleGetUserOrders(const QJsonObject &request);
    // 处理支付订单请求
    QJsonObject handlePayOrder(const QJsonObject &request);
    // 处理充值余额请求
    QJsonObject handleRechargeBalance(const QJsonObject &request);
    // 处理抽奖请求
    QJsonObject handleParticipateLottery(const QJsonObject &request);
    // 处理取消订单请求（买家）
    QJsonObject handleCancelOrder(const QJsonObject &request);
    // 处理确认收货请求（买家）
    QJsonObject handleConfirmReceiveOrder(const QJsonObject &request);
    // 处理发送聊天消息请求
    QJsonObject handleSendChatMessage(const QJsonObject &request);
    // 处理获取聊天历史请求
    QJsonObject handleGetChatHistory(const QJsonObject &request);
    // 处理添加评论请求
    QJsonObject handleAddReview(const QJsonObject &request);
    // 处理获取商品评论请求
    QJsonObject handleGetBookReviews(const QJsonObject &request);
    // 处理获取商品评分统计请求
    QJsonObject handleGetBookRatingStats(const QJsonObject &request);
    // 处理获取卖家所有商品评论请求
    QJsonObject handleGetSellerReviews(const QJsonObject &request);
    // 处理发货请求（卖家）
    QJsonObject handleShipOrder(const QJsonObject &request);
    // 处理卖家认证申请
    QJsonObject handleApplySellerCertification(const QJsonObject &request);
    // 处理查询卖家认证状态
    QJsonObject handleGetSellerCertStatus(const QJsonObject &request);

    // ===== 卖家端（BookMechrant）接口 =====
    QJsonObject handleSellerGetBooks(const QJsonObject &request);
    QJsonObject handleSellerAddBook(const QJsonObject &request);
    QJsonObject handleSellerUpdateBook(const QJsonObject &request);
    QJsonObject handleSellerDeleteBook(const QJsonObject &request);
    QJsonObject handleSellerGetOrders(const QJsonObject &request);
    QJsonObject handleSellerCreateOrder(const QJsonObject &request);
    QJsonObject handleSellerUpdateOrderStatus(const QJsonObject &request);
    QJsonObject handleSellerDeleteOrder(const QJsonObject &request);
    QJsonObject handleSellerGetMembers(const QJsonObject &request);
    QJsonObject handleSellerAddMember(const QJsonObject &request);
    QJsonObject handleSellerUpdateMember(const QJsonObject &request);
    QJsonObject handleSellerDeleteMember(const QJsonObject &request);
    QJsonObject handleSellerRechargeMember(const QJsonObject &request);
    QJsonObject handleSellerDashboardStats(const QJsonObject &request);
    QJsonObject handleSellerGetSystemSettings(const QJsonObject &request);
    QJsonObject handleSellerUpdateSystemSettings(const QJsonObject &request);
    QJsonObject handleSellerGetReportSales(const QJsonObject &request);
    QJsonObject handleSellerGetReportInventory(const QJsonObject &request);
    QJsonObject handleSellerGetReportMember(const QJsonObject &request);
    QJsonObject handleSellerGetProfile(const QJsonObject &request);  // 获取卖家个人信息
    QJsonObject handleSellerSubmitAppeal(const QJsonObject &request);  // 提交申诉
    QJsonObject handleSellerGetAppeal(const QJsonObject &request);  // 查询申诉状态
    
    // ===== 管理员端（BookAdmin）接口 =====
    QJsonObject handleAdminLogin(const QJsonObject &request);
    QJsonObject handleAdminGetAllUsers(const QJsonObject &request);
    QJsonObject handleAdminDeleteUser(const QJsonObject &request);
    QJsonObject handleAdminBanUser(const QJsonObject &request);
    QJsonObject handleAdminGetAllSellers(const QJsonObject &request);
    QJsonObject handleAdminDeleteSeller(const QJsonObject &request);
    QJsonObject handleAdminBanSeller(const QJsonObject &request);
    QJsonObject handleAdminGetAllBooks(const QJsonObject &request);
    QJsonObject handleAdminGetPendingBooks(const QJsonObject &request);  // 获取待审核书籍列表
    QJsonObject handleAdminGetPendingSellerCertifications(const QJsonObject &request);  // 获取待审核商家认证申请列表
    QJsonObject handleAdminApproveBook(const QJsonObject &request);  // 审核通过书籍
    QJsonObject handleAdminRejectBook(const QJsonObject &request);  // 审核拒绝书籍
    QJsonObject handleAdminDeleteBook(const QJsonObject &request);
    QJsonObject handleAdminUpdateBook(const QJsonObject &request);
    QJsonObject handleAdminGetAllOrders(const QJsonObject &request);
    QJsonObject handleAdminDeleteOrder(const QJsonObject &request);
    QJsonObject handleAdminGetSystemStats(const QJsonObject &request);
    QJsonObject handleAdminGetRequestLogs(const QJsonObject &request);
    QJsonObject handleAdminGetSellerCertification(const QJsonObject &request);
    QJsonObject handleAdminApproveSellerCertification(const QJsonObject &request);
    QJsonObject handleAdminRejectSellerCertification(const QJsonObject &request);
    QJsonObject handleAdminGetAllAppeals(const QJsonObject &request);  // 获取所有申诉
    QJsonObject handleAdminReviewAppeal(const QJsonObject &request);  // 审核申诉
};

// TCP服务器类：监听并处理客户端的文件传输连接
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

signals:
    void dataReceived(const QString& clientIp, quint16 clientPort, const QString& data);
    void logGenerated(const QString& log);

protected:
    // 新客户端连接触发：创建任务并提交到线程池
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // TCPSERVER_H
