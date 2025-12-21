#include "tcpserver.h"
#include "clientthreadfactory.h"  // 新增：包含线程工厂头文件
#include "data.h"  // MySQL数据库支持
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QSet>
#include <cstdlib>

// #region agent log
// 调试日志辅助函数
static void writeDebugLog(const QString& location, const QString& message, const QJsonObject& data, const QString& hypothesisId = "")
{
    QFile logFile("f:\\Qt\\project\\bookmall\\bookadmin\\.cursor\\debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QJsonObject logEntry;
        logEntry["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        logEntry["location"] = location;
        logEntry["message"] = message;
        logEntry["data"] = data;
        logEntry["sessionId"] = "debug-session";
        logEntry["runId"] = "run1";
        if (!hypothesisId.isEmpty()) {
            logEntry["hypothesisId"] = hypothesisId;
        }
        out << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
    }
}
// #endregion

// 数据库开关：true=使用MySQL，false=使用内存
#define USE_DATABASE true  // 使用MySQL数据库

// 前置声明
static void ensureAdminDataInited();
static void ensureSellerBooksInited();

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    // 构造函数：初始化TCP服务器，暂无额外逻辑
}

// 新客户端连接处理
//void TcpServer::incomingConnection(qintptr socketDescriptor)
//{
//    // 调用线程工厂创建TCP线程任务
//    Task* task = ClientThreadFactory::getInstance().createThread(ProtocolType::TCP, socketDescriptor);
//    if (task != nullptr) {
//        ThreadPool::getInstance().addTask(task);
//    }
//}

// TCP文件传输任务构造函数：保存客户端套接字描述符
TcpFileTask::TcpFileTask(qintptr socketDescriptor, QObject *parent)
    : Task(), m_socketDescriptor(socketDescriptor), m_currentSellerId(-1), m_currentUserType("")
{
    Q_UNUSED(parent);  // 标记未使用的参数，消除编译器警告
}

// 任务执行逻辑：处理JSON格式的TCP请求（长度前缀协议）
void TcpFileTask::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "TCP套接字初始化失败：" << socket.errorString();
        emit logGenerated("TCP套接字初始化失败：" + socket.errorString());
        return;
    }

    m_clientIp = socket.peerAddress().toString();
    m_clientPort = socket.peerPort();
    emit logGenerated("TCP客户端连接：" + m_clientIp + ":" + QString::number(m_clientPort));
    emit dataReceived(m_clientIp, m_clientPort, "客户端已连接");

    QByteArray recvBuffer;  // 接收缓冲区，用于处理分片数据

    // 长连接模式：持续处理客户端请求
    while (socket.state() == QAbstractSocket::ConnectedState) {
        // 等待客户端发送请求，超时时间30秒
        if (!socket.waitForReadyRead(30000)) {
            if (socket.state() != QAbstractSocket::ConnectedState) {
                break;
            }
            continue;  // 超时但连接正常，继续等待
        }

        recvBuffer += socket.readAll();

        // 解析长度前缀协议：4字节大端长度 + JSON payload
        while (recvBuffer.size() >= 4) {
            // 读取长度前缀
            QDataStream ds(recvBuffer.left(4));
            ds.setByteOrder(QDataStream::BigEndian);
            quint32 payloadLen = 0;
            ds >> payloadLen;

            // 防御：检查payload长度是否合理（最大10MB）
            if (payloadLen > 10 * 1024 * 1024) {
                emit logGenerated("错误：客户端 [" + m_clientIp + "] 发送的payload长度过大:" + QString::number(payloadLen));
                socket.close();
                return;
            }

            // 检查是否收到完整帧
            if (recvBuffer.size() < 4 + (int)payloadLen) {
                // 数据不完整，等待更多数据
                break;
            }

            // 提取JSON payload
            QByteArray payload = recvBuffer.mid(4, payloadLen);
            recvBuffer = recvBuffer.mid(4 + payloadLen);  // 移除已处理的数据

            // 解析JSON
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(payload, &error);

            if (error.error != QJsonParseError::NoError || !doc.isObject()) {
                emit logGenerated("错误：客户端 [" + m_clientIp + "] 发送的JSON格式错误:" + error.errorString());
                QJsonObject errorResponse;
                errorResponse["success"] = false;
                errorResponse["message"] = "JSON格式错误";
                sendJsonResponse(socket, errorResponse);
                continue;
            }

            QJsonObject request = doc.object();
            QString action = request.value("action").toString();
            emit logGenerated("收到客户端 [" + m_clientIp + "] 请求: " + action);

            // 处理请求并发送响应
            QJsonObject response = processJsonRequest(request);
            sendJsonResponse(socket, response);

            emit logGenerated("已向客户端 [" + m_clientIp + "] 返回响应: " + action);
        }
    }

    emit logGenerated("TCP客户端断开连接：" + m_clientIp);
    socket.disconnectFromHost();
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    Task* task = ClientThreadFactory::getInstance().createThread(ProtocolType::TCP, socketDescriptor);
    if (task != nullptr) {
        // 将任务转换为TcpFileTask，连接信号
        TcpFileTask* tcpTask = dynamic_cast<TcpFileTask*>(task);
        if (tcpTask) {
            connect(tcpTask, &TcpFileTask::dataReceived, this, &TcpServer::dataReceived, Qt::QueuedConnection);
            connect(tcpTask, &TcpFileTask::logGenerated, this, &TcpServer::logGenerated, Qt::QueuedConnection);
        }
        ThreadPool::getInstance().addTask(task);
    }
}

// 处理JSON格式的请求
QJsonObject TcpFileTask::processJsonRequest(const QJsonObject &request)
{
    QString action = request.value("action").toString();
    QString category = "unknown"; // 请求分类
    
    // 确定请求分类
    if (action == "login" || action == "register") {
        category = "user";
    } else if (action.startsWith("admin")) {
        category = "admin";
    } else if (action.startsWith("seller")) {
        category = "seller";
    } else if (action.contains("Book") || action.contains("Search")) {
        category = "book";
    } else if (action.contains("Order") || action == "payOrder" || action == "cancelOrder" || action == "confirmReceiveOrder" || action == "shipOrder") {
        category = "order";
    } else if (action.contains("Cart")) {
        category = "cart";
    }
    
    QJsonObject response;

    if (action == "login") {
        response = handleLogin(request);
    } else if (action == "register") {
        response = handleRegister(request);
    } else if (action == "changePassword") {
        response = handleChangePassword(request);
    } else if (action == "updateUserInfo") {
        response = handleUpdateUserInfo(request);
    } else if (action == "getAllBooks") {
        return handleGetAllBooks(request);
    } else if (action == "getBook") {
        return handleGetBook(request);
    } else if (action == "searchBooks") {
        return handleSearchBooks(request);
    } else if (action == "addToCart") {
        return handleAddToCart(request);
    } else if (action == "getCart") {
        return handleGetCart(request);
    } else if (action == "updateCartQuantity") {
        return handleUpdateCartQuantity(request);
    } else if (action == "removeFromCart") {
        return handleRemoveFromCart(request);
    } else if (action == "addFavorite") {
        return handleAddFavorite(request);
    } else if (action == "removeFavorite") {
        return handleRemoveFavorite(request);
    } else if (action == "createOrder") {
        return handleCreateOrder(request);
    } else if (action == "getUserOrders") {
        return handleGetUserOrders(request);
    } else if (action == "payOrder") {
        return handlePayOrder(request);
    } else if (action == "rechargeBalance") {
        return handleRechargeBalance(request);
    } else if (action == "participateLottery") {
        return handleParticipateLottery(request);
    } else if (action == "sendChatMessage") {
        return handleSendChatMessage(request);
    } else if (action == "getChatHistory") {
        return handleGetChatHistory(request);
    } else if (action == "addReview") {
        return handleAddReview(request);
    } else if (action == "getBookReviews") {
        return handleGetBookReviews(request);
    } else if (action == "getBookRatingStats") {
        return handleGetBookRatingStats(request);
    } else if (action == "getSellerReviews") {
        return handleGetSellerReviews(request);
    } else if (action == "cancelOrder") {
        return handleCancelOrder(request);
    } else if (action == "confirmReceiveOrder") {
        return handleConfirmReceiveOrder(request);
    } else if (action == "shipOrder") {
        return handleShipOrder(request);
    } else if (action == "applySellerCertification") {
        return handleApplySellerCertification(request);
    } else if (action == "getSellerCertStatus") {
        return handleGetSellerCertStatus(request);
    } else if (action == "sellerGetBooks") {
        return handleSellerGetBooks(request);
    } else if (action == "sellerAddBook") {
        return handleSellerAddBook(request);
    } else if (action == "sellerUpdateBook") {
        return handleSellerUpdateBook(request);
    } else if (action == "sellerDeleteBook") {
        return handleSellerDeleteBook(request);
    } else if (action == "sellerGetOrders") {
        return handleSellerGetOrders(request);
    } else if (action == "sellerCreateOrder") {
        return handleSellerCreateOrder(request);
    } else if (action == "sellerUpdateOrderStatus") {
        return handleSellerUpdateOrderStatus(request);
    } else if (action == "sellerDeleteOrder") {
        return handleSellerDeleteOrder(request);
    } else if (action == "sellerGetMembers") {
        return handleSellerGetMembers(request);
    } else if (action == "sellerAddMember") {
        return handleSellerAddMember(request);
    } else if (action == "sellerUpdateMember") {
        return handleSellerUpdateMember(request);
    } else if (action == "sellerDeleteMember") {
        return handleSellerDeleteMember(request);
    } else if (action == "sellerRechargeMember") {
        return handleSellerRechargeMember(request);
    } else if (action == "sellerDashboardStats") {
        return handleSellerDashboardStats(request);
    } else if (action == "sellerGetSystemSettings") {
        return handleSellerGetSystemSettings(request);
    } else if (action == "sellerUpdateSystemSettings") {
        return handleSellerUpdateSystemSettings(request);
    } else if (action == "sellerGetReportSales") {
        return handleSellerGetReportSales(request);
    } else if (action == "sellerGetReportInventory") {
        return handleSellerGetReportInventory(request);
    } else if (action == "sellerGetReportMember") {
        return handleSellerGetReportMember(request);
    } else if (action == "sellerGetProfile") {
        return handleSellerGetProfile(request);
    } else if (action == "sellerSubmitAppeal") {
        return handleSellerSubmitAppeal(request);
    } else if (action == "sellerGetAppeal") {
        return handleSellerGetAppeal(request);
    } else if (action == "adminLogin") {
        return handleAdminLogin(request);
    } else if (action == "adminGetAllUsers") {
        return handleAdminGetAllUsers(request);
    } else if (action == "adminDeleteUser") {
        return handleAdminDeleteUser(request);
    } else if (action == "adminBanUser") {
        return handleAdminBanUser(request);
    } else if (action == "adminGetSellerCertification") {
        return handleAdminGetSellerCertification(request);
    } else if (action == "adminApproveSellerCertification") {
        return handleAdminApproveSellerCertification(request);
    } else if (action == "adminRejectSellerCertification") {
        return handleAdminRejectSellerCertification(request);
    } else if (action == "adminGetAllAppeals") {
        return handleAdminGetAllAppeals(request);
    } else if (action == "adminReviewAppeal") {
        return handleAdminReviewAppeal(request);
    } else if (action == "adminGetAllSellers") {
        return handleAdminGetAllSellers(request);
    } else if (action == "adminDeleteSeller") {
        return handleAdminDeleteSeller(request);
    } else if (action == "adminBanSeller") {
        return handleAdminBanSeller(request);
    } else if (action == "adminGetAllBooks") {
        return handleAdminGetAllBooks(request);
    } else if (action == "adminGetPendingBooks") {
        return handleAdminGetPendingBooks(request);
    } else if (action == "adminGetPendingSellerCertifications") {
        return handleAdminGetPendingSellerCertifications(request);
    } else if (action == "adminApproveBook") {
        return handleAdminApproveBook(request);
    } else if (action == "adminRejectBook") {
        return handleAdminRejectBook(request);
    } else if (action == "adminDeleteBook") {
        return handleAdminDeleteBook(request);
    } else if (action == "adminUpdateBook") {
        return handleAdminUpdateBook(request);
    } else if (action == "adminGetAllOrders") {
        return handleAdminGetAllOrders(request);
    } else if (action == "adminDeleteOrder") {
        return handleAdminDeleteOrder(request);
    } else if (action == "adminGetSystemStats") {
        response = handleAdminGetSystemStats(request);
    } else if (action == "adminGetRequestLogs") {
        response = handleAdminGetRequestLogs(request);
    } else {
        response["success"] = false;
        response["message"] = "未知的请求类型: " + action;
    }
    
#if USE_DATABASE
    // 记录请求日志到数据库
    if (Database::getInstance().isConnected()) {
        Database::getInstance().logRequest(
            m_clientIp,
            m_clientPort,
            action,
            request,
            response,
            response.value("success").toBool(),
            category
        );
    }
#endif
    
    return response;
}

// ===== 卖家端：内存书库（线程安全）=====
// ===== 卖家端全局变量 =====
static QMutex g_sellerBooksMutex;
static QList<QJsonObject> g_sellerBooks;
static bool g_sellerBooksInited = false;
static QMutex g_sellerOrdersMutex;
static QList<QJsonObject> g_sellerOrders;
static QMutex g_sellerMembersMutex;
static QList<QJsonObject> g_sellerMembers;
static QMutex g_sellerSettingsMutex;
static QJsonObject g_sellerSettings;

// ===== 管理员端全局变量（买家和商家数据）=====
static QMutex g_adminMutex;
static QList<QJsonObject> g_adminUsers;      // 所有买家用户
static QList<QJsonObject> g_adminSellers;    // 所有商家
static bool g_adminDataInited = false;

static void ensureSellerBooksInited()
{
    QMutexLocker locker(&g_sellerBooksMutex);
    if (g_sellerBooksInited) return;

    // 初始化为空列表，等待卖家上架图书
    // 不再预设任何图书数据
    // 业务逻辑：卖家先上架商品，买家才能浏览购买

    g_sellerBooksInited = true;
}

QJsonObject TcpFileTask::handleSellerGetBooks(const QJsonObject &request)
{
    QString sellerId = request.value("sellerId").toString();
    
    QJsonObject resp;
    resp["success"] = true;
    resp["message"] = "获取卖家图书列表成功";

#if USE_DATABASE
    // 从数据库加载该商家的图书
    if (Database::getInstance().isConnected()) {
        int merchantId = sellerId.toInt();
        // 使用专门的方法获取该卖家的所有书籍（包括待审核等所有状态）
        QJsonArray sellerBooks = Database::getInstance().getAllBooksForSeller(merchantId);
        
        resp["books"] = sellerBooks;
        resp["total"] = sellerBooks.size();
    } else {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        resp["books"] = QJsonArray();
        resp["total"] = 0;
    }
#else
    // 不使用数据库时，从内存加载
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    QJsonArray arr;
    int merchantId = sellerId.toInt();
    for (const auto &b : g_sellerBooks) {
        // 只返回该商家的图书
        if (b.value("merchantId").toInt() == merchantId) {
            arr.append(b);
        }
    }
    resp["books"] = arr;
    resp["total"] = arr.size();
#endif
    
    return resp;
}

QJsonObject TcpFileTask::handleSellerAddBook(const QJsonObject &request)
{
    QString isbn = request.value("isbn").toString();
    if (isbn.isEmpty()) {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "isbn不能为空";
        return resp;
    }
    
    // 自动从当前登录会话获取商家ID（完全基于会话状态，不依赖客户端传递）
    int sellerId = -1;
    qDebug() << "=== 添加图书调试信息 ===";
    qDebug() << "客户端IP:" << m_clientIp << "端口:" << m_clientPort;
    qDebug() << "当前会话商家ID:" << m_currentSellerId;
    qDebug() << "当前用户类型:" << m_currentUserType;
    qDebug() << "请求中的sellerId:" << request.value("sellerId").toString();
    
    // 优先使用会话中的商家ID（最安全可靠的方式）
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
        qDebug() << "✓ 自动使用当前登录会话中的商家ID:" << sellerId;
    } else {
        // 如果会话中没有商家ID，尝试从请求中获取（向后兼容，但不推荐）
        QString sellerIdStr = request.value("sellerId").toString();
        qDebug() << "⚠ 警告：会话中无商家ID，尝试从请求中获取";
        qDebug() << "请求中的sellerId字符串:" << sellerIdStr;
        
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            int tempSellerId = sellerIdStr.toInt(&ok);
            if (ok && tempSellerId > 0) {
                sellerId = tempSellerId;
                qDebug() << "⚠ 使用请求中的商家ID（向后兼容）:" << sellerId;
                // 同时更新会话状态，以便后续请求使用
                m_currentSellerId = sellerId;
                m_currentUserType = "seller";
                qDebug() << "✓ 已更新会话状态，商家ID:" << sellerId;
            } else {
                qDebug() << "✗ 请求中的商家ID无效:" << sellerIdStr << "转换后:" << tempSellerId;
        QJsonObject resp;
        resp["success"] = false;
                resp["message"] = "商家ID无效（" + sellerIdStr + "），请先登录商家账号";
        return resp;
            }
        } else {
            qDebug() << "✗ 无法获取商家ID - 会话中无，请求中也为空或无效";
            qDebug() << "提示：请确保已成功登录商家账号，且登录和添加图书使用同一个TCP连接";
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "商家ID不能为空，请先登录商家账号";
            return resp;
        }
    }
    
#if USE_DATABASE
    // 使用数据库保存
    if (Database::getInstance().isConnected()) {
        // 检查商家状态，如果被封禁则不允许添加图书
        QJsonArray sellersArray = Database::getInstance().getAllSellers();
        bool sellerBanned = false;
        for (const QJsonValue &sellerVal : sellersArray) {
            QJsonObject seller = sellerVal.toObject();
            if (seller.value("sellerId").toInt() == sellerId) {
                QString sellerStatus = seller.value("status").toString();
                if (sellerStatus == "封禁") {
                    sellerBanned = true;
                }
                break;
            }
        }
        if (sellerBanned) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "您的账号已被封禁，无法添加图书";
            return resp;
        }
        
        // 检查ISBN是否已存在
        QJsonObject existingBook = Database::getInstance().getBook(isbn);
        if (!existingBook.isEmpty() && existingBook.contains("isbn")) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "ISBN已存在";
            return resp;
        }
        
        // 构建图书数据
        QJsonObject bookData;
        bookData["isbn"] = isbn;
        bookData["title"] = request.value("title").toString();
        bookData["author"] = request.value("author").toString();
        // 兼容旧数据：优先使用category1，如果没有则使用category
        bookData["category1"] = request.contains("category1") ? request.value("category1").toString() : 
                                 (request.contains("category") ? request.value("category").toString() : "");
        bookData["category2"] = request.contains("category2") ? request.value("category2").toString() : "";
        bookData["merchantId"] = sellerId;  // 使用自动获取的商家ID
        bookData["price"] = request.value("price").toDouble();
        bookData["stock"] = request.value("stock").toInt();
        // 卖家添加的书籍状态必须为"待审核"，需要管理员审核后才能上架
        // 强制设置为"待审核"，忽略请求中可能存在的status字段
        bookData["status"] = "待审核";
        // 书籍描述：优先使用description，如果没有则为空
        bookData["description"] = request.contains("description") ? request.value("description").toString() : QString();
        // 封面图片：优先使用coverImage，如果没有则使用cover_image（向后兼容）
        bookData["coverImage"] = request.contains("coverImage") ? request.value("coverImage").toString() : 
                                 (request.contains("cover_image") ? request.value("cover_image").toString() : QString());
        
        qDebug() << "添加图书 - ISBN:" << isbn << "封面图片长度:" << bookData["coverImage"].toString().length();
        
        // 保存到数据库
        if (Database::getInstance().addBook(bookData)) {
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "添加图书成功，等待管理员审核";
            return resp;
        } else {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "保存到数据库失败";
            return resp;
        }
    } else {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);
    
    for (const auto &b : g_sellerBooks) {
        if (b.value("isbn").toString() == isbn) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "isbn已存在";
            return resp;
        }
    }

    QJsonObject b;
    b["isbn"] = isbn;
    b["title"] = request.value("title").toString();
    b["author"] = request.value("author").toString();
    // 兼容旧数据：优先使用category1，如果没有则使用category
    b["category1"] = request.contains("category1") ? request.value("category1").toString() : 
                      (request.contains("category") ? request.value("category").toString() : "");
    b["category2"] = request.contains("category2") ? request.value("category2").toString() : "";
    b["merchantId"] = sellerId;  // 使用自动获取的商家ID
    b["price"] = request.value("price").toDouble();
    b["stock"] = request.value("stock").toInt();
    b["status"] = request.value("status").toString("正常");
    // 兼容旧数据：同时提供category字段
    b["category"] = b["category1"].toString();

    g_sellerBooks.append(b);

    QJsonObject resp;
    resp["success"] = true;
    resp["message"] = "添加图书成功";
    return resp;
#endif
}

QJsonObject TcpFileTask::handleSellerUpdateBook(const QJsonObject &request)
{
    QString isbn = request.value("isbn").toString();
    if (isbn.isEmpty()) {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "isbn不能为空";
        return resp;
    }
    
    // 自动从当前登录会话获取商家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            int tempSellerId = sellerIdStr.toInt(&ok);
            if (ok && tempSellerId > 0) {
                sellerId = tempSellerId;
            }
        }
    }
    
#if USE_DATABASE
    // 使用数据库保存
    if (Database::getInstance().isConnected()) {
        // 检查商家状态，如果被封禁则不允许更新图书
        if (sellerId > 0) {
            QJsonArray sellersArray = Database::getInstance().getAllSellers();
            bool sellerBanned = false;
            for (const QJsonValue &sellerVal : sellersArray) {
                QJsonObject seller = sellerVal.toObject();
                if (seller.value("sellerId").toInt() == sellerId) {
                    QString sellerStatus = seller.value("status").toString();
                    if (sellerStatus == "封禁") {
                        sellerBanned = true;
                    }
                    break;
                }
            }
            if (sellerBanned) {
                QJsonObject resp;
                resp["success"] = false;
                resp["message"] = "您的账号已被封禁，无法更新图书";
                return resp;
            }
        }
        
        // 检查图书是否存在，以及是否属于当前商家
        QJsonObject existingBook = Database::getInstance().getBook(isbn);
        if (existingBook.isEmpty() || !existingBook.contains("isbn")) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "未找到指定ISBN的图书";
            return resp;
        }
        
        // 如果提供了sellerId，验证图书是否属于该商家
        if (sellerId > 0) {
            int bookMerchantId = existingBook.value("merchantId").toInt();
            if (bookMerchantId != sellerId) {
                QJsonObject resp;
                resp["success"] = false;
                resp["message"] = "您无权更新此图书";
                return resp;
            }
        }
        
        // 构建更新数据
        QJsonObject bookData;
        if (request.contains("title")) bookData["title"] = request.value("title").toString();
        if (request.contains("author")) bookData["author"] = request.value("author").toString();
        if (request.contains("category1")) {
            bookData["category1"] = request.value("category1").toString();
        } else if (request.contains("category")) {
            bookData["category1"] = request.value("category").toString();
        }
        if (request.contains("category2")) bookData["category2"] = request.value("category2").toString();
        if (request.contains("price")) bookData["price"] = request.value("price").toDouble();
        if (request.contains("stock")) bookData["stock"] = request.value("stock").toInt();
        if (request.contains("status")) bookData["status"] = request.value("status").toString();
        if (request.contains("description")) bookData["description"] = request.value("description").toString();
        if (request.contains("coverImage") || request.contains("cover_image")) {
            bookData["coverImage"] = request.contains("coverImage") ? request.value("coverImage").toString() : 
                                     request.value("cover_image").toString();
        }
        
        // 更新到数据库
        if (Database::getInstance().updateBook(isbn, bookData)) {
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "更新图书成功";
            return resp;
        } else {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "更新图书失败";
            return resp;
        }
    } else {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    for (auto &b : g_sellerBooks) {
        if (b.value("isbn").toString() == isbn) {
            // 逐字段更新（缺省则保留）
            if (request.contains("title")) b["title"] = request.value("title").toString();
            if (request.contains("author")) b["author"] = request.value("author").toString();
            if (request.contains("category1")) {
                b["category1"] = request.value("category1").toString();
                b["category"] = request.value("category1").toString(); // 兼容旧数据
            } else if (request.contains("category")) {
                b["category1"] = request.value("category").toString();
                b["category"] = request.value("category").toString();
            }
            if (request.contains("category2")) b["category2"] = request.value("category2").toString();
            if (request.contains("merchantId") || request.contains("merchant_id")) {
                b["merchantId"] = request.contains("merchantId") ? request.value("merchantId").toInt() : 
                                  request.value("merchant_id").toInt();
            }
            if (request.contains("price")) b["price"] = request.value("price").toDouble();
            if (request.contains("stock")) b["stock"] = request.value("stock").toInt();
            if (request.contains("status")) b["status"] = request.value("status").toString();

            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "更新图书成功";
            return resp;
        }
    }

    QJsonObject resp;
    resp["success"] = false;
    resp["message"] = "未找到指定isbn";
    return resp;
#endif
}

QJsonObject TcpFileTask::handleSellerDeleteBook(const QJsonObject &request)
{
    QString isbn = request.value("isbn").toString();
    if (isbn.isEmpty()) {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "isbn不能为空";
        return resp;
    }
    
    // 自动从当前登录会话获取商家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            int tempSellerId = sellerIdStr.toInt(&ok);
            if (ok && tempSellerId > 0) {
                sellerId = tempSellerId;
            }
        }
    }
    
#if USE_DATABASE
    // 使用数据库保存
    if (Database::getInstance().isConnected()) {
        // 检查商家状态，如果被封禁则不允许删除图书
        if (sellerId > 0) {
            QJsonArray sellersArray = Database::getInstance().getAllSellers();
            bool sellerBanned = false;
            for (const QJsonValue &sellerVal : sellersArray) {
                QJsonObject seller = sellerVal.toObject();
                if (seller.value("sellerId").toInt() == sellerId) {
                    QString sellerStatus = seller.value("status").toString();
                    if (sellerStatus == "封禁") {
                        sellerBanned = true;
                    }
                    break;
                }
            }
            if (sellerBanned) {
                QJsonObject resp;
                resp["success"] = false;
                resp["message"] = "您的账号已被封禁，无法删除图书";
                return resp;
            }
        }
        
        // 检查图书是否存在，以及是否属于当前商家
        QJsonObject existingBook = Database::getInstance().getBook(isbn);
        if (existingBook.isEmpty() || !existingBook.contains("isbn")) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "未找到指定ISBN的图书";
            return resp;
        }
        
        // 如果提供了sellerId，验证图书是否属于该商家
        if (sellerId > 0) {
            int bookMerchantId = existingBook.value("merchantId").toInt();
            if (bookMerchantId != sellerId) {
                QJsonObject resp;
                resp["success"] = false;
                resp["message"] = "您无权删除此图书";
                return resp;
            }
        }
        
        // 从数据库删除
        if (Database::getInstance().deleteBook(isbn)) {
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "删除图书成功";
            return resp;
        } else {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "删除图书失败";
            return resp;
        }
    } else {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    for (int i = 0; i < g_sellerBooks.size(); ++i) {
        if (g_sellerBooks[i].value("isbn").toString() == isbn) {
            g_sellerBooks.removeAt(i);
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "删除图书成功";
            return resp;
        }
    }

    QJsonObject resp;
    resp["success"] = false;
    resp["message"] = "未找到指定isbn";
    return resp;
#endif
}

// ===== 卖家订单（内存模拟）=====
// 订单字段示例：orderId/customer/phone/amount/status/payment/createTime/address/operator/remark
static QString genOrderId()
{
    return QString("SELLER%1").arg(QDateTime::currentMSecsSinceEpoch());
}

QJsonObject TcpFileTask::handleSellerGetOrders(const QJsonObject &request)
{
    QJsonObject resp;
    
    // 自动从当前登录会话获取商家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                resp["success"] = false;
                resp["message"] = "商家ID无效";
                return resp;
            }
        }
    }
    
    if (sellerId <= 0) {
        resp["success"] = false;
        resp["message"] = "请先登录商家账号";
        return resp;
    }
    
#if USE_DATABASE
    // 从数据库读取订单
    if (Database::getInstance().isConnected()) {
        QJsonArray orders = Database::getInstance().getSellerOrders(sellerId);
        
        double totalSales = 0.0;
        int totalOrders = 0;
        int paidOrders = 0;
        int shippedOrders = 0;
        int cancelledOrders = 0;
        
        for (const QJsonValue &orderVal : orders) {
            QJsonObject order = orderVal.toObject();
            totalOrders++;
            
            QString status = order["status"].toString();
            if (status == "已支付" || status == "已发货") {
                totalSales += order["totalAmount"].toDouble();
                paidOrders++;
            }
            if (status == "已发货") {
                shippedOrders++;
            }
            if (status == "已取消") {
                cancelledOrders++;
            }
        }
        
        resp["success"] = true;
        resp["orders"] = orders;
        resp["total"] = totalOrders;
        resp["totalSales"] = totalSales;
        resp["paidOrders"] = paidOrders;
        resp["shippedOrders"] = shippedOrders;
        resp["cancelledOrders"] = cancelledOrders;
        return resp;
    } else {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);

    QJsonArray arr;
    double totalSales = 0.0;
    int totalOrders = 0;
    int paidOrders = 0;
    int shippedOrders = 0;
    int cancelledOrders = 0;
    
    for (const auto &o : g_sellerOrders) {
        // 检查订单项中是否包含该商家的商品
        bool hasSellerItem = false;
        if (o.contains("items")) {
            QJsonArray items = o["items"].toArray();
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                if (item.contains("merchantId") && item["merchantId"].toInt() == sellerId) {
                    hasSellerItem = true;
                    break;
                }
            }
        }
        
        if (hasSellerItem) {
            arr.append(o);
            totalOrders++;
            
            QString status = o["status"].toString();
            if (status == "已支付" || status == "已发货") {
                totalSales += o["totalAmount"].toDouble();
                paidOrders++;
            }
            if (status == "已发货") {
                shippedOrders++;
            }
            if (status == "已取消") {
                cancelledOrders++;
            }
        }
    }

    resp["success"] = true;
    resp["orders"] = arr;
    resp["total"] = totalOrders;
    resp["totalSales"] = totalSales;
    resp["paidOrders"] = paidOrders;
    resp["shippedOrders"] = shippedOrders;
    resp["cancelledOrders"] = cancelledOrders;
    return resp;
#endif
}

QJsonObject TcpFileTask::handleSellerCreateOrder(const QJsonObject &request)
{
    QMutexLocker locker(&g_sellerOrdersMutex);
    QJsonObject o;
    o["orderId"] = genOrderId();
    o["customer"] = request.value("customer").toString();
    o["phone"] = request.value("phone").toString();
    o["amount"] = request.value("amount").toDouble();
    o["status"] = request.value("status").toString("待付款");
    o["payment"] = request.value("payment").toString("现金");
    o["createTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    o["address"] = request.value("address").toString();
    o["operator"] = request.value("operator").toString("系统");
    o["remark"] = request.value("remark").toString();
    g_sellerOrders.append(o);

    QJsonObject resp;
    resp["success"] = true;
    resp["message"] = "创建订单成功";
    resp["orderId"] = o.value("orderId").toString();
    return resp;
}

QJsonObject TcpFileTask::handleSellerUpdateOrderStatus(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    QString status = request.value("status").toString();
    
    if (orderId.isEmpty() || status.isEmpty()) {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "订单ID和状态不能为空";
        return resp;
    }
    
#if USE_DATABASE
    // 从数据库更新订单状态
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().updateOrderStatus(orderId, status, "", "")) {
            // 同时更新内存中的订单（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (auto &o : g_sellerOrders) {
                if (o.value("orderId").toString() == orderId) {
                    o["status"] = status;
                    if (status == "已发货") {
                        o["shipTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    }
                    break;
                }
            }
            
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "更新订单状态成功";
            return resp;
        } else {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "更新订单状态失败";
            return resp;
        }
    } else {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    for (auto &o : g_sellerOrders) {
        if (o.value("orderId").toString() == orderId) {
            o["status"] = status;
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "更新订单状态成功";
            return resp;
        }
    }
    QJsonObject resp;
    resp["success"] = false;
    resp["message"] = "订单不存在";
    return resp;
#endif
}

QJsonObject TcpFileTask::handleSellerDeleteOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    
    if (orderId.isEmpty()) {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "订单ID不能为空";
        return resp;
    }
    
#if USE_DATABASE
    // 从数据库删除订单
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().deleteOrder(orderId)) {
            // 同时从内存中删除（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (int i = 0; i < g_sellerOrders.size(); ++i) {
                if (g_sellerOrders[i].value("orderId").toString() == orderId) {
                    g_sellerOrders.removeAt(i);
                    break;
                }
            }
            
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "删除订单成功";
            return resp;
        } else {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "删除订单失败";
            return resp;
        }
    } else {
        QJsonObject resp;
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i].value("orderId").toString() == orderId) {
            g_sellerOrders.removeAt(i);
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "删除订单成功";
            return resp;
        }
    }
    QJsonObject resp;
    resp["success"] = false;
    resp["message"] = "订单不存在";
    return resp;
#endif
}

// ===== 卖家会员（内存模拟）=====
QJsonObject TcpFileTask::handleSellerGetMembers(const QJsonObject &request)
{
    Q_UNUSED(request);
    QJsonObject resp;
    
#if USE_DATABASE
    // 使用数据库获取所有买家用户（role=1）及其会员等级
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        resp["members"] = QJsonArray();
        resp["total"] = 0;
        return resp;
    }
    
    // 获取当前登录的卖家ID
    int currentSellerId = m_currentSellerId;
    
    // 获取所有用户
    QJsonArray allUsers = Database::getInstance().getAllUsers();
    
    QJsonArray members;
    
    // 只处理users表中role=1的买家用户
    for (const QJsonValue &userValue : allUsers) {
        QJsonObject user = userValue.toObject();
        int userId = user["userId"].toInt();
        int role = user["role"].toInt();
        QString status = user["status"].toString();
        
        // 只返回买家用户（role = 1）
        if (role != 1) {
            continue;
        }
        
        // 排除当前登录的卖家自己（如果当前卖家ID在users表中且role=1，也要排除）
        if (userId == currentSellerId) {
            continue;
        }
        
        // 只返回状态为"正常"的用户（排除已删除的用户）
        if (status.isEmpty() || status == "正常") {
            QJsonObject member;
            member["userId"] = userId;
            member["username"] = user["username"];
            member["email"] = user["email"];
            // 卖家不能看到会员余额，不返回balance字段
            member["registerDate"] = user["registerDate"];
            member["role"] = 1;  // 买家
            
            // 使用member_level字段（字符串类型）
            QString memberLevel = user["memberLevel"].toString();
            // 确保memberLevel不为空，如果为空则设置为默认值
            if (memberLevel.isEmpty() || memberLevel.isNull()) {
                memberLevel = "普通会员";
            }
            member["memberLevel"] = memberLevel;
            
            // 保留向后兼容的membershipLevel字段（数字1-5）
            if (user.contains("membershipLevel")) {
                member["membershipLevel"] = user["membershipLevel"];
            } else {
                // 如果没有membershipLevel，根据memberLevel映射
                int membershipLevel = 1;
                if (memberLevel == "普通会员") membershipLevel = 1;
                else if (memberLevel == "银卡会员") membershipLevel = 2;
                else if (memberLevel == "金卡会员") membershipLevel = 3;
                else if (memberLevel == "白金会员") membershipLevel = 4;
                else if (memberLevel == "钻石会员") membershipLevel = 5;
                member["membershipLevel"] = membershipLevel;
            }
            
            members.append(member);
        }
    }
    
    resp["success"] = true;
    resp["members"] = members;
    resp["total"] = members.size();
#else
    // 使用内存存储（原有逻辑）
    QMutexLocker locker(&g_sellerMembersMutex);
    QJsonArray arr;
    for (const auto &m : g_sellerMembers) arr.append(m);
    resp["success"] = true;
    resp["members"] = arr;
    resp["total"] = arr.size();
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerAddMember(const QJsonObject &request)
{
    QMutexLocker locker(&g_sellerMembersMutex);
    QString cardNo = request.value("cardNo").toString();
    for (const auto &m : g_sellerMembers) {
        if (m.value("cardNo").toString() == cardNo) {
            QJsonObject resp;
            resp["success"] = false;
            resp["message"] = "会员卡号已存在";
            return resp;
        }
    }
    QJsonObject m;
    m["cardNo"] = cardNo;
    m["name"] = request.value("name").toString();
    m["phone"] = request.value("phone").toString();
    m["level"] = request.value("level").toString("普通");
    m["balance"] = request.value("balance").toDouble();
    m["points"] = request.value("points").toInt();
    m["createDate"] = QDate::currentDate().toString("yyyy-MM-dd");
    g_sellerMembers.append(m);

    QJsonObject resp;
    resp["success"] = true;
    resp["message"] = "新增会员成功";
    return resp;
}

QJsonObject TcpFileTask::handleSellerUpdateMember(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    // 使用数据库更新会员信息
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    QString userIdStr = request.value("userId").toString();
    if (userIdStr.isEmpty()) {
        resp["success"] = false;
        resp["message"] = "用户ID不能为空";
        return resp;
    }
    
    bool ok;
    int userId = userIdStr.toInt(&ok);
    if (!ok || userId <= 0) {
        resp["success"] = false;
        resp["message"] = "无效的用户ID";
        return resp;
    }
    
    // 更新邮箱
    if (request.contains("email")) {
        QString email = request.value("email").toString();
        QString phone = "";
        QString address = "";
        // 先获取用户当前信息
        QJsonObject user = Database::getInstance().getUserById(userId);
        if (!user.isEmpty()) {
            phone = user["phone"].toString();
            address = user["address"].toString();
        }
        if (!Database::getInstance().updateUserInfo(userId, phone, email, address)) {
            resp["success"] = false;
            resp["message"] = "更新邮箱失败";
            return resp;
        }
    }
    
    // 更新会员等级
    if (request.contains("membershipLevel") || request.contains("memberLevel")) {
        QString memberLevel;
        if (request.contains("memberLevel")) {
            memberLevel = request.value("memberLevel").toString();
        } else if (request.contains("membershipLevel")) {
            // 将数字等级转换为字符串
            int level = request.value("membershipLevel").toInt();
            switch (level) {
                case 1: memberLevel = "普通会员"; break;
                case 2: memberLevel = "银卡会员"; break;
                case 3: memberLevel = "金卡会员"; break;
                case 4: memberLevel = "白金会员"; break;
                case 5: memberLevel = "钻石会员"; break;
                default: memberLevel = "普通会员"; break;
            }
        }
        
        if (!memberLevel.isEmpty()) {
            // 使用Database类的公共方法更新会员等级
            if (!Database::getInstance().updateUserMemberLevel(userId, memberLevel)) {
                resp["success"] = false;
                resp["message"] = "更新会员等级失败";
                return resp;
            }
        }
    }
    
    resp["success"] = true;
    resp["message"] = "更新会员成功";
#else
    // 使用内存存储（原有逻辑）
    QMutexLocker locker(&g_sellerMembersMutex);
    QString cardNo = request.value("cardNo").toString();
    for (auto &m : g_sellerMembers) {
        if (m.value("cardNo").toString() == cardNo) {
            if (request.contains("name")) m["name"] = request.value("name").toString();
            if (request.contains("phone")) m["phone"] = request.value("phone").toString();
            if (request.contains("level")) m["level"] = request.value("level").toString();
            if (request.contains("balance")) m["balance"] = request.value("balance").toDouble();
            if (request.contains("points")) m["points"] = request.value("points").toInt();
            resp["success"] = true;
            resp["message"] = "更新会员成功";
            return resp;
        }
    }
    resp["success"] = false;
    resp["message"] = "会员不存在";
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerDeleteMember(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    // 使用数据库删除会员（将用户状态设置为"已删除"）
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    QString userIdStr = request.value("userId").toString();
    if (userIdStr.isEmpty()) {
        resp["success"] = false;
        resp["message"] = "用户ID不能为空";
        return resp;
    }
    
    bool ok;
    int userId = userIdStr.toInt(&ok);
    if (!ok || userId <= 0) {
        resp["success"] = false;
        resp["message"] = "无效的用户ID";
        return resp;
    }
    
    // 将用户状态设置为"已删除"
    if (Database::getInstance().updateUserStatus(userId, "已删除")) {
        resp["success"] = true;
        resp["message"] = "删除会员成功";
    } else {
        resp["success"] = false;
        resp["message"] = "删除会员失败";
    }
#else
    // 使用内存存储（原有逻辑）
    QMutexLocker locker(&g_sellerMembersMutex);
    QString cardNo = request.value("cardNo").toString();
    for (int i = 0; i < g_sellerMembers.size(); ++i) {
        if (g_sellerMembers[i].value("cardNo").toString() == cardNo) {
            g_sellerMembers.removeAt(i);
            resp["success"] = true;
            resp["message"] = "删除会员成功";
            return resp;
        }
    }
    resp["success"] = false;
    resp["message"] = "会员不存在";
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerRechargeMember(const QJsonObject &request)
{
    QMutexLocker locker(&g_sellerMembersMutex);
    QString cardNo = request.value("cardNo").toString();
    double amount = request.value("amount").toDouble();
    for (auto &m : g_sellerMembers) {
        if (m.value("cardNo").toString() == cardNo) {
            double bal = m.value("balance").toDouble();
            m["balance"] = bal + amount;
            QJsonObject resp;
            resp["success"] = true;
            resp["message"] = "充值成功";
            resp["balance"] = m["balance"];
            return resp;
        }
    }
    QJsonObject resp;
    resp["success"] = false;
    resp["message"] = "会员不存在";
    return resp;
}

// ===== 仪表盘/系统设置/报表（简易模拟）=====
QJsonObject TcpFileTask::handleSellerDashboardStats(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    // 使用数据库获取统计数据
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    // 获取当前登录的卖家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty()) {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                resp["success"] = false;
                resp["message"] = "无效的卖家ID";
                return resp;
            }
        }
    }
    
    if (sellerId <= 0) {
        resp["success"] = false;
        resp["message"] = "卖家ID不能为空";
        return resp;
    }
    
    // 使用Database类的公共方法获取统计数据
    QJsonObject stats = Database::getInstance().getSellerDashboardStats(sellerId);
    
    resp["success"] = true;
    resp["stats"] = stats;
#else
    // 使用内存存储（原有逻辑）
    resp["success"] = true;
    resp["todaySales"] = 5200.0;
    resp["pendingOrders"] = 12;
    resp["lowStock"] = 8;
    resp["memberCount"] = 230;
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerGetSystemSettings(const QJsonObject &request)
{
    Q_UNUSED(request);
    QMutexLocker locker(&g_sellerSettingsMutex);
    if (g_sellerSettings.isEmpty()) {
        g_sellerSettings["shopName"] = "示例书店";
        g_sellerSettings["owner"] = "Admin";
        g_sellerSettings["phone"] = "13800000000";
    }
    QJsonObject resp;
    resp["success"] = true;
    resp["settings"] = g_sellerSettings;
    return resp;
}

QJsonObject TcpFileTask::handleSellerUpdateSystemSettings(const QJsonObject &request)
{
    QMutexLocker locker(&g_sellerSettingsMutex);
    QJsonObject settings = request.value("settings").toObject();
    for (auto it = settings.begin(); it != settings.end(); ++it) {
        g_sellerSettings[it.key()] = it.value();
    }
    QJsonObject resp;
    resp["success"] = true;
    resp["message"] = "保存设置成功";
    return resp;
}

QJsonObject TcpFileTask::handleSellerGetReportSales(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    // 获取卖家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty()) {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                resp["success"] = false;
                resp["message"] = "无效的卖家ID";
                return resp;
            }
        }
    }
    
    if (sellerId <= 0) {
        resp["success"] = false;
        resp["message"] = "卖家ID不能为空";
        return resp;
    }
    
    // 获取日期范围
    QString startDate = request.value("startDate").toString();
    QString endDate = request.value("endDate").toString();
    
    if (startDate.isEmpty() || endDate.isEmpty()) {
        // 如果没有提供日期，默认查询最近30天
        QDate end = QDate::currentDate();
        QDate start = end.addDays(-30);
        startDate = start.toString("yyyy-MM-dd");
        endDate = end.toString("yyyy-MM-dd");
    }
    
    // 从数据库获取销售报表数据
    QJsonArray data = Database::getInstance().getSellerSalesReport(sellerId, startDate, endDate);
    
    resp["success"] = true;
    resp["data"] = data;
    resp["startDate"] = startDate;
    resp["endDate"] = endDate;
#else
    // 使用内存存储（原有逻辑）
    resp["success"] = true;
    QJsonArray arr;
    QJsonObject item;
    item["date"] = QDate::currentDate().toString("yyyy-MM-dd");
    item["amount"] = 1234.56;
    arr.append(item);
    resp["data"] = arr;
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerGetReportInventory(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    // 获取卖家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty()) {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                resp["success"] = false;
                resp["message"] = "无效的卖家ID";
                return resp;
            }
        }
    }
    
    if (sellerId <= 0) {
        resp["success"] = false;
        resp["message"] = "卖家ID不能为空";
        return resp;
    }
    
    // 获取日期范围
    QString startDate = request.value("startDate").toString();
    QString endDate = request.value("endDate").toString();
    
    if (startDate.isEmpty() || endDate.isEmpty()) {
        // 如果没有提供日期，默认查询最近30天
        QDate end = QDate::currentDate();
        QDate start = end.addDays(-30);
        startDate = start.toString("yyyy-MM-dd");
        endDate = end.toString("yyyy-MM-dd");
    }
    
    // 从数据库获取库存报表数据
    QJsonArray data = Database::getInstance().getSellerInventoryReport(sellerId, startDate, endDate);
    
    resp["success"] = true;
    resp["data"] = data;
    resp["startDate"] = startDate;
    resp["endDate"] = endDate;
#else
    // 使用内存存储（原有逻辑）
    resp["success"] = true;
    QJsonArray arr;
    QJsonObject item;
    item["category"] = "图书";
    item["count"] = 1200;
    arr.append(item);
    resp["data"] = arr;
#endif
    return resp;
}

QJsonObject TcpFileTask::handleSellerGetReportMember(const QJsonObject &request)
{
    QJsonObject resp;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        resp["success"] = false;
        resp["message"] = "数据库未连接";
        return resp;
    }
    
    // 获取卖家ID
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty()) {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                resp["success"] = false;
                resp["message"] = "无效的卖家ID";
                return resp;
            }
        }
    }
    
    if (sellerId <= 0) {
        resp["success"] = false;
        resp["message"] = "卖家ID不能为空";
        return resp;
    }
    
    // 获取日期范围
    QString startDate = request.value("startDate").toString();
    QString endDate = request.value("endDate").toString();
    
    if (startDate.isEmpty() || endDate.isEmpty()) {
        // 如果没有提供日期，默认查询最近30天
        QDate end = QDate::currentDate();
        QDate start = end.addDays(-30);
        startDate = start.toString("yyyy-MM-dd");
        endDate = end.toString("yyyy-MM-dd");
    }
    
    // 从数据库获取会员报表数据
    QJsonArray data = Database::getInstance().getSellerMemberReport(sellerId, startDate, endDate);
    
    resp["success"] = true;
    resp["data"] = data;
    resp["startDate"] = startDate;
    resp["endDate"] = endDate;
#else
    // 使用内存存储（原有逻辑）
    resp["success"] = true;
    QJsonArray arr;
    QJsonObject item;
    item["level"] = "钻石";
    item["count"] = 23;
    arr.append(item);
    resp["data"] = arr;
#endif
    return resp;
}

// 发送JSON格式的响应（长度前缀协议）
void TcpFileTask::sendJsonResponse(QTcpSocket &socket, const QJsonObject &response)
{
    if (socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QJsonDocument doc(response);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QByteArray frame;
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << (quint32)payload.size();
    frame.append(payload);

    socket.write(frame);
    socket.flush();
}

// 处理登录请求
QJsonObject TcpFileTask::handleLogin(const QJsonObject &request)
{
    QString username = request.value("username").toString();
    QString password = request.value("password").toString();
    QString userType = request.value("userType").toString("buyer");  // buyer或seller

    // #region agent log
    writeDebugLog("tcpserver.cpp:738", "登录请求处理入口", QJsonObject{{"username", username}, {"passwordLength", password.length()}, {"userType", userType}}, "E");
    // #endregion

    QJsonObject response;
    
#if USE_DATABASE
    // 使用MySQL数据库
    // #region agent log
    writeDebugLog("tcpserver.cpp:748", "检查数据库连接状态", QJsonObject{{"isConnected", Database::getInstance().isConnected()}}, "E");
    // #endregion
    if (!Database::getInstance().isConnected()) {
        // #region agent log
        writeDebugLog("tcpserver.cpp:750", "数据库未连接", QJsonObject{}, "E");
        // #endregion
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    qDebug() << "========================================";
    qDebug() << "数据库登录 - 用户名:" << username << "用户类型:" << userType;
    
    // 根据请求中的userType决定优先尝试哪种登录
    if (userType == "seller") {
        // 商家登录：只尝试商家登录，不尝试买家登录（因为客户端明确指定了seller）
        // #region agent log
        writeDebugLog("tcpserver.cpp:758", "尝试商家登录", QJsonObject{{"username", username}}, "E");
        // #endregion
        response = Database::getInstance().loginSeller(username, password);
        
        // #region agent log
        writeDebugLog("tcpserver.cpp:761", "商家登录结果", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}}, "E");
        // #endregion
        
        // 如果商家登录失败，直接返回失败（不尝试买家登录，因为客户端明确要求商家登录）
        if (!response["success"].toBool()) {
            qDebug() << "✗ 商家登录失败，不尝试买家登录（因为请求明确指定了userType=seller）";
        }
    } else {
        // 买家登录：优先尝试买家登录
    // #region agent log
    writeDebugLog("tcpserver.cpp:758", "尝试买家登录", QJsonObject{{"username", username}}, "E");
    // #endregion
    response = Database::getInstance().loginUser(username, password);
    
    // #region agent log
    writeDebugLog("tcpserver.cpp:761", "买家登录结果", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}}, "E");
    // #endregion
    
        // 如果买家登录失败，尝试商家登录（向后兼容）
    if (!response["success"].toBool()) {
        // #region agent log
        writeDebugLog("tcpserver.cpp:763", "买家登录失败，尝试商家登录", QJsonObject{{"username", username}}, "E");
        // #endregion
        response = Database::getInstance().loginSeller(username, password);
        // #region agent log
        writeDebugLog("tcpserver.cpp:764", "商家登录结果", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}}, "E");
        // #endregion
        }
    }
    
    if (!response["success"].toBool()) {
        response["message"] = "用户名或密码错误";
        qDebug() << "✗ 登录失败";
        // #region agent log
        writeDebugLog("tcpserver.cpp:768", "登录最终失败", QJsonObject{{"username", username}}, "E");
        // #endregion
        // 登录失败，清除保存的用户信息
        m_currentSellerId = -1;
        m_currentUserType = "";
    } else {
        // #region agent log
        writeDebugLog("tcpserver.cpp:770", "登录最终成功", QJsonObject{{"username", username}, {"userId", response.value("userId").toInt()}}, "E");
        // #endregion
        // 登录成功，保存用户信息
        QString userType = response.value("userType").toString();
        m_currentUserType = userType;
        if (userType == "seller") {
            m_currentSellerId = response.value("userId").toInt();
            qDebug() << "✓ 已保存当前登录商家ID:" << m_currentSellerId;
        } else {
            m_currentSellerId = -1;
        }
    }
    
    qDebug() << "========================================";
    return response;
#else
    // 使用内存存储（原有逻辑）
    
    // 确保用户数据已初始化
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    qDebug() << "========================================";
    qDebug() << "收到登录请求";
    qDebug() << "用户名: [" << username << "]";
    qDebug() << "密码: [" << password << "]";
    qDebug() << "当前买家用户总数:" << g_adminUsers.size();
    qDebug() << "当前商家用户总数:" << g_adminSellers.size();
    
    // 打印所有买家用户（用于调试）
    qDebug() << "--- 买家用户列表 ---";
    for (int i = 0; i < g_adminUsers.size(); ++i) {
        QString storedUsername = g_adminUsers[i]["username"].toString();
        QString storedPassword = g_adminUsers[i]["password"].toString();
        qDebug() << "  用户" << i << ": [" << storedUsername << "] 密码: [" << storedPassword << "]"
                 << "匹配:" << (storedUsername == username && storedPassword == password);
    }
    
    bool found = false;
    
    // 先尝试在买家列表中查找
    for (const QJsonObject &user : g_adminUsers) {
        QString storedUsername = user["username"].toString().trimmed();
        QString storedPassword = user["password"].toString().trimmed();
        QString inputUsername = username.trimmed();
        QString inputPassword = password.trimmed();
        
        qDebug() << "比较: 存储[" << storedUsername << "] vs 输入[" << inputUsername << "]";
        
        if (storedUsername == inputUsername && storedPassword == inputPassword) {
            response["success"] = true;
            response["message"] = "登录成功";
            response["userId"] = user["userId"].toInt();
            response["username"] = username;
            response["email"] = user["email"].toString();
            response["balance"] = user["balance"].toDouble();
            response["userType"] = "buyer";
            found = true;
            
            // 保存用户信息
            m_currentUserType = "buyer";
            m_currentSellerId = -1;
            
            qDebug() << "✓ 买家登录成功:" << username << "ID:" << user["userId"].toInt();
            break;
        }
    }
    
    // 如果买家中没找到，尝试在商家列表中查找
    if (!found) {
        qDebug() << "--- 商家用户列表 ---";
        for (int i = 0; i < g_adminSellers.size(); ++i) {
            QString storedName = g_adminSellers[i]["sellerName"].toString();
            QString storedPassword = g_adminSellers[i]["password"].toString();
            qDebug() << "  商家" << i << ": [" << storedName << "] 密码: [" << storedPassword << "]"
                     << "匹配:" << (storedName == username && storedPassword == password);
        }
        
        for (const QJsonObject &seller : g_adminSellers) {
            QString storedName = seller["sellerName"].toString().trimmed();
            QString storedPassword = seller["password"].toString().trimmed();
            QString inputUsername = username.trimmed();
            QString inputPassword = password.trimmed();
            
            if (storedName == inputUsername && storedPassword == inputPassword) {
                response["success"] = true;
                response["message"] = "登录成功";
                response["userId"] = seller["sellerId"].toInt();
                response["username"] = username;
                response["email"] = seller["email"].toString();
                response["userType"] = "seller";
                found = true;
                
                // 保存商家信息
                m_currentUserType = "seller";
                m_currentSellerId = seller["sellerId"].toInt();
                qDebug() << "✓ 商家登录成功:" << username << "ID:" << seller["sellerId"].toInt();
                qDebug() << "✓ 已保存当前登录商家ID:" << m_currentSellerId;
                break;
            }
        }
    }
    
    if (!found) {
        response["success"] = false;
        response["message"] = "用户名或密码错误";
        qDebug() << "✗ 登录失败 - 用户名或密码不匹配";
        // 登录失败，清除保存的用户信息
        m_currentSellerId = -1;
        m_currentUserType = "";
    }
    
    qDebug() << "========================================";

    return response;
#endif
}

// 处理修改密码请求
QJsonObject TcpFileTask::handleChangePassword(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString oldPassword = request.value("oldPassword").toString();
    QString newPassword = request.value("newPassword").toString();

    QJsonObject response;

    if (userId.isEmpty() || oldPassword.isEmpty() || newPassword.isEmpty()) {
        response["success"] = false;
        response["message"] = "参数不完整";
        return response;
    }

    if (newPassword.length() < 6) {
        response["success"] = false;
        response["message"] = "新密码长度至少为6位";
        return response;
    }

#if USE_DATABASE
    // 使用MySQL数据库
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }

    if (Database::getInstance().changePassword(userId.toInt(), oldPassword, newPassword)) {
        response["success"] = true;
        response["message"] = "密码修改成功";
    } else {
        response["success"] = false;
        response["message"] = "密码修改失败，请检查当前密码是否正确";
    }
#else
    // 不使用数据库的情况（临时处理）
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif

    return response;
}

// 处理更新用户信息请求
QJsonObject TcpFileTask::handleUpdateUserInfo(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString phone = request.value("phone").toString();
    QString email = request.value("email").toString();
    QString address = request.value("address").toString();

    QJsonObject response;

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }

#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }

    bool success = Database::getInstance().updateUserInfo(userId.toInt(), phone, email, address);
    if (success) {
        response["success"] = true;
        response["message"] = "用户信息更新成功";
        qDebug() << "用户信息更新成功 - 用户ID:" << userId << "电话:" << phone << "地址:" << address;
    } else {
        response["success"] = false;
        response["message"] = "用户信息更新失败，请检查日志";
        qWarning() << "用户信息更新失败 - 用户ID:" << userId << "电话:" << phone << "地址:" << address;
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif

    return response;
}

// 获取卖家个人信息
QJsonObject TcpFileTask::handleSellerGetProfile(const QJsonObject &request)
{
    QJsonObject response;
    
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                response["success"] = false;
                response["message"] = "商家ID无效";
                return response;
            }
        }
    }
    
    if (sellerId <= 0) {
        response["success"] = false;
        response["message"] = "请先登录商家账号";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        // 直接获取单个商家信息，避免查询所有商家
        QJsonObject seller = Database::getInstance().getSellerById(sellerId);
        
        if (!seller.isEmpty()) {
            response["success"] = true;
            response["message"] = "获取个人信息成功";
            response["sellerId"] = seller.value("sellerId").toInt();
            response["sellerName"] = seller.value("sellerName").toString();
            response["email"] = seller.value("email").toString();
            response["phoneNumber"] = seller.value("phoneNumber").toString();
            response["address"] = seller.value("address").toString();
            response["balance"] = seller.value("balance").toDouble();
            response["registerDate"] = seller.value("registerDate").toString();
            response["status"] = seller.value("status").toString();
            
            // 添加会员等级和积分信息
            response["memberLevel"] = seller.value("memberLevel").toString();
            response["totalRecharge"] = seller.value("totalRecharge").toDouble();
            response["points"] = seller.value("points").toInt();
            response["memberDiscount"] = seller.value("memberDiscount").toDouble();
            response["canParticipateLottery"] = seller.value("canParticipateLottery").toBool();
        } else {
            response["success"] = false;
            response["message"] = "未找到商家信息";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 卖家提交申诉
QJsonObject TcpFileTask::handleSellerSubmitAppeal(const QJsonObject &request)
{
    QJsonObject response;
    
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                response["success"] = false;
                response["message"] = "商家ID无效";
                return response;
            }
        }
    }
    
    if (sellerId <= 0) {
        response["success"] = false;
        response["message"] = "请先登录商家账号";
        return response;
    }
    
    QString appealReason = request.value("appealReason").toString();
    if (appealReason.isEmpty()) {
        response["success"] = false;
        response["message"] = "申诉理由不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        // 获取商家名称
        QJsonArray sellersArray = Database::getInstance().getAllSellers();
        QString sellerName;
        for (const QJsonValue &sellerVal : sellersArray) {
            QJsonObject seller = sellerVal.toObject();
            if (seller.value("sellerId").toInt() == sellerId) {
                sellerName = seller.value("sellerName").toString();
                break;
            }
        }
        
        if (sellerName.isEmpty()) {
            response["success"] = false;
            response["message"] = "未找到商家信息";
            return response;
        }
        
        if (Database::getInstance().submitSellerAppeal(sellerId, sellerName, appealReason)) {
            response["success"] = true;
            response["message"] = "申诉提交成功，等待管理员审核";
        } else {
            response["success"] = false;
            response["message"] = "提交申诉失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 卖家查询申诉状态
QJsonObject TcpFileTask::handleSellerGetAppeal(const QJsonObject &request)
{
    QJsonObject response;
    
    int sellerId = -1;
    if (m_currentSellerId > 0 && m_currentUserType == "seller") {
        sellerId = m_currentSellerId;
    } else {
        QString sellerIdStr = request.value("sellerId").toString();
        if (!sellerIdStr.isEmpty() && sellerIdStr != "0" && sellerIdStr != "-1") {
            bool ok;
            sellerId = sellerIdStr.toInt(&ok);
            if (!ok || sellerId <= 0) {
                response["success"] = false;
                response["message"] = "商家ID无效";
                return response;
            }
        }
    }
    
    if (sellerId <= 0) {
        response["success"] = false;
        response["message"] = "请先登录商家账号";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        QJsonObject appeal = Database::getInstance().getSellerAppeal(sellerId);
        if (!appeal.isEmpty()) {
            response["success"] = true;
            response["message"] = "获取申诉信息成功";
            response["appeal"] = appeal;
        } else {
            response["success"] = true;
            response["message"] = "暂无申诉记录";
            response["appeal"] = QJsonObject();
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 管理员获取所有申诉
QJsonObject TcpFileTask::handleAdminGetAllAppeals(const QJsonObject &request)
{
    QJsonObject response;
    
    QString status = request.value("status").toString();  // 可选：筛选状态
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        QJsonArray appeals = Database::getInstance().getAllAppeals(status);
        response["success"] = true;
        response["message"] = "获取申诉列表成功";
        response["appeals"] = appeals;
        response["total"] = appeals.size();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 管理员审核申诉
QJsonObject TcpFileTask::handleAdminReviewAppeal(const QJsonObject &request)
{
    QJsonObject response;
    
    int appealId = request.value("appealId").toInt();
    int reviewerId = request.value("reviewerId").toInt();
    QString status = request.value("status").toString();  // "已通过" 或 "未通过"
    QString reviewComment = request.value("reviewComment").toString();
    
    if (appealId <= 0) {
        response["success"] = false;
        response["message"] = "申诉ID无效";
        return response;
    }
    
    if (status != "已通过" && status != "未通过") {
        response["success"] = false;
        response["message"] = "审核状态无效，必须是'已通过'或'未通过'";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().reviewSellerAppeal(appealId, reviewerId, status, reviewComment)) {
            response["success"] = true;
            response["message"] = status == "已通过" ? "申诉已通过，卖家已自动解封" : "申诉已拒绝";
        } else {
            response["success"] = false;
            response["message"] = "审核申诉失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 处理注册请求
QJsonObject TcpFileTask::handleRegister(const QJsonObject &request)
{
    QString username = request.value("username").toString();
    QString password = request.value("password").toString();
    QString email = request.value("email").toString("");

    // #region agent log
    writeDebugLog("tcpserver.cpp:865", "注册请求处理入口", QJsonObject{{"username", username}, {"passwordLength", password.length()}, {"email", email}}, "F");
    // #endregion

    QJsonObject response;
    
    if (username.isEmpty() || password.isEmpty()) {
        // #region agent log
        writeDebugLog("tcpserver.cpp:873", "注册请求验证失败", QJsonObject{{"reason", "用户名或密码为空"}}, "F");
        // #endregion
        response["success"] = false;
        response["message"] = "用户名和密码不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 使用MySQL数据库
    // #region agent log
    writeDebugLog("tcpserver.cpp:881", "检查数据库连接状态", QJsonObject{{"isConnected", Database::getInstance().isConnected()}}, "F");
    // #endregion
    if (!Database::getInstance().isConnected()) {
        // #region agent log
        writeDebugLog("tcpserver.cpp:883", "数据库未连接", QJsonObject{}, "F");
        // #endregion
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // #region agent log
    writeDebugLog("tcpserver.cpp:887", "调用数据库注册函数前", QJsonObject{{"username", username}, {"email", email}}, "F");
    // #endregion
    if (Database::getInstance().registerUser(username, password, email)) {
        // #region agent log
        writeDebugLog("tcpserver.cpp:889", "数据库注册成功", QJsonObject{{"username", username}}, "F");
        // #endregion
        response["success"] = true;
        response["message"] = "注册成功";
        response["username"] = username;
        response["email"] = email.isEmpty() ? QString("%1@example.com").arg(username) : email;
    } else {
        // #region agent log
        writeDebugLog("tcpserver.cpp:895", "数据库注册失败", QJsonObject{{"username", username}}, "F");
        // #endregion
        response["success"] = false;
        response["message"] = "注册失败，用户名可能已存在";
    }
    
    // #region agent log
    writeDebugLog("tcpserver.cpp:898", "注册请求处理完成", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}}, "F");
    // #endregion
    
    return response;
#else
    // 使用内存存储（原有逻辑）
    
    // 确保用户数据已初始化
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    // 检查用户名是否已存在
    for (const QJsonObject &user : g_adminUsers) {
        if (user["username"].toString() == username) {
            response["success"] = false;
            response["message"] = "用户名已存在";
            qDebug() << "注册失败 - 用户名已存在:" << username;
            return response;
        }
    }
    
    // 生成新用户ID（使用当前最大ID+1）
    int newUserId = 1001;  // 起始ID
    for (const QJsonObject &user : g_adminUsers) {
        int userId = user["userId"].toInt();
        if (userId >= newUserId) {
            newUserId = userId + 1;
        }
    }
    
    // 创建新用户
    QJsonObject newUser;
    newUser["userId"] = newUserId;
    newUser["username"] = username;
    newUser["password"] = password;
    newUser["email"] = email.isEmpty() ? QString("%1@example.com").arg(username) : email;
    newUser["registerDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    newUser["status"] = "正常";
    newUser["balance"] = 0.0;  // 初始余额为0
    
    // 添加到用户列表
    g_adminUsers.append(newUser);
    
    qDebug() << "========================================";
    qDebug() << "用户注册成功:" << username << "ID:" << newUserId;
    qDebug() << "密码:" << password;
    qDebug() << "当前用户总数:" << g_adminUsers.size();
    qDebug() << "所有用户列表:";
    for (int i = 0; i < g_adminUsers.size(); ++i) {
        qDebug() << "  用户" << i << ":" << g_adminUsers[i]["username"].toString() 
                 << "密码:" << g_adminUsers[i]["password"].toString()
                 << "ID:" << g_adminUsers[i]["userId"].toInt();
    }
    qDebug() << "========================================";
    
    response["success"] = true;
    response["message"] = "注册成功";
    response["userId"] = newUserId;
    response["username"] = username;
    response["email"] = newUser["email"].toString();
    response["balance"] = 0.0;

    return response;
#endif
}

// 处理获取图书列表请求
QJsonObject TcpFileTask::handleGetAllBooks(const QJsonObject &request)
{
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库获取图书
    if (Database::getInstance().isConnected()) {
        QJsonArray allBooks = Database::getInstance().getAllBooks();
        QJsonArray booksArray;
        
        // 只返回状态为"正常"的图书
        for (const QJsonValue &bookVal : allBooks) {
            QJsonObject dbBook = bookVal.toObject();
            QString status = dbBook["status"].toString();
            
            // 只返回正常状态的图书
            if (status == "正常" || status.isEmpty()) {
                QJsonObject bookObj;
                // 将数据库字段映射到买家需要的字段
                bookObj["bookId"] = dbBook["isbn"].toString();
                bookObj["bookName"] = dbBook["title"].toString();
                bookObj["category1"] = dbBook["category1"].toString();
                bookObj["category2"] = dbBook["category2"].toString();
                bookObj["merchantId"] = dbBook["merchantId"].toInt();
                // 兼容旧数据：同时提供category字段
                bookObj["category"] = dbBook["category1"].toString();
                bookObj["subCategory"] = dbBook["category2"].toString();
                bookObj["price"] = dbBook["price"].toDouble();
                // 使用从数据库获取的averageRating，如果没有则使用0.0
                if (dbBook.contains("averageRating")) {
                    bookObj["averageRating"] = dbBook["averageRating"].toDouble();
                    bookObj["score"] = dbBook["averageRating"].toDouble();  // 兼容score字段
                } else if (dbBook.contains("score")) {
                    bookObj["score"] = dbBook["score"].toDouble();
                    bookObj["averageRating"] = dbBook["score"].toDouble();
                } else {
                    bookObj["averageRating"] = 0.0;
                    bookObj["score"] = 0.0;  // 无评分
                }
                bookObj["sales"] = 0;    // 销量暂时为0
                bookObj["stock"] = dbBook["stock"].toInt();
                bookObj["author"] = dbBook["author"].toString();
                bookObj["coverImage"] = dbBook["coverImage"].toString();  // 封面图片
                bookObj["description"] = dbBook["description"].toString();  // 书籍描述
                // 添加收藏量
                if (dbBook.contains("favoriteCount")) {
                    bookObj["favoriteCount"] = dbBook["favoriteCount"].toInt();
                } else {
                    bookObj["favoriteCount"] = 0;
                }
                booksArray.append(bookObj);
            }
        }
        
        response["success"] = true;
        response["message"] = "获取图书列表成功";
        response["books"] = booksArray;
        response["total"] = booksArray.size();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["books"] = QJsonArray();
        response["total"] = 0;
    }
#else
    // 使用内存存储（原有逻辑）
    // 买家获取卖家上架的图书
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    response["success"] = true;
    response["message"] = "获取图书列表成功";

    QJsonArray booksArray;
    // 遍历卖家上架的图书，转换为买家需要的格式
    for (const QJsonObject &sellerBook : g_sellerBooks) {
        QJsonObject bookObj;
        // 将卖家的字段映射到买家的字段
        bookObj["bookId"] = sellerBook["isbn"].toString();      // isbn -> bookId
        bookObj["bookName"] = sellerBook["title"].toString();   // title -> bookName
        bookObj["category1"] = sellerBook.contains("category1") ? sellerBook["category1"].toString() : 
                                (sellerBook.contains("category") ? sellerBook["category"].toString() : "");
        bookObj["category2"] = sellerBook.contains("category2") ? sellerBook["category2"].toString() : "";
        bookObj["merchantId"] = sellerBook.contains("merchantId") ? sellerBook["merchantId"].toInt() : 
                                (sellerBook.contains("merchant_id") ? sellerBook["merchant_id"].toInt() : 0);
        // 兼容旧数据：同时提供category字段
        bookObj["category"] = bookObj["category1"].toString();
        bookObj["subCategory"] = bookObj["category2"].toString();
        bookObj["price"] = sellerBook["price"].toDouble();
        // 使用从数据库获取的averageRating，如果没有则使用0.0
        if (sellerBook.contains("averageRating")) {
            bookObj["averageRating"] = sellerBook["averageRating"].toDouble();
            bookObj["score"] = sellerBook["averageRating"].toDouble();  // 兼容score字段
        } else if (sellerBook.contains("score")) {
            bookObj["score"] = sellerBook["score"].toDouble();
            bookObj["averageRating"] = sellerBook["score"].toDouble();
        } else {
            bookObj["averageRating"] = 0.0;
            bookObj["score"] = 0.0;  // 无评分
        }
        bookObj["sales"] = 0;    // 销量暂时为0
        bookObj["stock"] = sellerBook["stock"].toInt();
        bookObj["author"] = sellerBook["author"].toString();
        bookObj["coverImage"] = sellerBook.contains("coverImage") ? sellerBook["coverImage"].toString() : 
                                  (sellerBook.contains("cover_image") ? sellerBook["cover_image"].toString() : QString());
        booksArray.append(bookObj);
    }

    response["books"] = booksArray;
    response["total"] = booksArray.size();
#endif

    return response;
}

// 处理获取图书详情请求
QJsonObject TcpFileTask::handleGetBook(const QJsonObject &request)
{
    QString bookId = request.value("bookId").toString();
    
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库获取图书详情
    if (Database::getInstance().isConnected()) {
        QJsonObject dbBook = Database::getInstance().getBook(bookId);
        
        if (!dbBook.isEmpty() && dbBook.contains("isbn")) {
            QString status = dbBook["status"].toString();
            
            // 只返回正常状态的图书
            if (status == "正常" || status.isEmpty()) {
                response["success"] = true;
                response["message"] = "获取图书详情成功";
                response["bookId"] = dbBook["isbn"].toString();
                response["bookName"] = dbBook["title"].toString();
                response["category1"] = dbBook["category1"].toString();
                response["category2"] = dbBook["category2"].toString();
                response["merchantId"] = dbBook["merchantId"].toInt();
                // 兼容旧数据：同时提供category字段
                response["category"] = dbBook["category1"].toString();
                response["subCategory"] = dbBook["category2"].toString();
                response["price"] = dbBook["price"].toDouble();
                // 使用从数据库获取的averageRating，如果没有则使用0.0
                if (dbBook.contains("averageRating")) {
                    response["averageRating"] = dbBook["averageRating"].toDouble();
                    response["score"] = dbBook["averageRating"].toDouble();  // 兼容score字段
                } else if (dbBook.contains("score")) {
                    response["score"] = dbBook["score"].toDouble();
                    response["averageRating"] = dbBook["score"].toDouble();
                } else {
                    response["averageRating"] = 0.0;
                    response["score"] = 0.0;  // 无评分
                }
                response["sales"] = 0;
                response["stock"] = dbBook["stock"].toInt();
                response["author"] = dbBook["author"].toString();
                response["coverImage"] = dbBook["coverImage"].toString();  // 封面图片
                response["description"] = dbBook["description"].toString();  // 书籍描述
            } else {
                response["success"] = false;
                response["message"] = "图书已下架或不存在";
            }
        } else {
            response["success"] = false;
            response["message"] = "未找到指定的图书";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 使用内存存储（原有逻辑）
    // 从卖家上架的图书中查找
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    // 查找图书（bookId对应卖家的isbn）
    bool found = false;
    for (const QJsonObject &sellerBook : g_sellerBooks) {
        if (sellerBook["isbn"].toString() == bookId) {
            response["success"] = true;
            response["message"] = "获取图书详情成功";
            response["bookId"] = sellerBook["isbn"].toString();
            response["bookName"] = sellerBook["title"].toString();
            response["category1"] = sellerBook.contains("category1") ? sellerBook["category1"].toString() : 
                                     (sellerBook.contains("category") ? sellerBook["category"].toString() : "");
            response["category2"] = sellerBook.contains("category2") ? sellerBook["category2"].toString() : "";
            response["merchantId"] = sellerBook.contains("merchantId") ? sellerBook["merchantId"].toInt() : 
                                      (sellerBook.contains("merchant_id") ? sellerBook["merchant_id"].toInt() : 0);
            // 兼容旧数据：同时提供category字段
            response["category"] = response["category1"].toString();
            response["subCategory"] = response["category2"].toString();
            response["price"] = sellerBook["price"].toDouble();
            // 使用从数据库获取的averageRating，如果没有则使用0.0
            if (sellerBook.contains("averageRating")) {
                response["averageRating"] = sellerBook["averageRating"].toDouble();
                response["score"] = sellerBook["averageRating"].toDouble();  // 兼容score字段
            } else if (sellerBook.contains("score")) {
                response["score"] = sellerBook["score"].toDouble();
                response["averageRating"] = sellerBook["score"].toDouble();
            } else {
                response["averageRating"] = 0.0;
                response["score"] = 0.0;  // 无评分
            }
            response["sales"] = 0;
            response["stock"] = sellerBook["stock"].toInt();
            response["author"] = sellerBook["author"].toString();
            response["coverImage"] = sellerBook.contains("coverImage") ? sellerBook["coverImage"].toString() : 
                                      (sellerBook.contains("cover_image") ? sellerBook["cover_image"].toString() : QString());
            found = true;
            break;
        }
    }

    if (!found) {
        response["success"] = false;
        response["message"] = "未找到指定的图书";
    }
#endif

    return response;
}

// 处理搜索图书请求
QJsonObject TcpFileTask::handleSearchBooks(const QJsonObject &request)
{
    QString keyword = request.value("keyword").toString();
    
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库搜索图书
    if (Database::getInstance().isConnected()) {
        QJsonArray searchResults = Database::getInstance().searchBooks(keyword);
        QJsonArray booksArray;
        
        // 转换为买家需要的格式，只返回状态为"正常"的图书
        for (const QJsonValue &bookVal : searchResults) {
            QJsonObject dbBook = bookVal.toObject();
            QString status = dbBook["status"].toString();
            
            // 只返回正常状态的图书
            if (status == "正常" || status.isEmpty()) {
                QJsonObject bookObj;
                bookObj["bookId"] = dbBook["isbn"].toString();
                bookObj["bookName"] = dbBook["title"].toString();
                bookObj["category1"] = dbBook["category1"].toString();
                bookObj["category2"] = dbBook["category2"].toString();
                bookObj["merchantId"] = dbBook["merchantId"].toInt();
                // 兼容旧数据：同时提供category字段
                bookObj["category"] = dbBook["category1"].toString();
                bookObj["subCategory"] = dbBook["category2"].toString();
                bookObj["price"] = dbBook["price"].toDouble();
                // 使用从数据库获取的averageRating，如果没有则使用0.0
                if (dbBook.contains("averageRating")) {
                    bookObj["averageRating"] = dbBook["averageRating"].toDouble();
                    bookObj["score"] = dbBook["averageRating"].toDouble();  // 兼容score字段
                } else if (dbBook.contains("score")) {
                    bookObj["score"] = dbBook["score"].toDouble();
                    bookObj["averageRating"] = dbBook["score"].toDouble();
                } else {
                    bookObj["averageRating"] = 0.0;
                    bookObj["score"] = 0.0;  // 无评分
                }
                bookObj["sales"] = 0;
                bookObj["stock"] = dbBook["stock"].toInt();
                bookObj["author"] = dbBook["author"].toString();
                bookObj["coverImage"] = dbBook["coverImage"].toString();  // 封面图片
                bookObj["description"] = dbBook["description"].toString();  // 书籍描述
                // 添加收藏量
                if (dbBook.contains("favoriteCount")) {
                    bookObj["favoriteCount"] = dbBook["favoriteCount"].toInt();
                } else {
                    bookObj["favoriteCount"] = 0;
                }
                booksArray.append(bookObj);
            }
        }
        
        response["success"] = true;
        response["books"] = booksArray;
        response["total"] = booksArray.size();
        response["message"] = QString("找到 %1 本相关图书").arg(booksArray.size());
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["books"] = QJsonArray();
        response["total"] = 0;
    }
#else
    // 使用内存存储（原有逻辑）
    // 从卖家上架的图书中搜索
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);

    response["success"] = true;

    QJsonArray booksArray;
    QString keywordLower = keyword.toLower();
    for (const QJsonObject &sellerBook : g_sellerBooks) {
        QString title = sellerBook["title"].toString();
        QString category1 = sellerBook.contains("category1") ? sellerBook["category1"].toString() : 
                            (sellerBook.contains("category") ? sellerBook["category"].toString() : "");
        QString category2 = sellerBook.contains("category2") ? sellerBook["category2"].toString() : "";
        QString author = sellerBook["author"].toString();
        
        // 在标题、分类、作者中搜索关键词
        if (title.toLower().contains(keywordLower) || 
            category1.toLower().contains(keywordLower) ||
            category2.toLower().contains(keywordLower) ||
            author.toLower().contains(keywordLower)) {
            QJsonObject bookObj;
            bookObj["bookId"] = sellerBook["isbn"].toString();
            bookObj["bookName"] = title;
            bookObj["category1"] = category1;
            bookObj["category2"] = category2;
            bookObj["merchantId"] = sellerBook.contains("merchantId") ? sellerBook["merchantId"].toInt() : 
                                    (sellerBook.contains("merchant_id") ? sellerBook["merchant_id"].toInt() : 0);
            // 兼容旧数据：同时提供category字段
            bookObj["category"] = category1;
            bookObj["subCategory"] = category2;
            bookObj["price"] = sellerBook["price"].toDouble();
            // 使用从数据库获取的averageRating，如果没有则使用0.0
            if (sellerBook.contains("averageRating")) {
                bookObj["averageRating"] = sellerBook["averageRating"].toDouble();
                bookObj["score"] = sellerBook["averageRating"].toDouble();  // 兼容score字段
            } else if (sellerBook.contains("score")) {
                bookObj["score"] = sellerBook["score"].toDouble();
                bookObj["averageRating"] = sellerBook["score"].toDouble();
            } else {
                bookObj["averageRating"] = 0.0;
                bookObj["score"] = 0.0;  // 无评分
            }
            bookObj["sales"] = 0;
            bookObj["stock"] = sellerBook["stock"].toInt();
            bookObj["author"] = author;
            bookObj["coverImage"] = sellerBook.contains("coverImage") ? sellerBook["coverImage"].toString() : 
                                    (sellerBook.contains("cover_image") ? sellerBook["cover_image"].toString() : QString());
            booksArray.append(bookObj);
        }
    }

    response["books"] = booksArray;
    response["total"] = booksArray.size();
    response["message"] = QString("找到 %1 本相关图书").arg(booksArray.size());
#endif

    return response;
}

// 处理添加到购物车请求
QJsonObject TcpFileTask::handleAddToCart(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();
    int quantity = request.value("quantity").toInt(1);

    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和图书ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 检查用户是否被封禁
    QJsonArray usersArray = Database::getInstance().getAllUsers();
    for (const QJsonValue &userVal : usersArray) {
        QJsonObject user = userVal.toObject();
        if (user.value("userId").toInt() == userId.toInt()) {
            QString userStatus = user.value("status").toString();
            if (userStatus == "封禁") {
                response["success"] = false;
                response["message"] = "您的账户已被封禁，无法购买图书";
                return response;
            }
            break;
        }
    }
    
    // 将书籍添加到购物车（保存到数据库cart表）
    if (Database::getInstance().addToCart(userId.toInt(), bookId, quantity)) {
        response["success"] = true;
        response["message"] = "已添加到购物车";
    } else {
        response["success"] = false;
        response["message"] = "添加到购物车失败，请重试";
    }
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "已添加到购物车（内存模式）";
    response["cartItemId"] = QString("cart_%1_%2").arg(userId).arg(bookId);
#endif

    return response;
}

// 处理获取购物车请求
QJsonObject TcpFileTask::handleGetCart(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();

    QJsonObject response;
    
    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 从数据库cart表中查询该用户的购物车（每个用户只能看到自己的购物车）
    QJsonArray cartItems = Database::getInstance().getCart(userId.toInt());
    
    // 计算总价
    double total = 0.0;
    for (const QJsonValue &itemVal : cartItems) {
        QJsonObject item = itemVal.toObject();
        double price = item.value("price").toDouble();
        int quantity = item.value("quantity").toInt();
        total += price * quantity;
    }
    
    response["success"] = true;
    response["message"] = "获取购物车成功";
    response["items"] = cartItems;
    response["total"] = total;
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "获取购物车成功（内存模式）";
    response["items"] = QJsonArray();
    response["total"] = 0;
#endif

    return response;
}

// 处理更新购物车数量请求
QJsonObject TcpFileTask::handleUpdateCartQuantity(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();
    int quantity = request.value("quantity").toInt();

    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和图书ID不能为空";
        return response;
    }
    
    if (quantity <= 0) {
        response["success"] = false;
        response["message"] = "数量必须大于0";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 更新数据库cart表中的数量
    if (Database::getInstance().updateCartQuantity(userId.toInt(), bookId, quantity)) {
        response["success"] = true;
        response["message"] = "数量更新成功";
    } else {
        response["success"] = false;
        response["message"] = "数量更新失败，请重试";
    }
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "数量更新成功（内存模式）";
#endif

    return response;
}

// 处理从购物车移除书籍请求
QJsonObject TcpFileTask::handleRemoveFromCart(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();

    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和图书ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 从数据库cart表中删除对应的记录
    if (Database::getInstance().removeFromCart(userId.toInt(), bookId)) {
        response["success"] = true;
        response["message"] = "已从购物车移除";
    } else {
        response["success"] = false;
        response["message"] = "移除失败，请重试";
    }
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "已从购物车移除（内存模式）";
#endif

    return response;
}

// 处理添加到收藏请求
QJsonObject TcpFileTask::handleAddFavorite(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();

    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和图书ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 将书籍添加到收藏（保存到数据库favorites表）
    if (Database::getInstance().addFavorite(userId.toInt(), bookId)) {
        response["success"] = true;
        response["message"] = "已添加到收藏";
    } else {
        response["success"] = false;
        response["message"] = "添加到收藏失败，请重试";
    }
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "已添加到收藏（内存模式）";
#endif
    
    return response;
}

// 处理从收藏移除请求
QJsonObject TcpFileTask::handleRemoveFavorite(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();

    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和图书ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 从数据库favorites表中删除对应的记录
    if (Database::getInstance().removeFavorite(userId.toInt(), bookId)) {
        response["success"] = true;
        response["message"] = "已从收藏移除";
    } else {
        response["success"] = false;
        response["message"] = "移除失败，请重试";
    }
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "已从收藏移除（内存模式）";
#endif
    
    return response;
}

// 处理创建订单请求
QJsonObject TcpFileTask::handleCreateOrder(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QJsonArray items = request.value("items").toArray();

    QJsonObject response;
    
    if (userId.isEmpty() || items.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和订单项不能为空";
    } else {
        // 基本收货信息校验（服务器端兜底校验，防止老客户端绕过）
        QString customer = request.value("customer").toString().trimmed();
        QString phone = request.value("phone").toString().trimmed();
        QString address = request.value("address").toString().trimmed();

        if (address.isEmpty()) {
            response["success"] = false;
            response["message"] = "收货地址不能为空，请先完善收货信息";
            return response;
        }

        // 检查用户是否被封禁
#if USE_DATABASE
        if (Database::getInstance().isConnected()) {
            QJsonArray usersArray = Database::getInstance().getAllUsers();
            for (const QJsonValue &userVal : usersArray) {
                QJsonObject user = userVal.toObject();
                if (user.value("userId").toInt() == userId.toInt()) {
                    QString userStatus = user.value("status").toString();
                    if (userStatus == "封禁") {
                        response["success"] = false;
                        response["message"] = "您的账户已被封禁，无法购买图书";
                        return response;
                    }
                    break;
                }
            }
        }
#endif
        // 生成订单ID
        QString orderId = QString("ORDER_%1").arg(QDateTime::currentMSecsSinceEpoch());
        
        // 计算总金额，并确保订单项包含merchantId
        double totalAmount = 0.0;
        double originalAmount = 0.0;  // 原始金额（折扣前）
        QJsonArray enrichedItems;
        
        // 获取用户会员等级和折扣率
        QString memberLevel = "普通会员";
        double memberDiscount = 1.0;  // 默认无折扣
        
#if USE_DATABASE
        if (Database::getInstance().isConnected()) {
            // 直接通过getUserById获取用户信息以获取会员等级（更高效且准确）
            QJsonObject user = Database::getInstance().getUserById(userId.toInt());
            if (!user.isEmpty()) {
                memberLevel = user.value("memberLevel").toString();
                if (memberLevel.isEmpty()) {
                    memberLevel = "普通会员";
                }
                memberDiscount = user.value("memberDiscount").toDouble();
                if (memberDiscount <= 0 || memberDiscount > 1.0) {
                    memberDiscount = 1.0;  // 确保折扣率在有效范围内
                }
            }
            
            // 从数据库获取图书信息，补充merchantId
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                QString bookId = item["bookId"].toString();
                
                // 从数据库获取图书信息
                QJsonObject book = Database::getInstance().getBook(bookId);
                if (!book.isEmpty()) {
                    // 添加merchantId到订单项
                    item["merchantId"] = book["merchantId"].toInt();
                }
                
                double price = item["price"].toDouble();
                int quantity = item["quantity"].toInt();
                originalAmount += price * quantity;
                enrichedItems.append(item);
            }
        } else {
            // 数据库未连接，使用原始items
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                double price = item["price"].toDouble();
                int quantity = item["quantity"].toInt();
                originalAmount += price * quantity;
                enrichedItems.append(item);
            }
        }
#else
        // 不使用数据库时，使用原始items
        for (const QJsonValue &itemVal : items) {
            QJsonObject item = itemVal.toObject();
            double price = item["price"].toDouble();
            int quantity = item["quantity"].toInt();
            originalAmount += price * quantity;
            enrichedItems.append(item);
        }
#endif
        
        // 应用会员折扣
        totalAmount = originalAmount * memberDiscount;
        
        // 处理优惠券使用
        QString useCouponType = request.value("useCoupon").toString();  // "30" 或 "50" 或 ""
        double couponDiscount = 0.0;
        if (useCouponType == "30") {
            // 检查用户是否有30元优惠券
            int coupon30Count = Database::getInstance().getCoupon30Count(userId.toInt());
            if (coupon30Count > 0) {
                couponDiscount = 30.0;
                // 使用优惠券（在支付时扣除）
            } else {
                response["success"] = false;
                response["message"] = "您没有30元优惠券";
                return response;
            }
        } else if (useCouponType == "50") {
            // 检查用户是否有50元优惠券
            int coupon50Count = Database::getInstance().getCoupon50Count(userId.toInt());
            if (coupon50Count > 0) {
                couponDiscount = 50.0;
                // 使用优惠券（在支付时扣除）
            } else {
                response["success"] = false;
                response["message"] = "您没有50元优惠券";
                return response;
            }
        }
        
        // 应用优惠券折扣
        totalAmount = qMax(0.0, totalAmount - couponDiscount);
        
        // 创建订单对象
        QJsonObject order;
        order["orderId"] = orderId;
        // 确保userId是整数类型，而不是字符串
        int userIdInt = userId.toInt();
        if (userIdInt <= 0) {
            qWarning() << "handleCreateOrder: 无效的用户ID字符串:" << userId;
            response["success"] = false;
            response["message"] = "用户ID无效，请重新登录";
            return response;
        }
        order["userId"] = userIdInt;  // 存储为整数类型
        order["items"] = enrichedItems;
        order["originalAmount"] = originalAmount;  // 原始金额（折扣前）
        order["totalAmount"] = totalAmount;  // 折扣后金额（会员折扣+优惠券）
        order["memberLevel"] = memberLevel;  // 会员等级
        order["memberDiscount"] = memberDiscount;  // 折扣率
        order["couponDiscount"] = couponDiscount;  // 优惠券折扣金额
        order["useCoupon"] = useCouponType;  // 使用的优惠券类型
        order["discountAmount"] = originalAmount - totalAmount;  // 总折扣金额（会员折扣+优惠券）
        order["status"] = "待支付";
        order["orderDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        // 处理客户信息，确保不为空（使用之前已声明的变量）
        if (customer.isEmpty()) {
            customer = "客户";
        }
        order["customer"] = customer;
        order["phone"] = phone;
        order["address"] = address;
        
        order["paymentMethod"] = "";
        order["operator"] = "系统";
        order["remark"] = "";
        
        qDebug() << "准备保存订单到数据库，订单ID:" << orderId << "用户ID:" << userId << "金额:" << totalAmount;
        
#if USE_DATABASE
        // 保存到数据库（必须成功才能返回成功）
        if (Database::getInstance().isConnected()) {
            QString savedOrderId = Database::getInstance().createOrder(order);
            if (!savedOrderId.isEmpty() && savedOrderId == orderId) {
                qDebug() << "✓ 订单已成功保存到数据库:" << orderId << "用户:" << userId << "金额:" << totalAmount;
                
                // 同时添加到全局订单列表（用于向后兼容）
                QMutexLocker locker(&g_sellerOrdersMutex);
                g_sellerOrders.append(order);
                
                response["success"] = true;
                response["message"] = "订单创建成功";
                response["orderId"] = orderId;
                response["totalAmount"] = totalAmount;
                response["orderDate"] = order["orderDate"].toString();
            } else {
                qWarning() << "✗ 订单保存到数据库失败:" << orderId << "返回的订单ID:" << savedOrderId;
                response["success"] = false;
                response["message"] = "订单创建失败：无法保存到数据库，请检查服务器日志";
            }
        } else {
            qWarning() << "✗ 数据库未连接，无法创建订单";
            response["success"] = false;
            response["message"] = "订单创建失败：数据库未连接";
        }
#else
        // 不使用数据库时，使用内存存储
        QMutexLocker locker(&g_sellerOrdersMutex);
        g_sellerOrders.append(order);
        
        qDebug() << "订单创建成功（内存模式）:" << orderId << "用户:" << userId << "金额:" << totalAmount;
        
        response["success"] = true;
        response["message"] = "订单创建成功";
        response["orderId"] = orderId;
        response["totalAmount"] = totalAmount;
        response["orderDate"] = order["orderDate"].toString();
#endif
    }

    return response;
}

// 处理获取订单列表请求
QJsonObject TcpFileTask::handleGetUserOrders(const QJsonObject &request)
{
    QString userIdStr = request.value("userId").toString();

    QJsonObject response;
    
    if (userIdStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
    // 处理userId可能是字符串或数字的情况
    bool ok;
    int userId = userIdStr.toInt(&ok);
    if (!ok || userId <= 0) {
        qWarning() << "handleGetUserOrders: 无效的用户ID:" << userIdStr;
        response["success"] = false;
        response["message"] = QString("无效的用户ID: %1").arg(userIdStr);
        response["orders"] = QJsonArray();
        response["total"] = 0;
        return response;
    }
    
    qDebug() << "handleGetUserOrders: 开始获取用户订单，用户ID:" << userId;
    
#if USE_DATABASE
    // 从数据库读取订单
    if (Database::getInstance().isConnected()) {
        QJsonArray userOrders = Database::getInstance().getUserOrders(userId);
        qDebug() << "handleGetUserOrders: 查询完成，用户ID:" << userId << "找到订单数:" << userOrders.size();
        
        // 验证返回的订单数据
        if (userOrders.size() > 0) {
            qDebug() << "handleGetUserOrders: 第一个订单ID:" << userOrders[0].toObject()["orderId"].toString();
        }
        
        response["success"] = true;
        response["message"] = QString("获取订单列表成功，共%1个订单").arg(userOrders.size());
        response["orders"] = userOrders;
        response["total"] = userOrders.size();
    } else {
        qWarning() << "handleGetUserOrders: 数据库未连接";
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["orders"] = QJsonArray();
        response["total"] = 0;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    // 筛选该用户的订单
    QJsonArray userOrders;
    for (const QJsonObject &order : g_sellerOrders) {
        if (order["userId"].toString() == userId) {
            userOrders.append(order);
        }
    }
    
    response["success"] = true;
    response["message"] = "获取订单列表成功";
    response["orders"] = userOrders;
    response["total"] = userOrders.size();
#endif

    return response;
}

// 处理支付订单请求
QJsonObject TcpFileTask::handlePayOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    QString paymentMethod = request.value("paymentMethod").toString();
    QString useCoupon = request.value("useCoupon").toString();  // 获取优惠券信息

    QJsonObject response;
    
    if (orderId.isEmpty()) {
        response["success"] = false;
        response["message"] = "订单ID不能为空";
        return response;
    }
    
    // 检查用户是否被封禁（通过订单获取用户ID）
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        QJsonObject order = Database::getInstance().getOrder(orderId);
        if (!order.isEmpty() && order.contains("userId")) {
            int userId = order.value("userId").toInt();
            QJsonArray usersArray = Database::getInstance().getAllUsers();
            for (const QJsonValue &userVal : usersArray) {
                QJsonObject user = userVal.toObject();
                if (user.value("userId").toInt() == userId) {
                    QString userStatus = user.value("status").toString();
                    if (userStatus == "封禁") {
                        response["success"] = false;
                        response["message"] = "您的账户已被封禁，无法支付订单";
                        return response;
                    }
                    break;
                }
            }
        }
    }
#endif
    
#if USE_DATABASE
    // 从数据库读取订单
    if (Database::getInstance().isConnected()) {
        QJsonObject order = Database::getInstance().getOrder(orderId);
        if (order.isEmpty()) {
            response["success"] = false;
            response["message"] = "订单不存在";
            return response;
        }
        
        // 验证订单状态
        QString status = order["status"].toString();
        if (status != "待支付") {
            response["success"] = false;
            response["message"] = "订单状态不正确，无法支付";
            return response;
        }
        
        // 获取用户ID和订单金额
        int userId = 0;
        // 处理userId可能是不同JSON类型的情况
        if (order.contains("userId")) {
            if (order["userId"].isDouble()) {
                userId = order["userId"].toInt();
            } else if (order["userId"].isString()) {
                bool ok;
                userId = order["userId"].toString().toInt(&ok);
                if (!ok) {
                    qWarning() << "handlePayOrder: 用户ID字符串无法转换为整数:" << order["userId"].toString();
                    userId = 0;
                }
            } else {
                userId = order["userId"].toInt();
            }
        } else {
            qWarning() << "handlePayOrder: 订单对象中不包含userId字段";
        }
        
        double totalAmount = order["totalAmount"].toDouble();
        double originalAmount = totalAmount;  // 保存原始金额
        
        // 处理优惠券使用（如果支付时选择了优惠券）
        double couponDiscount = 0.0;
        if (useCoupon == "30") {
            // 检查用户是否有30元优惠券
            int coupon30Count = Database::getInstance().getCoupon30Count(userId);
            if (coupon30Count > 0) {
                couponDiscount = 30.0;
            } else {
                response["success"] = false;
                response["message"] = "您没有30元优惠券";
                return response;
            }
        } else if (useCoupon == "50") {
            // 检查用户是否有50元优惠券
            int coupon50Count = Database::getInstance().getCoupon50Count(userId);
            if (coupon50Count > 0) {
                couponDiscount = 50.0;
            } else {
                response["success"] = false;
                response["message"] = "您没有50元优惠券";
                return response;
            }
        }
        
        // 应用优惠券折扣
        totalAmount = qMax(0.0, totalAmount - couponDiscount);
        
        qDebug() << "handlePayOrder: 订单信息 - 订单ID:" << orderId << "用户ID:" << userId << "原始金额:" << originalAmount << "优惠券折扣:" << couponDiscount << "最终金额:" << totalAmount;
        if (order.contains("userId")) {
            qDebug() << "handlePayOrder: 订单对象中的userId类型:" << order["userId"].type() << "值:" << order["userId"].toVariant().toString();
        }
        
        // 验证用户ID是否有效
        if (userId <= 0) {
            qWarning() << "handlePayOrder: 无效的用户ID:" << userId;
            qWarning() << "handlePayOrder: 订单对象内容:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
            response["success"] = false;
            response["message"] = QString("订单中的用户ID无效（%1），请重新下单").arg(userId);
            return response;
        }
        
        // 如果是余额支付，检查并扣除余额
        if (paymentMethod == "账户余额支付" || paymentMethod.contains("余额")) {
            // 直接通过getUserById获取用户当前余额（更高效且准确）
            qDebug() << "handlePayOrder: 开始获取用户余额，用户ID:" << userId << "订单ID:" << orderId;
            QJsonObject user = Database::getInstance().getUserById(userId);
            double currentBalance = 0.0;
            
            qDebug() << "handlePayOrder: getUserById返回，用户对象是否为空:" << user.isEmpty();
            if (!user.isEmpty()) {
                qDebug() << "handlePayOrder: 用户对象包含的字段:" << user.keys();
                qDebug() << "handlePayOrder: 用户对象中的userId:" << user.value("userId").toInt();
            }
            
            // 检查用户对象是否有效
            if (user.isEmpty()) {
                qWarning() << "handlePayOrder: 获取用户信息失败，用户对象为空，用户ID:" << userId;
                response["success"] = false;
                response["message"] = QString("无法获取用户信息，用户ID: %1。请确认用户账户存在且正常").arg(userId);
                return response;
            }
            
            // 验证返回的用户ID是否匹配
            int returnedUserId = user.value("userId").toInt();
            if (returnedUserId != userId) {
                qWarning() << "handlePayOrder: 用户ID不匹配，请求ID:" << userId << "返回ID:" << returnedUserId;
                response["success"] = false;
                response["message"] = QString("用户信息不匹配，请重新登录后重试");
                return response;
            }
            
            // 检查balance字段是否存在
            if (user.contains("balance")) {
                currentBalance = user.value("balance").toDouble();
                qDebug() << "获取用户余额成功，用户ID:" << userId << "当前余额:" << currentBalance;
            } else {
                // 如果balance字段不存在，尝试使用默认值0，并记录警告
                qWarning() << "用户对象中不包含balance字段，用户ID:" << userId;
                qWarning() << "用户对象包含的字段:" << user.keys();
                // 尝试从用户对象中查找可能的余额字段
                if (user.contains("Balance")) {
                    currentBalance = user.value("Balance").toDouble();
                } else {
                    qWarning() << "无法从用户对象获取余额，使用默认值0";
                    currentBalance = 0.0;
                }
            }
            
            // 再次验证余额是否有效（防止异常值）
            if (currentBalance < 0) {
                qWarning() << "用户余额为负数，用户ID:" << userId << "余额:" << currentBalance;
                currentBalance = 0.0;
            }
            
            // 检查余额是否足够
            if (currentBalance < totalAmount) {
                response["success"] = false;
                response["message"] = QString("账户余额不足，当前余额：%1 元，需要支付：%2 元").arg(currentBalance, 0, 'f', 2).arg(totalAmount, 0, 'f', 2);
                return response;
            }
            
            // 如果支付时使用了优惠券，先标记优惠券为已使用（在扣除余额之前）
            bool couponUsed = false;
            if (useCoupon == "30") {
                if (!Database::getInstance().useCoupon30(userId, 1, orderId)) {
                    qWarning() << "使用30元优惠券失败，用户ID:" << userId << "订单ID:" << orderId;
                    response["success"] = false;
                    response["message"] = "使用30元优惠券失败，请检查优惠券数量";
                    return response;
                } else {
                    couponUsed = true;
                    qDebug() << "30元优惠券使用成功，用户ID:" << userId << "订单ID:" << orderId;
                }
            } else if (useCoupon == "50") {
                if (!Database::getInstance().useCoupon50(userId, 1, orderId)) {
                    qWarning() << "使用50元优惠券失败，用户ID:" << userId << "订单ID:" << orderId;
                    response["success"] = false;
                    response["message"] = "使用50元优惠券失败，请检查优惠券数量";
                    return response;
                } else {
                    couponUsed = true;
                    qDebug() << "50元优惠券使用成功，用户ID:" << userId << "订单ID:" << orderId;
                }
            }
            
            // 扣除余额（在优惠券使用成功后）
            if (!Database::getInstance().deductUserBalance(userId, totalAmount)) {
                // 如果扣除余额失败，需要回滚优惠券使用
                if (couponUsed) {
                    if (useCoupon == "30") {
                        Database::getInstance().rollbackCoupon30(userId, orderId);
                    } else if (useCoupon == "50") {
                        Database::getInstance().rollbackCoupon50(userId, orderId);
                    }
                }
                response["success"] = false;
                response["message"] = "扣除余额失败";
                return response;
            }
            
            // 获取扣除后的余额（直接通过getUserById获取）
            QJsonObject updatedUser = Database::getInstance().getUserById(userId);
            double newBalance = 0.0;
            if (!updatedUser.isEmpty() && updatedUser.contains("balance")) {
                newBalance = updatedUser.value("balance").toDouble();
            }
            response["balance"] = newBalance;
            qDebug() << "余额支付：用户ID:" << userId << "扣除金额:" << totalAmount << "新余额:" << newBalance;
        }
        
        // 更新数据库中的订单状态（包括支付方式和优惠券折扣后的金额）
        if (Database::getInstance().updateOrderStatus(orderId, "已支付", paymentMethod, "", "", totalAmount)) {
            // 同时更新内存中的订单（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (int i = 0; i < g_sellerOrders.size(); ++i) {
                if (g_sellerOrders[i]["orderId"].toString() == orderId) {
                    g_sellerOrders[i]["status"] = "已支付";
                    g_sellerOrders[i]["paymentMethod"] = paymentMethod;
                    g_sellerOrders[i]["payTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    // 更新订单金额为使用优惠券后的金额
                    g_sellerOrders[i]["totalAmount"] = totalAmount;
                    g_sellerOrders[i]["couponDiscount"] = couponDiscount;
                    g_sellerOrders[i]["useCoupon"] = useCoupon;
                    qDebug() << "内存订单金额已更新，订单ID:" << orderId << "新金额:" << totalAmount;
                    break;
                }
            }
            
            qDebug() << "订单支付成功:" << orderId << "支付方式:" << paymentMethod;
            
            response["success"] = true;
            response["message"] = "支付成功";
            response["orderId"] = orderId;
            response["paymentMethod"] = paymentMethod;
            response["payTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            if (!useCoupon.isEmpty()) {
                response["couponUsed"] = useCoupon;  // 返回使用的优惠券类型
            }
        } else {
            response["success"] = false;
            response["message"] = "更新订单状态失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    // 查找订单
    bool found = false;
    QString orderUserId;
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i]["orderId"].toString() == orderId) {
            QJsonObject order = g_sellerOrders[i];
            orderUserId = order["userId"].toString();
            
            // 验证订单状态
            QString status = order["status"].toString();
            if (status != "待支付") {
                response["success"] = false;
                response["message"] = "订单状态不正确，无法支付";
                return response;
            }
            
            // 更新订单状态为已支付
            order["status"] = "已支付";
            order["paymentMethod"] = paymentMethod;
            order["payTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            
            g_sellerOrders[i] = order;
            found = true;
            
            qDebug() << "订单支付成功:" << orderId << "支付方式:" << paymentMethod;
            
            response["success"] = true;
            response["message"] = "支付成功";
            response["orderId"] = orderId;
            response["paymentMethod"] = paymentMethod;
            response["payTime"] = order["payTime"].toString();
            break;
        }
    }
    
    if (!found) {
        response["success"] = false;
        response["message"] = "订单不存在";
    }
#endif

    return response;
}

// 处理充值余额请求
QJsonObject TcpFileTask::handleRechargeBalance(const QJsonObject &request)
{
    // 支持字符串和数字格式的userId
    QJsonValue userIdValue = request.value("userId");
    int userId = 0;
    
    if (userIdValue.isString()) {
        bool ok = false;
        userId = userIdValue.toString().toInt(&ok);
        if (!ok) {
            qWarning() << "充值失败：用户ID格式错误（字符串无法转换为整数），userId:" << userIdValue.toString();
            QJsonObject response;
            response["success"] = false;
            response["message"] = "用户ID格式错误";
            return response;
        }
    } else if (userIdValue.isDouble()) {
        userId = userIdValue.toInt();
    } else {
        qWarning() << "充值失败：用户ID类型错误，userId类型:" << userIdValue.type();
        QJsonObject response;
        response["success"] = false;
        response["message"] = "用户ID类型错误";
        return response;
    }
    
    double amount = request.value("amount").toDouble();
    
    qDebug() << "充值请求：用户ID:" << userId << "金额:" << amount;
    
    QJsonObject response;
    
    if (userId <= 0) {
        qWarning() << "充值失败：用户ID无效，userId:" << userId << "原始值:" << userIdValue;
        response["success"] = false;
        response["message"] = QString("用户ID无效（ID: %1）").arg(userId);
        return response;
    }
    
    if (amount <= 0) {
        response["success"] = false;
        response["message"] = "充值金额必须大于0";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        // 充值余额
        if (Database::getInstance().rechargeUserBalance(userId, amount)) {
            // 直接获取单个用户信息，避免查询所有用户
            QJsonObject user = Database::getInstance().getUserById(userId);
            
            if (!user.isEmpty()) {
                double newBalance = user.value("balance").toDouble();
                QString memberLevel = user.value("memberLevel").toString();
                if (memberLevel.isEmpty()) {
                    memberLevel = "普通会员";
                }
                double totalRecharge = user.value("totalRecharge").toDouble();
                double memberDiscount = user.value("memberDiscount").toDouble();
                int points = user.value("points").toInt();
                bool canParticipateLottery = user.value("canParticipateLottery").toBool();
                
                // 计算本次获得的积分
                int pointsEarned = Database::getInstance().calculatePoints(amount);
                
                response["success"] = true;
                response["message"] = "充值成功";
                response["amount"] = amount;
                response["balance"] = newBalance;
                response["memberLevel"] = memberLevel;
                response["totalRecharge"] = totalRecharge;
                response["memberDiscount"] = memberDiscount;
                response["points"] = points;
                response["pointsEarned"] = pointsEarned;  // 本次获得的积分
                response["canParticipateLottery"] = canParticipateLottery;
                qDebug() << "用户余额充值成功，用户ID:" << userId << "充值金额:" << amount << "新余额:" << newBalance 
                         << "会员等级:" << memberLevel << "累计充值:" << totalRecharge 
                         << "总积分:" << points << "本次获得积分:" << pointsEarned;
            } else {
                response["success"] = false;
                response["message"] = "获取用户信息失败";
            }
        } else {
            response["success"] = false;
            response["message"] = "充值失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
#endif
    
    return response;
}

// 处理抽奖请求
QJsonObject TcpFileTask::handleParticipateLottery(const QJsonObject &request)
{
    QJsonObject response;
    
    // 解析用户ID
    QJsonValue userIdValue = request.value("userId");
    int userId = 0;
    if (userIdValue.isString()) {
        userId = userIdValue.toString().toInt();
    } else if (userIdValue.isDouble()) {
        userId = userIdValue.toInt();
    }
    
    if (userId <= 0) {
        response["success"] = false;
        response["message"] = "用户ID无效";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        // 检查用户是否可以参与抽奖（累计满3积分）
        if (!Database::getInstance().canParticipateLottery(userId)) {
            int currentPoints = Database::getInstance().getUserPoints(userId);
            response["success"] = false;
            response["message"] = QString("积分不足，无法参与抽奖。当前积分：%1，需要3积分才能参与抽奖").arg(currentPoints);
            return response;
        }
        
        // 抽奖逻辑：随机生成奖品（等概率）
        // 奖品列表：30元优惠券、50元优惠券、谢谢参与
        QStringList prizes = {"30元优惠券", "50元优惠券", "谢谢参与"};
        
        // 等概率随机选择奖品（每个奖品1/3概率）
        qsrand(QTime::currentTime().msec());
        int randomIndex = qrand() % prizes.size();
        QString prize = prizes[randomIndex];
        
        // 扣除3积分（无论抽中什么都要扣除）
        int currentPoints = Database::getInstance().getUserPoints(userId);
        if (currentPoints >= 3) {
            // 使用addPoints添加负数来扣除积分
            if (!Database::getInstance().addPoints(userId, -3)) {
                response["success"] = false;
                response["message"] = "扣除积分失败";
                return response;
            }
        }
        
        // 根据抽中的奖品类型进行处理
        if (prize == "30元优惠券") {
            // 添加30元优惠券到用户账户（直接更新users表的coupon_30字段）
            if (!Database::getInstance().addCoupon30(userId, 1)) {
                qWarning() << "添加30元优惠券失败";
                response["success"] = false;
                response["message"] = "添加优惠券失败";
                return response;
            }
        } else if (prize == "50元优惠券") {
            // 添加50元优惠券到用户账户（直接更新users表的coupon_50字段）
            if (!Database::getInstance().addCoupon50(userId, 1)) {
                qWarning() << "添加50元优惠券失败";
                response["success"] = false;
                response["message"] = "添加优惠券失败";
                return response;
            }
        }
        // "谢谢参与"不需要处理
        
        // 获取更新后的优惠券数量
        int coupon30 = Database::getInstance().getCoupon30Count(userId);
        int coupon50 = Database::getInstance().getCoupon50Count(userId);
        
        response["success"] = true;
        response["message"] = "抽奖成功";
        response["prize"] = prize;
        response["remainingPoints"] = Database::getInstance().getUserPoints(userId);
        response["coupon30"] = coupon30;
        response["coupon50"] = coupon50;
        
        qDebug() << "用户ID:" << userId << "参与抽奖，获得奖品:" << prize << "剩余积分:" << response["remainingPoints"].toInt();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
#endif
    
    return response;
}

// 处理发送聊天消息请求
QJsonObject TcpFileTask::handleSendChatMessage(const QJsonObject &request)
{
    // 支持字符串和数字格式的userId
    QJsonValue senderIdValue = request.value("senderId");
    QString senderType = request.value("senderType").toString();
    int senderId = 0;
    
    // 特殊处理管理员ID：管理员ID是字符串格式（如"ADMIN001"），使用固定数字ID 999999
    if (senderType == "admin") {
        QString senderIdStr = senderIdValue.toString();
        if (!senderIdStr.isEmpty() && (senderIdStr.startsWith("ADMIN") || senderIdStr == "admin")) {
            senderId = 999999;  // 管理员固定ID
        } else {
            // 如果管理员ID是数字格式，也尝试转换
            bool ok = false;
            senderId = senderIdStr.toInt(&ok);
            if (!ok) {
                senderId = 999999;  // 默认管理员ID
            }
        }
    } else {
        // 普通用户（买家/卖家）：必须是数字ID
        if (senderIdValue.isString()) {
            bool ok = false;
            senderId = senderIdValue.toString().toInt(&ok);
            if (!ok) {
                QJsonObject response;
                response["success"] = false;
                response["message"] = "发送者ID格式错误";
                return response;
            }
        } else if (senderIdValue.isDouble()) {
            senderId = senderIdValue.toInt();
        }
    }
    
    // 处理receiverId（支持字符串和数字格式）
    QJsonValue receiverIdValue = request.value("receiverId");
    int receiverId = -1;
    if (receiverIdValue.isString()) {
        QString receiverIdStr = receiverIdValue.toString();
        if (!receiverIdStr.isEmpty()) {
            bool ok = false;
            receiverId = receiverIdStr.toInt(&ok);
            if (!ok) {
                receiverId = -1;
            }
        }
    } else if (receiverIdValue.isDouble()) {
        receiverId = receiverIdValue.toInt();
    }
    QString receiverType = request.value("receiverType").toString();
    QString message = request.value("message").toString().trimmed();
    
    QJsonObject response;
    
    // 管理员ID特殊处理：允许使用999999
    if (senderId <= 0 && senderType != "admin") {
        response["success"] = false;
        response["message"] = "发送者ID无效";
        return response;
    }
    
    // 管理员ID验证
    if (senderType == "admin" && senderId != 999999) {
        // 如果管理员ID不是999999，尝试设置为999999
        senderId = 999999;
    }
    
    if (senderType.isEmpty()) {
        response["success"] = false;
        response["message"] = "发送者类型不能为空";
        return response;
    }
    
    if (message.isEmpty()) {
        response["success"] = false;
        response["message"] = "消息内容不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().saveChatMessage(senderId, senderType, receiverId, receiverType, message)) {
            response["success"] = true;
            response["message"] = "消息发送成功";
            qDebug() << "聊天消息发送成功，发送者ID:" << senderId << "类型:" << senderType;
        } else {
            response["success"] = false;
            response["message"] = "消息发送失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
#endif
    
    return response;
}

// 处理获取聊天历史请求
QJsonObject TcpFileTask::handleGetChatHistory(const QJsonObject &request)
{
    // 支持字符串和数字格式的userId
    QJsonValue userIdValue = request.value("userId");
    QString userType = request.value("userType").toString();
    int userId = 0;
    
    // 特殊处理管理员ID：管理员ID是字符串格式（如"ADMIN001"），使用固定数字ID 999999
    if (userType == "admin") {
        QString userIdStr = userIdValue.toString();
        if (!userIdStr.isEmpty() && (userIdStr.startsWith("ADMIN") || userIdStr == "admin")) {
            userId = 999999;  // 管理员固定ID
        } else {
            // 如果管理员ID是数字格式，也尝试转换
            bool ok = false;
            userId = userIdStr.toInt(&ok);
            if (!ok) {
                userId = 999999;  // 默认管理员ID
            }
        }
    } else {
        // 普通用户（买家/卖家）：必须是数字ID
        if (userIdValue.isString()) {
            bool ok = false;
            userId = userIdValue.toString().toInt(&ok);
            if (!ok) {
                QJsonObject response;
                response["success"] = false;
                response["message"] = "用户ID格式错误";
                return response;
            }
        } else if (userIdValue.isDouble()) {
            userId = userIdValue.toInt();
        }
    }
    
    int otherUserId = request.value("otherUserId").toInt(-1);
    QString otherUserType = request.value("otherUserType").toString();
    
    QJsonObject response;
    
    // 管理员ID特殊处理：允许使用999999
    if (userId <= 0 && userType != "admin") {
        response["success"] = false;
        response["message"] = "用户ID无效";
        return response;
    }
    
    // 管理员ID验证
    if (userType == "admin" && userId != 999999) {
        userId = 999999;
    }
    
    if (userType.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户类型不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        QJsonArray messages;
        
        // 如果是管理员，且指定了otherUserId，则获取与特定用户的聊天记录
        // 否则获取所有聊天记录（用于管理员查看所有消息）
        if (userType == "admin") {
            if (otherUserId > 0 && !otherUserType.isEmpty()) {
                // 管理员查看与特定用户的聊天记录
                messages = Database::getInstance().getChatHistory(userId, userType, otherUserId, otherUserType);
            } else {
                // 管理员查看所有聊天记录
                messages = Database::getInstance().getAllChatMessagesForAdmin();
            }
        } else {
            // 普通用户获取聊天记录
            messages = Database::getInstance().getChatHistory(userId, userType, otherUserId, otherUserType);
        }
        
        response["success"] = true;
        response["messages"] = messages;
        response["message"] = "获取聊天历史成功";
        qDebug() << "获取聊天历史成功，用户ID:" << userId << "类型:" << userType << "消息数量:" << messages.size();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
    response["messages"] = QJsonArray();
#endif
    
    return response;
}

// 处理添加评论请求
QJsonObject TcpFileTask::handleAddReview(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString bookId = request.value("bookId").toString();
    int rating = request.value("rating").toInt();
    QString comment = request.value("comment").toString();
    
    QJsonObject response;
    
    if (userId.isEmpty() || bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID和商品ID不能为空";
        return response;
    }
    
    if (rating < 1 || rating > 5) {
        response["success"] = false;
        response["message"] = "评分必须在1-5分之间";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 检查用户是否已购买过该商品
    if (!Database::getInstance().hasUserPurchasedBook(userId.toInt(), bookId)) {
        response["success"] = false;
        response["message"] = "只有购买过该商品的用户才能评论";
        return response;
    }
    
    // 添加评论
    if (Database::getInstance().addReview(userId.toInt(), bookId, rating, comment)) {
        response["success"] = true;
        response["message"] = "评论添加成功";
    } else {
        response["success"] = false;
        response["message"] = "评论添加失败，请稍后重试";
    }
#else
    response["success"] = true;
    response["message"] = "评论添加成功（内存模式）";
#endif
    
    return response;
}

// 处理获取商品评论请求
QJsonObject TcpFileTask::handleGetBookReviews(const QJsonObject &request)
{
    QString bookId = request.value("bookId").toString();
    
    QJsonObject response;
    
    if (bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "商品ID不能为空";
        response["reviews"] = QJsonArray();
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["reviews"] = QJsonArray();
        return response;
    }
    
    QJsonArray reviews = Database::getInstance().getBookReviews(bookId);
    response["success"] = true;
    response["reviews"] = reviews;
    response["count"] = reviews.size();
#else
    response["success"] = true;
    response["reviews"] = QJsonArray();
    response["count"] = 0;
#endif
    
    return response;
}

// 处理获取商品评分统计请求
QJsonObject TcpFileTask::handleGetBookRatingStats(const QJsonObject &request)
{
    QString bookId = request.value("bookId").toString();
    
    QJsonObject response;
    
    if (bookId.isEmpty()) {
        response["success"] = false;
        response["message"] = "商品ID不能为空";
        response["averageRating"] = 0.0;
        response["reviewCount"] = 0;
        response["hasRating"] = false;
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["averageRating"] = 0.0;
        response["reviewCount"] = 0;
        response["hasRating"] = false;
        return response;
    }
    
    QJsonObject stats = Database::getInstance().getBookRatingStats(bookId);
    response["success"] = true;
    response["averageRating"] = stats.value("averageRating").toDouble();
    response["reviewCount"] = stats.value("reviewCount").toInt();
    response["hasRating"] = stats.value("hasRating").toBool();
#else
    response["success"] = true;
    response["averageRating"] = 0.0;
    response["reviewCount"] = 0;
    response["hasRating"] = false;
#endif
    
    return response;
}

// 处理获取卖家所有商品评论请求
QJsonObject TcpFileTask::handleGetSellerReviews(const QJsonObject &request)
{
    QString sellerId = request.value("sellerId").toString();
    
    QJsonObject response;
    
    if (sellerId.isEmpty()) {
        response["success"] = false;
        response["message"] = "卖家ID不能为空";
        response["reviews"] = QJsonArray();
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["reviews"] = QJsonArray();
        return response;
    }
    
    QJsonArray reviews = Database::getInstance().getSellerReviews(sellerId.toInt());
    response["success"] = true;
    response["reviews"] = reviews;
    response["count"] = reviews.size();
#else
    response["success"] = true;
    response["reviews"] = QJsonArray();
    response["count"] = 0;
#endif
    
    return response;
}

QList<BookInfo> TcpFileTask::getPresetBooks()
{
    QList<BookInfo> books;
    // 预设10本图书数据
    books << BookInfo{"001", "深入理解计算机系统", "tech", "tech_computer", 99.0, 4.8, 1000, 95};
    books << BookInfo{"002", "C++ Primer", "tech", "tech_computer", 128.0, 4.9, 800, 90};
    books << BookInfo{"003", "三体", "fiction", "fiction_novel", 48.0, 4.7, 1500, 88};
    books << BookInfo{"004", "活着", "fiction", "fiction_novel", 35.0, 4.8, 1200, 85};
    books << BookInfo{"005", "时间简史", "tech", "tech_physics", 45.0, 4.6, 600, 80};
    books << BookInfo{"006", "红楼梦", "fiction", "fiction_novel", 68.0, 4.9, 2000, 92};
    books << BookInfo{"007", "Python编程", "tech", "tech_computer", 79.0, 4.7, 1500, 87};
    books << BookInfo{"008", "百年孤独", "fiction", "fiction_novel", 39.0, 4.8, 1800, 89};
    books << BookInfo{"009", "算法导论", "tech", "tech_computer", 118.0, 4.8, 1200, 86};
    books << BookInfo{"010", "小王子", "fiction", "fiction_short", 25.0, 4.9, 2500, 94};
    return books;
}

// 处理取消订单请求（买家申请取消）
QJsonObject TcpFileTask::handleCancelOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    QString userId = request.value("userId").toString();
    QString reason = request.value("reason").toString("用户申请取消");

    QJsonObject response;
    
    if (orderId.isEmpty()) {
        response["success"] = false;
        response["message"] = "订单ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 从数据库读取订单
    if (Database::getInstance().isConnected()) {
        qDebug() << "尝试取消订单，订单ID:" << orderId << "用户ID:" << userId;
        
        QJsonObject order = Database::getInstance().getOrder(orderId);
        if (order.isEmpty() || !order.contains("orderId")) {
            qWarning() << "取消订单失败：订单不存在，订单ID:" << orderId;
            response["success"] = false;
            response["message"] = "订单不存在，订单ID: " + orderId;
            return response;
        }
        
        qDebug() << "找到订单，订单ID:" << orderId << "用户ID:" << order["userId"].toInt() << "状态:" << order["status"].toString();
        
        // 验证订单归属
        if (!userId.isEmpty() && order["userId"].toInt() != userId.toInt()) {
            qWarning() << "取消订单失败：无权操作，订单ID:" << orderId << "请求用户ID:" << userId << "订单用户ID:" << order["userId"].toInt();
            response["success"] = false;
            response["message"] = "无权操作此订单";
            return response;
        }
        
        // 验证订单状态（已支付状态才能申请取消）
        QString status = order["status"].toString();
        if (status != "已支付" && status != "待支付") {
            qWarning() << "取消订单失败：订单状态不允许，订单ID:" << orderId << "状态:" << status;
            response["success"] = false;
            response["message"] = "订单状态不允许取消（" + status + "）";
            return response;
        }
        
        // 更新数据库中的订单状态（包括取消原因）
        if (Database::getInstance().updateOrderStatus(orderId, "已取消", "", reason, "")) {
            // 同时更新内存中的订单（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (int i = 0; i < g_sellerOrders.size(); ++i) {
                if (g_sellerOrders[i]["orderId"].toString() == orderId) {
                    g_sellerOrders[i]["status"] = "已取消";
                    g_sellerOrders[i]["cancelTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    g_sellerOrders[i]["cancelReason"] = reason;
                    break;
                }
            }
            
            qDebug() << "订单取消成功:" << orderId << "原因:" << reason;
            
            response["success"] = true;
            response["message"] = "订单已取消";
            response["orderId"] = orderId;
            response["cancelTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        } else {
            qWarning() << "取消订单失败：更新订单状态失败，订单ID:" << orderId;
            response["success"] = false;
            response["message"] = "更新订单状态失败";
        }
    } else {
        qWarning() << "取消订单失败：数据库未连接";
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    // 查找订单
    bool found = false;
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i]["orderId"].toString() == orderId) {
            QJsonObject order = g_sellerOrders[i];
            
            // 验证订单归属
            if (!userId.isEmpty() && order["userId"].toString() != userId) {
                response["success"] = false;
                response["message"] = "无权操作此订单";
                return response;
            }
            
            // 验证订单状态（已支付状态才能申请取消）
            QString status = order["status"].toString();
            if (status != "已支付" && status != "待支付") {
                response["success"] = false;
                response["message"] = "订单状态不允许取消（" + status + "）";
                return response;
            }
            
            // 更新订单状态为已取消
            order["status"] = "已取消";
            order["cancelTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            order["cancelReason"] = reason;
            
            g_sellerOrders[i] = order;
            found = true;
            
            qDebug() << "订单取消成功:" << orderId << "原因:" << reason;
            
            response["success"] = true;
            response["message"] = "订单已取消";
            response["orderId"] = orderId;
            response["cancelTime"] = order["cancelTime"].toString();
            break;
        }
    }
    
    if (!found) {
        response["success"] = false;
        response["message"] = "订单不存在";
    }
#endif

    return response;
}

// 处理确认收货请求（买家确认收货）
QJsonObject TcpFileTask::handleConfirmReceiveOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    QString userId = request.value("userId").toString();

    QJsonObject response;
    
    if (orderId.isEmpty()) {
        response["success"] = false;
        response["message"] = "订单ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 从数据库读取订单
    if (Database::getInstance().isConnected()) {
        qDebug() << "尝试确认收货，订单ID:" << orderId << "用户ID:" << userId;
        
        QJsonObject order = Database::getInstance().getOrder(orderId);
        if (order.isEmpty() || !order.contains("orderId")) {
            qWarning() << "确认收货失败：订单不存在，订单ID:" << orderId;
            response["success"] = false;
            response["message"] = "订单不存在，订单ID: " + orderId;
            return response;
        }
        
        qDebug() << "找到订单，订单ID:" << orderId << "用户ID:" << order["userId"].toInt() << "状态:" << order["status"].toString();
        
        // 验证订单归属
        if (!userId.isEmpty() && order["userId"].toInt() != userId.toInt()) {
            qWarning() << "确认收货失败：无权操作，订单ID:" << orderId << "请求用户ID:" << userId << "订单用户ID:" << order["userId"].toInt();
            response["success"] = false;
            response["message"] = "无权操作此订单";
            return response;
        }
        
        // 验证订单状态（只有已发货状态才能确认收货）
        QString status = order["status"].toString();
        if (status != "已发货") {
            qWarning() << "确认收货失败：订单状态不允许，订单ID:" << orderId << "状态:" << status;
            response["success"] = false;
            response["message"] = "只有【已发货】状态的订单才能确认收货（当前状态：" + status + "）";
            return response;
        }
        
        // 更新数据库中的订单状态为"已完成"
        if (Database::getInstance().updateOrderStatus(orderId, "已完成", "", "", "")) {
            // 同时更新内存中的订单（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (int i = 0; i < g_sellerOrders.size(); ++i) {
                if (g_sellerOrders[i]["orderId"].toString() == orderId) {
                    g_sellerOrders[i]["status"] = "已完成";
                    g_sellerOrders[i]["receiveTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    break;
                }
            }
            
            qDebug() << "确认收货成功:" << orderId;
            
            response["success"] = true;
            response["message"] = "确认收货成功，订单状态已更新为已完成";
            response["orderId"] = orderId;
            response["receiveTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        } else {
            qWarning() << "确认收货失败：更新订单状态失败，订单ID:" << orderId;
            response["success"] = false;
            response["message"] = "更新订单状态失败";
        }
    } else {
        qWarning() << "确认收货失败：数据库未连接";
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    // 查找订单
    bool found = false;
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i]["orderId"].toString() == orderId) {
            // 验证订单归属
            if (!userId.isEmpty() && g_sellerOrders[i]["userId"].toInt() != userId.toInt()) {
                response["success"] = false;
                response["message"] = "无权操作此订单";
                return response;
            }
            
            // 验证订单状态
            QString status = g_sellerOrders[i]["status"].toString();
            if (status != "已发货") {
                response["success"] = false;
                response["message"] = "只有【已发货】状态的订单才能确认收货（当前状态：" + status + "）";
                return response;
            }
            
            // 更新订单状态
            g_sellerOrders[i]["status"] = "已完成";
            g_sellerOrders[i]["receiveTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            found = true;
            break;
        }
    }
    
    if (found) {
        response["success"] = true;
        response["message"] = "确认收货成功，订单状态已更新为已完成";
        response["orderId"] = orderId;
    } else {
        response["success"] = false;
        response["message"] = "订单不存在";
    }
#endif

    return response;
}

// 处理发货请求（卖家发货）
QJsonObject TcpFileTask::handleShipOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    QString sellerId = request.value("sellerId").toString();
    QString trackingNumber = request.value("trackingNumber").toString();

    QJsonObject response;
    
    if (orderId.isEmpty()) {
        response["success"] = false;
        response["message"] = "订单ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 从数据库查询订单信息
    if (Database::getInstance().isConnected()) {
        QJsonObject order = Database::getInstance().getOrder(orderId);
        
        if (order.isEmpty()) {
            response["success"] = false;
            response["message"] = "订单不存在";
            return response;
        }
        
        // 验证订单状态（必须是已支付状态才能发货）
        QString status = order["status"].toString();
        if (status != "已支付") {
            response["success"] = false;
            response["message"] = QString("订单状态不正确，无法发货（当前状态：%1）").arg(status);
            return response;
        }
        
        // 更新数据库中的订单状态
        QString finalTrackingNumber = trackingNumber.isEmpty() ? QString("SF%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000) : trackingNumber;
        if (Database::getInstance().updateOrderStatus(orderId, "已发货", "", "", finalTrackingNumber)) {
            // 同时更新内存中的订单（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (auto &o : g_sellerOrders) {
                if (o.value("orderId").toString() == orderId) {
                    o["status"] = "已发货";
                    o["shipTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    o["trackingNumber"] = finalTrackingNumber;
                    break;
                }
            }
            
            qDebug() << "订单发货成功:" << orderId << "物流单号:" << finalTrackingNumber;
            
            response["success"] = true;
            response["message"] = "发货成功";
            response["orderId"] = orderId;
            response["shipTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            response["trackingNumber"] = finalTrackingNumber;
        } else {
            response["success"] = false;
            response["message"] = "更新订单状态失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    // 查找订单
    bool found = false;
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i]["orderId"].toString() == orderId) {
            QJsonObject order = g_sellerOrders[i];
            
            // 验证订单状态（必须是已支付状态才能发货）
            QString status = order["status"].toString();
            if (status != "已支付") {
                response["success"] = false;
                response["message"] = "订单状态不正确，无法发货（当前状态：" + status + "）";
                return response;
            }
            
            // 更新订单状态为已发货
            QString finalTrackingNumber = trackingNumber.isEmpty() ? QString("SF%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000) : trackingNumber;
            order["status"] = "已发货";
            order["shipTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            order["trackingNumber"] = finalTrackingNumber;
            
            g_sellerOrders[i] = order;
            found = true;
            
            qDebug() << "订单发货成功:" << orderId << "物流单号:" << finalTrackingNumber;
            
            response["success"] = true;
            response["message"] = "发货成功";
            response["orderId"] = orderId;
            response["shipTime"] = order["shipTime"].toString();
            response["trackingNumber"] = finalTrackingNumber;
            break;
        }
    }
    
    if (!found) {
        response["success"] = false;
        response["message"] = "订单不存在";
    }
#endif

    return response;
}

// 处理卖家认证申请
QJsonObject TcpFileTask::handleApplySellerCertification(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    QString username = request.value("username").toString();
    QString password = request.value("password").toString();
    QString email = request.value("email").toString();
    QString licenseImageBase64 = request.value("licenseImage").toString();
    
    qDebug() << "========================================";
    qDebug() << "收到卖家认证申请";
    qDebug() << "用户ID:" << userId;
    qDebug() << "用户名:" << username;
    qDebug() << "邮箱:" << email;
    qDebug() << "图片Base64大小:" << licenseImageBase64.size() << "字节";
    qDebug() << "========================================";
    
    QJsonObject response;
    
    // 只检查关键参数，允许email为空
    if (userId.isEmpty() || username.isEmpty() || password.isEmpty()) {
        qDebug() << "参数验证失败:";
        qDebug() << "  userId为空:" << userId.isEmpty()<<userId;
        qDebug() << "  username为空:" << username.isEmpty()<<username;
        qDebug() << "  password为空:" << password.isEmpty()<<password;
        //qDebug() << "  licenseImageBase64为空:" << licenseImageBase64.isEmpty();
        response["success"] = false;
        response["message"] = "缺少必要信息，请确保个人信息完善";
        return response;
    }
    
#if USE_DATABASE
    // 使用MySQL数据库
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 检查是否已经申请过认证
    // 先检查用户的role，确定当前状态
    QJsonArray usersArray = Database::getInstance().getAllUsers();
    int userRole = 1;  // 默认为买家
    for (const QJsonValue &userVal : usersArray) {
        QJsonObject user = userVal.toObject();
        if (user.value("userId").toInt() == userId.toInt()) {
            userRole = user.value("role").toInt();
            break;
        }
    }
    
    // 如果用户已经是商家（role = 2），不允许再次申请
    if (userRole == 2) {
        response["success"] = false;
        response["message"] = "您已经是商家，无需再次申请认证";
        return response;
    }
    
    // 如果用户的role = 0（审核中），不允许再次提交
    if (userRole == 0) {
            response["success"] = false;
        response["message"] = "提交失败，正在审核中，请耐心等待管理员审核";
            return response;
    }
    
    // role=1的用户可以提交申请
    
    // 注意：提交申请时会将role设置为0（审核中），只有管理员审核通过后才会设置为2（商家）
    
    // 保存认证申请（包括营业执照图片）
    if (Database::getInstance().applySellerCertification(userId.toInt(), username, password, email, licenseImageBase64)) {
        response["success"] = true;
        response["message"] = "认证申请已提交，等待审核";
        response["status"] = "审核中";
    } else {
        response["success"] = false;
        response["message"] = "提交认证申请失败";
    }
    
    return response;
#else
    // 内存模式（简单实现）
    response["success"] = true;
    response["message"] = "认证申请已提交（内存模式）";
    response["status"] = "审核中";
    return response;
#endif
}

// 处理查询卖家认证状态
QJsonObject TcpFileTask::handleGetSellerCertStatus(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    
    QJsonObject response;
    
    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 使用MySQL数据库
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    QJsonObject certInfo = Database::getInstance().getSellerCertification(userId.toInt());
    
    // 检查用户是否已认证为商家：必须同时满足两个条件
    // 1. seller_certifications表中有记录且status = "已认证"
    // 2. users表的role = 2（商家）
    
    // 先检查users表的role字段
    QJsonArray usersArray = Database::getInstance().getAllUsers();
    int userRole = 1;  // 默认为买家
    for (const QJsonValue &userVal : usersArray) {
        QJsonObject user = userVal.toObject();
        if (user.value("userId").toInt() == userId.toInt()) {
            userRole = user.value("role").toInt();
            break;
        }
    }
    
    // 根据role字段直接返回状态
    // role=0 → 审核中
    // role=1 → 未认证
    // role=2 → 已认证
    if (userRole == 0) {
        response["success"] = true;
        response["status"] = "审核中";
        response["message"] = "认证审核中，请耐心等待";
        return response;
    } else if (userRole == 1) {
        response["success"] = true;
        response["status"] = "未认证";
        response["message"] = "未申请认证";
        return response;
    } else if (userRole == 2) {
            response["success"] = true;
            response["status"] = "已认证";
            response["message"] = "已认证为卖家";
        return response;
    }
    
    // 其他情况，返回未认证
            response["success"] = true;
            response["status"] = "未认证";
            response["message"] = "未申请认证";
    
    return response;
#else
    // 内存模式
    response["success"] = true;
    response["status"] = "未认证";
    response["message"] = "未申请认证（内存模式）";
    return response;
#endif
}

// ===== 管理员端接口实现 =====

static void ensureAdminDataInited()
{
    QMutexLocker locker(&g_adminMutex);
    if (g_adminDataInited) return;
    
    // 从数据库加载用户和商家数据
#if USE_DATABASE
    if (Database::getInstance().isConnected()) {
        // 加载所有用户
        QJsonArray usersArray = Database::getInstance().getAllUsers();
        for (const QJsonValue &userVal : usersArray) {
            QJsonObject user = userVal.toObject();
            g_adminUsers.append(user);
        }
        
        // 加载所有商家
        QJsonArray sellersArray = Database::getInstance().getAllSellers();
        for (const QJsonValue &sellerVal : sellersArray) {
            QJsonObject seller = sellerVal.toObject();
            g_adminSellers.append(seller);
        }
        
        qDebug() << "从数据库加载用户和商家数据完成";
    } else {
        qWarning() << "数据库未连接，无法加载用户和商家数据";
    }
#else
    // 不使用数据库时，添加一个默认商家账号
    QJsonObject seller1;
    seller1["sellerId"] = 2001;
    seller1["sellerName"] = "seller";
    seller1["password"] = "123456";
    seller1["email"] = "seller@example.com";
    seller1["registerDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    seller1["status"] = "正常";
    g_adminSellers.append(seller1);
#endif
    
    g_adminDataInited = true;
    
    qDebug() << "========================================";
    qDebug() << "系统初始化完成";
    qDebug() << "买家用户: " << g_adminUsers.size() << "个";
    qDebug() << "商家账号: " << g_adminSellers.size() << "个";
    qDebug() << "========================================";
}

// 管理员登录
QJsonObject TcpFileTask::handleAdminLogin(const QJsonObject &request)
{
    QString username = request.value("username").toString();
    QString password = request.value("password").toString();
    
    QJsonObject response;
    
    // 唯一管理员账户：用户名admin，密码admin123
    if (username == "admin" && password == "admin123") {
        response["success"] = true;
        response["message"] = "管理员登录成功";
        response["adminId"] = "ADMIN001";
        response["username"] = username;
        qDebug() << "管理员登录成功：" << username;
    } else {
        response["success"] = false;
        response["message"] = "用户名或密码错误";
        qDebug() << "管理员登录失败：用户名=" << username << "密码=" << (password.isEmpty() ? "空" : "***");
    }
    
    return response;
}

// 获取所有用户
QJsonObject TcpFileTask::handleAdminGetAllUsers(const QJsonObject &request)
{
    QMutexLocker locker(&g_adminMutex);
    
    QJsonObject response;
    response["success"] = true;
    response["message"] = "获取用户列表成功";
    
#if USE_DATABASE
    // 从数据库加载最新数据
    if (Database::getInstance().isConnected()) {
        g_adminUsers.clear();
        QJsonArray usersArray = Database::getInstance().getAllUsers();
        for (const QJsonValue &userVal : usersArray) {
            QJsonObject user = userVal.toObject();
            g_adminUsers.append(user);
        }
    } else {
        ensureAdminDataInited();
    }
#else
    ensureAdminDataInited();
#endif
    
    QJsonArray usersArray;
    for (const auto &user : g_adminUsers) {
        usersArray.append(user);
    }
    
    response["users"] = usersArray;
    response["total"] = usersArray.size();
    
    return response;
}

// 删除用户
QJsonObject TcpFileTask::handleAdminDeleteUser(const QJsonObject &request)
{
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    QString userId = request.value("userId").toString();
    
    for (int i = 0; i < g_adminUsers.size(); ++i) {
        if (g_adminUsers[i].value("userId").toString() == userId) {
            g_adminUsers.removeAt(i);
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "用户删除成功";
            return response;
        }
    }
    
    QJsonObject response;
    response["success"] = false;
    response["message"] = "未找到指定用户";
    return response;
}

// 封禁/解封用户
QJsonObject TcpFileTask::handleAdminBanUser(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    bool banned = request.value("banned").toBool();
    
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库更新用户状态
    if (Database::getInstance().isConnected()) {
        QString status = banned ? "封禁" : "正常";
        if (Database::getInstance().updateUserStatus(userId.toInt(), status)) {
            response["success"] = true;
            response["message"] = banned ? "用户已封禁" : "用户已解封";
        } else {
            response["success"] = false;
            response["message"] = "更新用户状态失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 使用内存存储（原有逻辑）
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    for (int i = 0; i < g_adminUsers.size(); ++i) {
        if (g_adminUsers[i].value("userId").toString() == userId) {
            g_adminUsers[i]["status"] = banned ? "封禁" : "正常";
            
            response["success"] = true;
            response["message"] = banned ? "用户已封禁" : "用户已解封";
            return response;
        }
    }
    
    response["success"] = false;
    response["message"] = "未找到指定用户";
#endif
    
    return response;
}

// 获取所有商家
QJsonObject TcpFileTask::handleAdminGetAllSellers(const QJsonObject &request)
{
    QMutexLocker locker(&g_adminMutex);
    
    QJsonObject response;
    response["success"] = true;
    response["message"] = "获取商家列表成功";
    
#if USE_DATABASE
    // 从数据库加载最新数据
    if (Database::getInstance().isConnected()) {
        g_adminSellers.clear();
        QJsonArray sellersArray = Database::getInstance().getAllSellers();
        for (const QJsonValue &sellerVal : sellersArray) {
            QJsonObject seller = sellerVal.toObject();
            g_adminSellers.append(seller);
        }
    } else {
        ensureAdminDataInited();
    }
#else
    ensureAdminDataInited();
#endif
    
    QJsonArray sellersArray;
    for (const auto &seller : g_adminSellers) {
        sellersArray.append(seller);
    }
    
    response["sellers"] = sellersArray;
    response["total"] = sellersArray.size();
    
    return response;
}

// 删除商家
QJsonObject TcpFileTask::handleAdminDeleteSeller(const QJsonObject &request)
{
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    QString sellerId = request.value("sellerId").toString();
    
    for (int i = 0; i < g_adminSellers.size(); ++i) {
        if (g_adminSellers[i].value("sellerId").toString() == sellerId) {
            g_adminSellers.removeAt(i);
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "商家删除成功";
            return response;
        }
    }
    
    QJsonObject response;
    response["success"] = false;
    response["message"] = "未找到指定商家";
    return response;
}

// 封禁/解封商家
QJsonObject TcpFileTask::handleAdminBanSeller(const QJsonObject &request)
{
    QString sellerId = request.value("sellerId").toString();
    bool banned = request.value("banned").toBool();
    
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库更新商家状态
    if (Database::getInstance().isConnected()) {
        QString status = banned ? "封禁" : "正常";
        if (Database::getInstance().updateSellerStatus(sellerId.toInt(), status)) {
            // 如果封禁商家，将该商家所有图书状态改为"下架"
            if (banned) {
                Database::getInstance().updateBooksStatusBySellerId(sellerId.toInt(), "下架");
            } else {
                // 解封商家时，将该商家所有图书状态恢复为"正常"
                Database::getInstance().updateBooksStatusBySellerId(sellerId.toInt(), "正常");
            }
            response["success"] = true;
            response["message"] = banned ? "商家已封禁，其所有图书已下架" : "商家已解封，其所有图书已恢复上架";
        } else {
            response["success"] = false;
            response["message"] = "更新商家状态失败";
        }
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
    }
#else
    // 使用内存存储（原有逻辑）
    ensureAdminDataInited();
    QMutexLocker locker(&g_adminMutex);
    
    for (int i = 0; i < g_adminSellers.size(); ++i) {
        if (g_adminSellers[i].value("sellerId").toString() == sellerId) {
            g_adminSellers[i]["status"] = banned ? "封禁" : "正常";
            
            response["success"] = true;
            response["message"] = banned ? "商家已封禁" : "商家已解封";
            return response;
        }
    }
    
    response["success"] = false;
    response["message"] = "未找到指定商家";
#endif
    
    return response;
}

// 获取所有图书（全局）
QJsonObject TcpFileTask::handleAdminGetAllBooks(const QJsonObject &request)
{
    Q_UNUSED(request);
    
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库获取所有图书
    if (Database::getInstance().isConnected()) {
        QJsonArray dbBooks = Database::getInstance().getAllBooks();
        QJsonArray booksArray;
        
        for (const QJsonValue &value : dbBooks) {
            QJsonObject dbBook = value.toObject();
            QJsonObject bookObj;
            
            // 映射数据库字段到客户端需要的字段
            bookObj["isbn"] = dbBook["isbn"].toString();
            bookObj["title"] = dbBook["title"].toString();
            bookObj["author"] = dbBook["author"].toString();
            // 组合分类信息：一级分类 + 二级分类
            QString category1 = dbBook["category1"].toString();
            QString category2 = dbBook["category2"].toString();
            QString categoryDisplay = category1;
            if (!category2.isEmpty()) {
                categoryDisplay += " / " + category2;
            }
            bookObj["category"] = categoryDisplay;
            bookObj["category1"] = category1;
            bookObj["category2"] = category2;
            bookObj["price"] = dbBook["price"].toDouble();
            bookObj["stock"] = dbBook["stock"].toInt();
            bookObj["status"] = dbBook["status"].toString();
            bookObj["merchantId"] = dbBook["merchantId"].toInt();
            bookObj["coverImage"] = dbBook["coverImage"].toString();
            
            booksArray.append(bookObj);
        }
        
    response["success"] = true;
    response["message"] = "获取图书列表成功";
        response["books"] = booksArray;
        response["total"] = booksArray.size();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["books"] = QJsonArray();
        response["total"] = 0;
    }
#else
    // 使用内存存储（原有逻辑）
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);
    
    QJsonArray booksArray;
    for (const auto &book : g_sellerBooks) {
        booksArray.append(book);
    }
    
    response["success"] = true;
    response["message"] = "获取图书列表成功";
    response["books"] = booksArray;
    response["total"] = booksArray.size();
#endif
    
    return response;
}

// 删除图书（全局）
QJsonObject TcpFileTask::handleAdminDeleteBook(const QJsonObject &request)
{
    QString bookId = request.value("bookId").toString();
    
#if USE_DATABASE
    // 使用数据库删除图书
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().deleteBook(bookId)) {
            QJsonObject response;
            response["success"] = true;
            response["message"] = "图书删除成功";
            return response;
        } else {
            QJsonObject response;
            response["success"] = false;
            response["message"] = "删除图书失败";
            return response;
        }
    } else {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
#else
    // 使用内存存储（原有逻辑）
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);
    
    for (int i = 0; i < g_sellerBooks.size(); ++i) {
        if (g_sellerBooks[i].value("isbn").toString() == bookId) {
            g_sellerBooks.removeAt(i);
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "图书删除成功";
            return response;
        }
    }
    
    QJsonObject response;
    response["success"] = false;
    response["message"] = "未找到指定图书";
    return response;
#endif
}

// 更新图书（全局）
QJsonObject TcpFileTask::handleAdminGetPendingBooks(const QJsonObject &request)
{
    Q_UNUSED(request);
    
    QJsonObject response;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    QJsonArray books = Database::getInstance().getPendingBooks();
    response["success"] = true;
    response["books"] = books;
#else
    response["success"] = true;
    response["books"] = QJsonArray();
#endif
    return response;
}

QJsonObject TcpFileTask::handleAdminGetPendingSellerCertifications(const QJsonObject &request)
{
    Q_UNUSED(request);
    
    QJsonObject response;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    QJsonArray certifications = Database::getInstance().getPendingSellerCertifications();
    response["success"] = true;
    response["certifications"] = certifications;
#else
    response["success"] = true;
    response["certifications"] = QJsonArray();
#endif
    return response;
}

QJsonObject TcpFileTask::handleAdminApproveBook(const QJsonObject &request)
{
    QString isbn = request.value("isbn").toString();
    if (isbn.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "ISBN不能为空";
        return response;
    }
    
    QJsonObject response;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    if (Database::getInstance().approveBook(isbn)) {
        response["success"] = true;
        response["message"] = "审核通过成功，书籍已上架";
        response["isbn"] = isbn;
        response["status"] = "正常";
    } else {
        response["success"] = false;
        response["message"] = "审核通过失败，请检查书籍状态是否为'待审核'";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
#endif
    return response;
}

QJsonObject TcpFileTask::handleAdminRejectBook(const QJsonObject &request)
{
    QString isbn = request.value("isbn").toString();
    if (isbn.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "ISBN不能为空";
        return response;
    }
    
    QJsonObject response;
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    if (Database::getInstance().rejectBook(isbn)) {
        response["success"] = true;
        response["message"] = "审核拒绝成功，书籍不上架";
        response["isbn"] = isbn;
        response["status"] = "已拒绝";
    } else {
        response["success"] = false;
        response["message"] = "审核拒绝失败，请检查书籍状态是否为'待审核'";
    }
#else
    response["success"] = false;
    response["message"] = "未启用数据库";
#endif
    return response;
}

QJsonObject TcpFileTask::handleAdminUpdateBook(const QJsonObject &request)
{
    QString bookId = request.value("bookId").toString();
    if (bookId.isEmpty()) {
        bookId = request.value("isbn").toString();  // 兼容isbn字段
    }
    
#if USE_DATABASE
    // 使用数据库更新图书
    if (Database::getInstance().isConnected()) {
        QJsonObject bookData;
        
        // 收集要更新的字段
        if (request.contains("title")) {
            bookData["title"] = request["title"].toString();
        }
        if (request.contains("author")) {
            bookData["author"] = request["author"].toString();
        }
        if (request.contains("category1")) {
            bookData["category1"] = request["category1"].toString();
        }
        if (request.contains("category2")) {
            bookData["category2"] = request["category2"].toString();
        }
        if (request.contains("price")) {
            bookData["price"] = request["price"].toDouble();
        }
        if (request.contains("stock")) {
            bookData["stock"] = request["stock"].toInt();
        }
        if (request.contains("status")) {
            bookData["status"] = request["status"].toString();
        }
        if (request.contains("coverImage")) {
            bookData["coverImage"] = request["coverImage"].toString();
        }
        
        if (Database::getInstance().updateBook(bookId, bookData)) {
            QJsonObject response;
            response["success"] = true;
            response["message"] = "图书更新成功";
            return response;
        } else {
            QJsonObject response;
            response["success"] = false;
            response["message"] = "更新图书失败";
            return response;
        }
    } else {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
#else
    // 使用内存存储（原有逻辑）
    ensureSellerBooksInited();
    QMutexLocker locker(&g_sellerBooksMutex);
    
    for (int i = 0; i < g_sellerBooks.size(); ++i) {
        if (g_sellerBooks[i].value("isbn").toString() == bookId) {
            QJsonObject &book = g_sellerBooks[i];
            
            // 更新字段
            if (request.contains("title")) {
                book["title"] = request["title"].toString();
            }
            if (request.contains("author")) {
                book["author"] = request["author"].toString();
            }
            if (request.contains("category1")) {
                book["category1"] = request["category1"].toString();
            }
            if (request.contains("category2")) {
                book["category2"] = request["category2"].toString();
            }
            if (request.contains("price")) {
                book["price"] = request["price"].toDouble();
            }
            if (request.contains("stock")) {
                book["stock"] = request["stock"].toInt();
            }
            if (request.contains("status")) {
                book["status"] = request["status"].toString();
            }
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "图书更新成功";
            return response;
        }
    }
    
    QJsonObject response;
    response["success"] = false;
    response["message"] = "未找到指定图书";
    return response;
#endif
}

// 获取所有订单（全局）
QJsonObject TcpFileTask::handleAdminGetAllOrders(const QJsonObject &request)
{
    QJsonObject response;
    
#if USE_DATABASE
    // 从数据库读取所有订单
    if (Database::getInstance().isConnected()) {
        QJsonArray orders = Database::getInstance().getAllOrders();
        response["success"] = true;
        response["message"] = "获取订单列表成功";
        response["orders"] = orders;
        response["total"] = orders.size();
    } else {
        response["success"] = false;
        response["message"] = "数据库未连接";
        response["orders"] = QJsonArray();
        response["total"] = 0;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    response["success"] = true;
    response["message"] = "获取订单列表成功";
    
    QJsonArray ordersArray;
    for (const auto &order : g_sellerOrders) {
        ordersArray.append(order);
    }
    
    response["orders"] = ordersArray;
    response["total"] = ordersArray.size();
#endif
    
    return response;
}

// 删除订单（全局）
QJsonObject TcpFileTask::handleAdminDeleteOrder(const QJsonObject &request)
{
    QString orderId = request.value("orderId").toString();
    
    if (orderId.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "订单ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    // 从数据库删除订单
    if (Database::getInstance().isConnected()) {
        if (Database::getInstance().deleteOrder(orderId)) {
            // 同时从内存中删除（用于向后兼容）
            QMutexLocker locker(&g_sellerOrdersMutex);
            for (int i = 0; i < g_sellerOrders.size(); ++i) {
                if (g_sellerOrders[i].value("orderId").toString() == orderId) {
                    g_sellerOrders.removeAt(i);
                    break;
                }
            }
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "订单删除成功";
            return response;
        } else {
            QJsonObject response;
            response["success"] = false;
            response["message"] = "删除订单失败";
            return response;
        }
    } else {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
#else
    // 不使用数据库时，使用内存存储
    QMutexLocker locker(&g_sellerOrdersMutex);
    
    for (int i = 0; i < g_sellerOrders.size(); ++i) {
        if (g_sellerOrders[i].value("orderId").toString() == orderId) {
            g_sellerOrders.removeAt(i);
            
            QJsonObject response;
            response["success"] = true;
            response["message"] = "订单删除成功";
            return response;
        }
    }
    
    QJsonObject response;
    response["success"] = false;
    response["message"] = "未找到指定订单";
    return response;
#endif
}

// 获取系统统计
QJsonObject TcpFileTask::handleAdminGetSystemStats(const QJsonObject &request)
{
#if USE_DATABASE
    // 使用数据库统计
    if (Database::getInstance().isConnected()) {
        QJsonObject stats = Database::getInstance().getSystemStats();
        QJsonObject response;
        response["success"] = true;
        response["message"] = "获取统计数据成功";
        response["stats"] = stats;
        return response;
    }
#endif
    
    // 使用内存统计
    ensureAdminDataInited();
    ensureSellerBooksInited();
    
    QMutexLocker adminLocker(&g_adminMutex);
    QMutexLocker booksLocker(&g_sellerBooksMutex);
    QMutexLocker ordersLocker(&g_sellerOrdersMutex);
    QMutexLocker membersLocker(&g_sellerMembersMutex);
    
    QJsonObject stats;
    stats["totalUsers"] = g_adminUsers.size();
    stats["totalSellers"] = g_adminSellers.size();
    stats["totalBooks"] = g_sellerBooks.size();
    stats["totalOrders"] = g_sellerOrders.size();
    stats["totalMembers"] = g_sellerMembers.size();
    
    QJsonObject response;
    response["success"] = true;
    response["message"] = "获取统计数据成功";
    response["stats"] = stats;
    
    return response;
}

// 获取请求日志
QJsonObject TcpFileTask::handleAdminGetRequestLogs(const QJsonObject &request)
{
    QJsonObject response;
    
#if USE_DATABASE
    // 使用数据库查询
    if (Database::getInstance().isConnected()) {
        int limit = request.value("limit").toInt(100);
        QString category = request.value("category").toString("");
        
        QJsonArray logs = Database::getInstance().getRequestLogs(limit, category);
        
        response["success"] = true;
        response["message"] = "获取请求日志成功";
        response["logs"] = logs;
        response["total"] = logs.size();
        return response;
    }
#endif
    
    // 数据库未连接
    response["success"] = false;
    response["message"] = "数据库未连接，无法查询请求日志";
    return response;
}

// 管理员获取卖家认证信息
QJsonObject TcpFileTask::handleAdminGetSellerCertification(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    
    QJsonObject response;
    
    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    // 获取认证信息
    QJsonObject certInfo = Database::getInstance().getSellerCertification(userId.toInt());
    if (certInfo.isEmpty()) {
        response["success"] = false;
        response["message"] = "未找到认证申请";
    } else {
        response["success"] = true;
        response["message"] = "获取认证信息成功";
        response["certification"] = certInfo;
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 管理员审核通过卖家认证
QJsonObject TcpFileTask::handleAdminApproveSellerCertification(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    
    QJsonObject response;
    
    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    if (Database::getInstance().approveSellerCertification(userId.toInt())) {
        response["success"] = true;
        response["message"] = "审核通过，用户已成功成为卖家";
    } else {
        response["success"] = false;
        response["message"] = "审核失败，请检查认证申请是否存在";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

// 管理员拒绝卖家认证
QJsonObject TcpFileTask::handleAdminRejectSellerCertification(const QJsonObject &request)
{
    QString userId = request.value("userId").toString();
    
    QJsonObject response;
    
    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户ID不能为空";
        return response;
    }
    
#if USE_DATABASE
    if (!Database::getInstance().isConnected()) {
        response["success"] = false;
        response["message"] = "数据库未连接";
        return response;
    }
    
    if (Database::getInstance().rejectSellerCertification(userId.toInt())) {
        response["success"] = true;
        response["message"] = "审核已拒绝";
    } else {
        response["success"] = false;
        response["message"] = "拒绝审核失败";
    }
#else
    response["success"] = false;
    response["message"] = "数据库功能未启用";
#endif
    
    return response;
}

