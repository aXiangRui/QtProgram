#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QMutex>

// --- 数据库管理类（单例模式）---
class Database
{
public:
    // 获取单例实例
    static Database& getInstance();
    
    // 初始化数据库连接
    bool initConnection(const QString& host = "49.232.145.193",
                       int port = 3306,
                       const QString& dbName = "test_db",
                       const QString& username = "root01",
                       const QString& password = "123456");
    
    // 检查是否已连接
    bool isConnected() const;
    
    // 关闭数据库连接
    void closeConnection();
    
    // ===== 请求日志功能 =====
    bool logRequest(const QString& clientIp, quint16 clientPort,
                   const QString& action, const QJsonObject& requestData,
                   const QJsonObject& responseData, bool success, 
                   const QString& category);
    QJsonArray getRequestLogs(int limit = 100, const QString& category = "");
    
    // ===== 用户相关 =====
    bool registerUser(const QString& username, const QString& password, const QString& email);
    QJsonObject loginUser(const QString& username, const QString& password);
    QJsonArray getAllUsers();
    QJsonObject getUserById(int userId);  // 根据用户ID获取单个用户信息
    bool deleteUser(int userId);
    bool updateUserBalance(int userId, double balance);
    bool rechargeUserBalance(int userId, double amount);  // 充值余额（增加余额）
    bool deductUserBalance(int userId, double amount);  // 扣除余额（减少余额）
    bool changePassword(int userId, const QString& oldPassword, const QString& newPassword);
    bool updateUserStatus(int userId, const QString& status);  // 更新用户状态（封禁/解封）
    bool updateUserInfo(int userId, const QString& phone, const QString& email, const QString& address);  // 更新用户信息（电话、邮箱、地址）
    bool updateUserMemberLevel(int userId, const QString& memberLevel);  // 更新用户会员等级
    
    // ===== 会员等级相关 =====
    QString calculateMemberLevel(double totalRecharge);  // 根据累计充值总额计算会员等级
    double getMemberDiscount(const QString& memberLevel);  // 获取会员等级对应的折扣率（0.75-1.0）
    bool updateMemberLevel(int userId);  // 根据累计充值总额更新会员等级（会获取锁）
    bool updateMemberLevelUnlocked(int userId);  // 根据累计充值总额更新会员等级（不获取锁，必须在已持有锁的情况下调用）
    
    // ===== 积分相关 =====
    int calculatePoints(double rechargeAmount);  // 计算充值获得的积分（每100元1积分）
    bool addPoints(int userId, int points);  // 增加用户积分
    int getUserPoints(int userId);  // 获取用户积分
    bool canParticipateLottery(int userId);  // 检查用户是否可以参与抽奖（累计满3积分）
    
    // ===== 优惠券相关 =====
    bool addCoupon30(int userId, int count = 1);  // 增加30元优惠券（存入user_coupons表）
    bool addCoupon50(int userId, int count = 1);  // 增加50元优惠券（存入user_coupons表）
    bool useCoupon30(int userId, int count = 1, const QString& orderId = "");  // 使用30元优惠券（更新user_coupons表状态）
    bool useCoupon50(int userId, int count = 1, const QString& orderId = "");  // 使用50元优惠券（更新user_coupons表状态）
    bool rollbackCoupon30(int userId, const QString& orderId);  // 回滚30元优惠券使用
    bool rollbackCoupon50(int userId, const QString& orderId);  // 回滚50元优惠券使用
    int getCoupon30Count(int userId);  // 获取30元优惠券数量（从user_coupons表查询）
    int getCoupon50Count(int userId);  // 获取50元优惠券数量（从user_coupons表查询）
    
    // ===== 抽奖和奖品相关 =====
    bool addUserCoupon(int userId, double couponValue);  // 添加用户优惠券到user_coupons表
    QJsonArray getUserCoupons(int userId);  // 获取用户的优惠券列表
    
    // ===== 商家相关 =====
    bool registerSeller(const QString& sellerName, const QString& password, const QString& email);
    QJsonObject loginSeller(const QString& sellerName, const QString& password);
    QJsonArray getAllSellers();
    QJsonObject getSellerById(int sellerId);  // 根据商家ID获取单个商家信息
    bool deleteSeller(int sellerId);
    bool updateSellerStatus(int sellerId, const QString& status);  // 更新商家状态（封禁/解封）
    bool updateBooksStatusBySellerId(int sellerId, const QString& status);  // 批量更新商家的图书状态
    bool rechargeSellerBalance(int sellerId, double amount);  // 充值商家余额（增加余额，同时更新积分和会员等级）
    bool updateSellerMemberLevel(int sellerId);  // 根据累计充值总额更新商家会员等级（不获取锁）
    bool updateSellerMemberLevelUnlocked(int sellerId);  // 根据累计充值总额更新商家会员等级（不获取锁，必须在已持有锁的情况下调用）
    
    // ===== 卖家认证相关 =====
    bool applySellerCertification(int userId, const QString& username, const QString& password, 
                                  const QString& email, const QString& licenseImageBase64);
    QJsonObject getSellerCertification(int userId);
    QJsonArray getPendingSellerCertifications();  // 获取待审核的商家认证申请列表
    bool approveSellerCertification(int userId);  // 审核通过，将用户添加到卖家表
    bool rejectSellerCertification(int userId);  // 审核拒绝，将用户role改回1（买家）
    
    // ===== 图书相关 =====
    bool addBook(const QJsonObject& book);
    bool updateBook(const QString& isbn, const QJsonObject& book);
    bool deleteBook(const QString& isbn);
    QJsonArray getAllBooks();  // 买家使用：只返回状态为"正常"的书籍
    QJsonArray getAllBooksForSeller(int sellerId);  // 卖家使用：返回该卖家的所有书籍（包括待审核等所有状态）
    QJsonObject getBook(const QString& isbn);
    QJsonArray searchBooks(const QString& keyword);
    QJsonArray getPendingBooks();  // 获取待审核的书籍列表
    bool approveBook(const QString& isbn);  // 审核通过书籍
    bool rejectBook(const QString& isbn);  // 审核拒绝书籍
    
    // ===== 订单相关 =====
    QString createOrder(const QJsonObject& order);
    bool updateOrderStatus(const QString& orderId, const QString& status, const QString& paymentMethod = "", const QString& cancelReason = "", const QString& trackingNumber = "", double totalAmount = -1.0);
    QJsonArray getUserOrders(int userId);
    QJsonArray getAllOrders();
    QJsonArray getSellerOrders(int sellerId);  // 根据商家ID获取订单（通过解析items JSON中的merchantId）
    bool deleteOrder(const QString& orderId);
    QJsonObject getOrder(const QString& orderId);
    
    // ===== 购物车相关 =====
    bool addToCart(int userId, const QString& bookId, int quantity);
    QJsonArray getCart(int userId);
    bool updateCartQuantity(int userId, const QString& bookId, int quantity);
    bool removeFromCart(int userId, const QString& bookId);
    bool clearCart(int userId);
    
    // ===== 收藏相关 =====
    bool addFavorite(int userId, const QString& bookId);
    bool removeFavorite(int userId, const QString& bookId);
    QJsonArray getUserFavorites(int userId);
    int getBookFavoriteCount(const QString& bookId);
    
    // ===== 会员相关 =====
    bool addMember(const QJsonObject& member);
    bool updateMember(const QString& cardNo, const QJsonObject& member);
    bool deleteMember(const QString& cardNo);
    QJsonArray getAllMembers();
    bool rechargeMember(const QString& cardNo, double amount);
    
    // ===== 统计相关 =====
    QJsonObject getSystemStats();
    QJsonObject getSellerDashboardStats(int sellerId);  // 获取卖家统计报表数据
    QJsonArray getSellerSalesReport(int sellerId, const QString& startDate, const QString& endDate);  // 获取销售报表数据
    QJsonArray getSellerInventoryReport(int sellerId, const QString& startDate, const QString& endDate);  // 获取库存报表数据
    QJsonArray getSellerMemberReport(int sellerId, const QString& startDate, const QString& endDate);  // 获取会员报表数据
    
    // ===== 卖家申诉相关 =====
    bool submitSellerAppeal(int sellerId, const QString& sellerName, const QString& appealReason);
    QJsonObject getSellerAppeal(int sellerId);
    QJsonArray getAllAppeals(const QString& status = "");  // 获取所有申诉，可筛选状态
    bool reviewSellerAppeal(int appealId, int reviewerId, const QString& status, const QString& reviewComment);
    
    // ===== 聊天相关 =====
    bool saveChatMessage(int senderId, const QString& senderType, int receiverId, const QString& receiverType, const QString& message);
    QJsonArray getChatHistory(int userId, const QString& userType, int otherUserId = -1, const QString& otherUserType = "");
    QJsonArray getAllChatMessagesForAdmin();  // 管理员获取所有聊天记录
    
    // ===== 评论相关 =====
    bool addReview(int userId, const QString& bookId, int rating, const QString& comment);  // 添加评论
    QJsonArray getBookReviews(const QString& bookId);  // 获取商品的所有评论
    QJsonObject getBookRatingStats(const QString& bookId);  // 获取商品评分统计（平均分、评论数等）
    bool hasUserReviewedBook(int userId, const QString& bookId);  // 检查用户是否已评论过该商品
    bool hasUserPurchasedBook(int userId, const QString& bookId);  // 检查用户是否已购买过该商品
    QJsonArray getSellerReviews(int sellerId);  // 获取该卖家的所有商品评论（通过商品merchant_id关联）
    
    // ===== 初始化示例数据 =====
    bool initSampleBooks();
    
private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    
    QSqlDatabase m_db;
    bool m_connected;
    QMutex m_mutex;
};

#endif // DATABASE_H
