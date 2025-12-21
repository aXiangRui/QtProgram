#include "data.h"
#include <QDateTime>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QSqlRecord>

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

// ==========================================
// Database类实现 - MySQL数据库管理
// ==========================================

Database::Database() : m_connected(false)
{
    qDebug() << "Database实例创建";
}

Database::~Database()
{
    closeConnection();
}

Database& Database::getInstance()
{
    static Database instance;
    return instance;
}

bool Database::initConnection(const QString& host, int port, const QString& dbName, 
                              const QString& username, const QString& password)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        qDebug() << "数据库已经连接";
        return true;
    }
    
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(username);
    m_db.setPassword(password);
    
    if (!m_db.open()) {
        qCritical() << "❌ 数据库连接失败:" << m_db.lastError().text();
        return false;
    }
    
    qDebug() << "✅ 数据库连接成功:" << dbName;
    m_connected = true;
    
    // 创建表结构
    if (!createTables()) {
        qCritical() << "❌ 创建数据库表失败";
        closeConnection();
        return false;
    }
    
    return true;
}

bool Database::isConnected() const
{
    return m_connected && m_db.isOpen();
}

void Database::closeConnection()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_connected) {
        m_db.close();
        m_connected = false;
        qDebug() << "数据库连接已关闭";
    }
}

bool Database::createTables()
{
    QSqlQuery query(m_db);
    
    qDebug() << "开始创建数据库表...";
    
    // 1. 请求日志表 - 记录所有API请求
    QString createRequestLogsTable = R"(
        CREATE TABLE IF NOT EXISTS request_logs (
            id INT AUTO_INCREMENT PRIMARY KEY COMMENT '日志ID',
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '请求时间',
            client_ip VARCHAR(50) COMMENT '客户端IP',
            client_port INT COMMENT '客户端端口',
            action VARCHAR(100) COMMENT '请求动作',
            request_data TEXT COMMENT '请求数据(JSON)',
            response_data TEXT COMMENT '响应数据(JSON)',
            success BOOLEAN COMMENT '是否成功',
            category VARCHAR(50) COMMENT '请求分类',
            INDEX idx_timestamp (timestamp),
            INDEX idx_action (action),
            INDEX idx_category (category)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='API请求日志表'
    )";
    
    if (!query.exec(createRequestLogsTable)) {
        qCritical() << "创建request_logs表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ request_logs表创建成功";
    
    // 2. 用户表（买家）
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '用户ID',
            username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
            password VARCHAR(100) NOT NULL COMMENT '密码',
            email VARCHAR(100) COMMENT '邮箱',
            phone_number VARCHAR(20) COMMENT '电话号码',
            address VARCHAR(300) COMMENT '地址',
            balance DECIMAL(10, 2) DEFAULT 0.00 COMMENT '账户余额',
            register_date DATE COMMENT '注册日期',
            status VARCHAR(20) DEFAULT '正常' COMMENT '账户状态',
            license_image_base64 TEXT COMMENT '营业执照图片(Base64编码)',
            role TINYINT DEFAULT 1 COMMENT '用户身份：1-买家，2-卖家，0-买家申请成为卖家且在审核中',
            INDEX idx_username (username),
            INDEX idx_role (role)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='买家用户表'
    )";
    
    if (!query.exec(createUsersTable)) {
        qCritical() << "创建users表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ users表创建成功";
    
    // 检查并添加license_image_base64字段（如果表已存在但字段不存在）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'license_image_base64'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        // 字段不存在，添加字段
        QString addColumnSql = "ALTER TABLE users ADD COLUMN license_image_base64 TEXT COMMENT '营业执照图片(Base64编码)'";
        if (query.exec(addColumnSql)) {
            qDebug() << "✓ 已添加license_image_base64字段到users表";
        } else {
            qWarning() << "添加license_image_base64字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加role字段（如果表已存在但字段不存在）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'role'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        // 字段不存在，添加字段，默认值为1（买家）
        QString addRoleSql = "ALTER TABLE users ADD COLUMN role TINYINT DEFAULT 1 COMMENT '用户身份：1-买家，2-卖家，0-买家申请成为卖家且在审核中'";
        if (query.exec(addRoleSql)) {
            qDebug() << "✓ 已添加role字段到users表";
            // 更新现有用户的role值，默认为1（买家）
            query.exec("UPDATE users SET role = 1 WHERE role IS NULL");
        } else {
            qWarning() << "添加role字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加total_recharge字段（累计充值总额）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'total_recharge'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addTotalRechargeSql = "ALTER TABLE users ADD COLUMN total_recharge DECIMAL(10, 2) DEFAULT 0.00 COMMENT '累计充值总额' AFTER balance";
        if (query.exec(addTotalRechargeSql)) {
            qDebug() << "✓ 已添加total_recharge字段到users表";
        } else {
            qWarning() << "添加total_recharge字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加member_level字段（会员等级）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'member_level'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addMemberLevelSql = "ALTER TABLE users ADD COLUMN member_level VARCHAR(20) DEFAULT '普通会员' COMMENT '会员等级：普通会员、银卡会员、金卡会员、铂金会员、钻石会员、黑钻会员' AFTER total_recharge";
        if (query.exec(addMemberLevelSql)) {
            qDebug() << "✓ 已添加member_level字段到users表";
            // 初始化现有用户的会员等级
            query.exec("UPDATE users SET member_level = '普通会员' WHERE member_level IS NULL");
        } else {
            qWarning() << "添加member_level字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加points字段（积分）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'points'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addPointsSql = "ALTER TABLE users ADD COLUMN points INT DEFAULT 0 COMMENT '积分：每充值100元获得1积分' AFTER member_level";
        if (query.exec(addPointsSql)) {
            qDebug() << "✓ 已添加points字段到users表";
        } else {
            qWarning() << "添加points字段失败:" << query.lastError().text();
        }
    }
    
    // 注意：优惠券现在存储在user_coupons表中，不再使用users表的coupon_30和coupon_50字段
    
    // 检查并添加phone_number字段（如果表已存在但字段不存在）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'phone_number'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addPhoneSql = "ALTER TABLE users ADD COLUMN phone_number VARCHAR(20) COMMENT '电话号码' AFTER email";
        if (query.exec(addPhoneSql)) {
            qDebug() << "✓ 已添加phone_number字段到users表";
        } else {
            qWarning() << "添加phone_number字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加address字段（如果表已存在但字段不存在）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'address'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addAddressSql = "ALTER TABLE users ADD COLUMN address VARCHAR(300) COMMENT '地址' AFTER phone_number";
        if (query.exec(addAddressSql)) {
            qDebug() << "✓ 已添加address字段到users表";
        } else {
            qWarning() << "添加address字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加membership_level字段（如果表已存在但字段不存在）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'membership_level'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        // 字段不存在，添加字段，类型为TINYINT，默认值为1
        QString addLevelSql = "ALTER TABLE users ADD COLUMN membership_level TINYINT DEFAULT 1 COMMENT '会员等级：1-普通，2-银卡，3-金卡，4-白金，5-钻石'";
        if (query.exec(addLevelSql)) {
            qDebug() << "✓ 已添加membership_level字段到users表";
            // 更新现有用户的membership_level值，默认为1
            query.exec("UPDATE users SET membership_level = 1 WHERE membership_level IS NULL");
        } else {
            qWarning() << "添加membership_level字段失败:" << query.lastError().text();
        }
    } else {
        // 字段已存在，检查是否为VARCHAR类型，如果是则转换为TINYINT
        query.prepare("SELECT DATA_TYPE FROM information_schema.COLUMNS "
                     "WHERE TABLE_SCHEMA = DATABASE() "
                     "AND TABLE_NAME = 'users' "
                     "AND COLUMN_NAME = 'membership_level'");
        if (query.exec() && query.next()) {
            QString dataType = query.value("DATA_TYPE").toString();
            if (dataType.toUpper() == "VARCHAR") {
                // 先将VARCHAR值转换为整数，然后修改字段类型
                query.exec("UPDATE users SET membership_level = CASE "
                          "WHEN membership_level = '普通' THEN 1 "
                          "WHEN membership_level = '银卡' THEN 2 "
                          "WHEN membership_level = '金卡' THEN 3 "
                          "WHEN membership_level = '白金' THEN 4 "
                          "WHEN membership_level = '钻石' THEN 5 "
                          "ELSE 1 END "
                          "WHERE membership_level IS NOT NULL");
                
                QString alterSql = "ALTER TABLE users MODIFY COLUMN membership_level TINYINT DEFAULT 1 COMMENT '会员等级：1-普通，2-银卡，3-金卡，4-白金，5-钻石'";
                if (query.exec(alterSql)) {
                    qDebug() << "✓ 已将membership_level字段类型从VARCHAR转换为TINYINT";
                } else {
                    qWarning() << "转换membership_level字段类型失败:" << query.lastError().text();
                }
            }
        }
    }
    
    // 如果存在旧的user_level字段，将其数据迁移到membership_level并删除旧字段
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'users' "
                 "AND COLUMN_NAME = 'user_level'");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        // 检查membership_level是否存在
        query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                     "WHERE TABLE_SCHEMA = DATABASE() "
                     "AND TABLE_NAME = 'users' "
                     "AND COLUMN_NAME = 'membership_level'");
        if (query.exec() && query.next() && query.value(0).toInt() == 0) {
            // user_level存在但membership_level不存在，重命名字段并转换类型
            query.exec("UPDATE users SET user_level = CASE "
                      "WHEN user_level = '普通' THEN 1 "
                      "WHEN user_level = '银卡' THEN 2 "
                      "WHEN user_level = '金卡' THEN 3 "
                      "WHEN user_level = '白金' THEN 4 "
                      "WHEN user_level = '钻石' THEN 5 "
                      "ELSE 1 END "
                      "WHERE user_level IS NOT NULL");
            
            QString renameSql = "ALTER TABLE users CHANGE COLUMN user_level membership_level TINYINT DEFAULT 1 COMMENT '会员等级：1-普通，2-银卡，3-金卡，4-白金，5-钻石'";
            if (query.exec(renameSql)) {
                qDebug() << "✓ 已将user_level字段重命名为membership_level并转换为TINYINT";
            } else {
                qWarning() << "重命名user_level字段失败:" << query.lastError().text();
            }
        } else {
            // 两个字段都存在，迁移数据后删除user_level
            query.exec("UPDATE users SET membership_level = CASE "
                      "WHEN user_level = '普通' THEN 1 "
                      "WHEN user_level = '银卡' THEN 2 "
                      "WHEN user_level = '金卡' THEN 3 "
                      "WHEN user_level = '白金' THEN 4 "
                      "WHEN user_level = '钻石' THEN 5 "
                      "ELSE COALESCE(membership_level, 1) END "
                      "WHERE user_level IS NOT NULL");
            
            query.exec("ALTER TABLE users DROP COLUMN user_level");
            qDebug() << "✓ 已迁移user_level数据到membership_level并删除旧字段";
        }
    }
    
    // 3. 商家表
    // sellers表中的字段与users表中除role字段外的字段一一对应
    QString createSellersTable = R"(
        CREATE TABLE IF NOT EXISTS sellers (
            seller_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '商家ID',
            seller_name VARCHAR(50) UNIQUE NOT NULL COMMENT '商家名称',
            password VARCHAR(100) NOT NULL COMMENT '密码',
            email VARCHAR(100) COMMENT '邮箱',
            phone_number VARCHAR(20) COMMENT '电话号码',
            address VARCHAR(300) COMMENT '地址',
            balance DECIMAL(10, 2) DEFAULT 0.00 COMMENT '账户余额',
            register_date DATE COMMENT '注册日期',
            status VARCHAR(20) DEFAULT '正常' COMMENT '账户状态',
            license_image_base64 TEXT COMMENT '营业执照图片(Base64编码)',
            INDEX idx_seller_name (seller_name)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='商家表'
    )";
    
    if (!query.exec(createSellersTable)) {
        qCritical() << "创建sellers表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ sellers表创建成功";
    
    // 修改现有表结构：将contact改为email，并添加license_image_base64字段
    // 检查是否存在contact字段，如果存在则重命名为email
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'sellers' AND COLUMN_NAME = 'contact'");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        // 如果存在contact字段，先重命名为email
        if (!query.exec("ALTER TABLE sellers CHANGE COLUMN contact email VARCHAR(100) COMMENT '邮箱'")) {
            qWarning() << "修改contact字段为email失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ contact字段已重命名为email";
        }
    }
    
    // 检查是否存在license_image_base64字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'sellers' AND COLUMN_NAME = 'license_image_base64'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        // 如果不存在license_image_base64字段，则添加
        if (!query.exec("ALTER TABLE sellers ADD COLUMN license_image_base64 TEXT COMMENT '营业执照图片(Base64编码)' AFTER address")) {
            qWarning() << "添加license_image_base64字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ license_image_base64字段已添加";
        }
    }
    
    // 检查是否存在phone_number字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'sellers' AND COLUMN_NAME = 'phone_number'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE sellers ADD COLUMN phone_number VARCHAR(20) COMMENT '电话号码' AFTER email")) {
            qWarning() << "添加phone_number字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ phone_number字段已添加到sellers表";
        }
    }
    
    // 检查是否存在address字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'sellers' AND COLUMN_NAME = 'address'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE sellers ADD COLUMN address VARCHAR(300) COMMENT '地址' AFTER phone_number")) {
            qWarning() << "添加address字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ address字段已添加到sellers表";
        }
    }
    
    // 检查是否存在balance字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'sellers' AND COLUMN_NAME = 'balance'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE sellers ADD COLUMN balance DECIMAL(10, 2) DEFAULT 0.00 COMMENT '账户余额' AFTER address")) {
            qWarning() << "添加balance字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ balance字段已添加到sellers表";
        }
    }
    
    // 检查并添加total_recharge字段（累计充值总额）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'sellers' "
                 "AND COLUMN_NAME = 'total_recharge'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addTotalRechargeSql = "ALTER TABLE sellers ADD COLUMN total_recharge DECIMAL(10, 2) DEFAULT 0.00 COMMENT '累计充值总额' AFTER balance";
        if (query.exec(addTotalRechargeSql)) {
            qDebug() << "✓ 已添加total_recharge字段到sellers表";
        } else {
            qWarning() << "添加total_recharge字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加member_level字段（会员等级）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'sellers' "
                 "AND COLUMN_NAME = 'member_level'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addMemberLevelSql = "ALTER TABLE sellers ADD COLUMN member_level VARCHAR(20) DEFAULT '普通会员' COMMENT '会员等级：普通会员、银卡会员、金卡会员、铂金会员、钻石会员、黑钻会员' AFTER total_recharge";
        if (query.exec(addMemberLevelSql)) {
            qDebug() << "✓ 已添加member_level字段到sellers表";
            query.exec("UPDATE sellers SET member_level = '普通会员' WHERE member_level IS NULL");
        } else {
            qWarning() << "添加member_level字段失败:" << query.lastError().text();
        }
    }
    
    // 检查并添加points字段（积分）
    query.prepare("SELECT COUNT(*) FROM information_schema.COLUMNS "
                 "WHERE TABLE_SCHEMA = DATABASE() "
                 "AND TABLE_NAME = 'sellers' "
                 "AND COLUMN_NAME = 'points'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        QString addPointsSql = "ALTER TABLE sellers ADD COLUMN points INT DEFAULT 0 COMMENT '积分：每充值100元获得1积分' AFTER member_level";
        if (query.exec(addPointsSql)) {
            qDebug() << "✓ 已添加points字段到sellers表";
        } else {
            qWarning() << "添加points字段失败:" << query.lastError().text();
        }
    }
    
    // 卖家认证表
    QString createSellerCertTable = R"(
        CREATE TABLE IF NOT EXISTS seller_certifications (
            cert_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '认证ID',
            user_id INT NOT NULL COMMENT '用户ID',
            username VARCHAR(50) NOT NULL COMMENT '用户名',
            password VARCHAR(100) NOT NULL COMMENT '密码',
            email VARCHAR(100) COMMENT '邮箱',
            license_image TEXT COMMENT '营业执照图片(Base64)',
            status VARCHAR(20) DEFAULT '审核中' COMMENT '认证状态：审核中/已认证/已拒绝',
            apply_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '申请时间',
            approve_time DATETIME COMMENT '审核时间',
            UNIQUE KEY unique_user_id (user_id),
            INDEX idx_status (status)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='卖家认证表'
    )";
    
    if (!query.exec(createSellerCertTable)) {
        qCritical() << "创建seller_certifications表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ seller_certifications表创建成功";
    
    // 插入默认商家账号
    // 由于卖家都是由买家认证而成的，所以初始化sellers表的时候，同时也要将同样的用户信息加入users表
    query.prepare("SELECT COUNT(*) FROM sellers WHERE seller_name = 'seller'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        // 先检查users表中是否已存在该用户
        query.prepare("SELECT COUNT(*) FROM users WHERE username = 'seller'");
        bool userExists = false;
        if (query.exec() && query.next()) {
            userExists = query.value(0).toInt() > 0;
        }
        
        // 如果users表中不存在，先添加到users表
        if (!userExists) {
            query.prepare("INSERT INTO users (username, password, email, phone_number, address, register_date, role) "
                         "VALUES ('seller', '123456', 'seller@example.com', NULL, NULL, CURDATE(), 2)");
            if (query.exec()) {
                qDebug() << "✓ 默认商家账号已添加到users表 (seller/123456, role=2)";
            } else {
                qWarning() << "添加默认商家账号到users表失败:" << query.lastError().text();
            }
        } else {
            // 如果已存在，更新role为2（卖家）
            query.prepare("UPDATE users SET role = 2 WHERE username = 'seller'");
            if (query.exec()) {
                qDebug() << "✓ 已更新默认商家账号的role为2";
            }
        }
        
        // 然后添加到sellers表，包含所有字段（与users表除role外一一对应）
        query.prepare("INSERT INTO sellers (seller_name, password, email, phone_number, address, balance, register_date, status) "
                     "VALUES ('seller', '123456', 'seller@example.com', NULL, NULL, 0.00, CURDATE(), '正常')");
        if (query.exec()) {
            qDebug() << "✓ 默认商家账号创建成功 (seller/123456)";
        } else {
            qWarning() << "添加默认商家账号到sellers表失败:" << query.lastError().text();
        }
    }
    
    // 4. 图书表
    QString createBooksTable = R"(
        CREATE TABLE IF NOT EXISTS books (
            isbn VARCHAR(50) PRIMARY KEY COMMENT '图书ISBN',
            title VARCHAR(200) NOT NULL COMMENT '书名',
            author VARCHAR(100) COMMENT '作者',
            category1 VARCHAR(50) COMMENT '一级分类',
            category2 VARCHAR(50) COMMENT '二级分类',
            merchant_id INT COMMENT '商家ID',
            price DECIMAL(10, 2) COMMENT '售价',
            stock INT DEFAULT 0 COMMENT '库存',
            status VARCHAR(20) DEFAULT '正常' COMMENT '状态',
            cover_image TEXT COMMENT '封面图片(Base64编码)',
            INDEX idx_title (title),
            INDEX idx_category1 (category1),
            INDEX idx_category2 (category2),
            INDEX idx_merchant_id (merchant_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='图书表'
    )";
    
    if (!query.exec(createBooksTable)) {
        qCritical() << "创建books表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ books表创建成功";
    
    // 修改现有表结构
    // 检查是否存在category字段，如果存在则重命名为category1
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'category'");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        if (!query.exec("ALTER TABLE books CHANGE COLUMN category category1 VARCHAR(50) COMMENT '一级分类'")) {
            qWarning() << "修改category字段为category1失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ category字段已重命名为category1";
        }
    }
    
    // 检查是否存在category2字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'category2'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE books ADD COLUMN category2 VARCHAR(50) COMMENT '二级分类' AFTER category1")) {
            qWarning() << "添加category2字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ category2字段已添加";
        }
    }
    
    // 检查是否存在merchant_id字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'merchant_id'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE books ADD COLUMN merchant_id INT COMMENT '商家ID' AFTER category2")) {
            qWarning() << "添加merchant_id字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ merchant_id字段已添加";
        }
    }
    
    // 检查是否存在description字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'description'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE books ADD COLUMN description TEXT COMMENT '书籍描述' AFTER cover_image")) {
            qWarning() << "添加description字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ description字段已添加";
        }
    }
    
    // 检查是否存在warning_stock字段，如果存在则删除
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'warning_stock'");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        if (!query.exec("ALTER TABLE books DROP COLUMN warning_stock")) {
            qWarning() << "删除warning_stock字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ warning_stock字段已删除";
        }
    }
    
    // 检查是否存在cost字段，如果存在则删除
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'cost'");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        if (!query.exec("ALTER TABLE books DROP COLUMN cost")) {
            qWarning() << "删除cost字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ cost字段已删除";
        }
    }
    
    // 检查是否存在cover_image字段，如果不存在则添加
    query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                  "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'books' AND COLUMN_NAME = 'cover_image'");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("ALTER TABLE books ADD COLUMN cover_image TEXT COMMENT '封面图片(Base64编码)' AFTER status")) {
            qWarning() << "添加cover_image字段失败:" << query.lastError().text();
        } else {
            qDebug() << "✓ cover_image字段已添加";
        }
    }
    
    // 5. 订单表
    QString createOrdersTable = R"(
        CREATE TABLE IF NOT EXISTS orders (
            order_id VARCHAR(50) PRIMARY KEY COMMENT '订单ID',
            user_id INT COMMENT '用户ID',
            merchant_id INT COMMENT '商家ID（订单中主要商家，用于卖家筛选订单）',
            customer VARCHAR(100) COMMENT '客户姓名',
            phone VARCHAR(20) COMMENT '联系电话',
            total_amount DECIMAL(10, 2) COMMENT '订单总额',
            status VARCHAR(20) DEFAULT '待支付' COMMENT '订单状态：待支付/已支付/已发货/已取消',
            payment_method VARCHAR(50) COMMENT '支付方式',
            order_date DATETIME COMMENT '下单时间',
            pay_time DATETIME COMMENT '支付时间',
            ship_time DATETIME COMMENT '发货时间',
            cancel_time DATETIME COMMENT '取消时间',
            cancel_reason VARCHAR(200) COMMENT '取消原因',
            tracking_number VARCHAR(100) COMMENT '物流单号',
            address VARCHAR(300) COMMENT '收货地址',
            operator VARCHAR(50) DEFAULT '系统' COMMENT '操作员',
            remark TEXT COMMENT '备注',
            items TEXT COMMENT '订单项(JSON格式，包含bookId、bookName、quantity、price、merchantId等)',
            INDEX idx_user_id (user_id),
            INDEX idx_merchant_id (merchant_id),
            INDEX idx_order_date (order_date),
            INDEX idx_status (status)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='订单表'
    )";
    
    if (!query.exec(createOrdersTable)) {
        qCritical() << "创建orders表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ orders表创建成功（包含merchant_id字段和索引）";
    
    // 6. 购物车表
    QString createCartTable = R"(
        CREATE TABLE IF NOT EXISTS cart (
            cart_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '购物车ID',
            user_id INT NOT NULL COMMENT '用户ID',
            book_id VARCHAR(50) NOT NULL COMMENT '图书ID',
            quantity INT DEFAULT 1 COMMENT '数量',
            add_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
            INDEX idx_user_id (user_id),
            UNIQUE KEY unique_user_book (user_id, book_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='购物车表'
    )";
    
    if (!query.exec(createCartTable)) {
        qCritical() << "创建cart表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ cart表创建成功";
    
    // 6.5. 收藏表
    QString createFavoritesTable = R"(
        CREATE TABLE IF NOT EXISTS favorites (
            favorite_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '收藏ID',
            user_id INT NOT NULL COMMENT '用户ID',
            book_id VARCHAR(50) NOT NULL COMMENT '图书ID',
            add_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '收藏时间',
            INDEX idx_user_id (user_id),
            INDEX idx_book_id (book_id),
            UNIQUE KEY unique_user_book (user_id, book_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='收藏表'
    )";
    
    if (!query.exec(createFavoritesTable)) {
        qCritical() << "创建favorites表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ favorites表创建成功";
    
    // 7. 会员表
    QString createMembersTable = R"(
        CREATE TABLE IF NOT EXISTS members (
            card_no VARCHAR(50) PRIMARY KEY COMMENT '会员卡号',
            name VARCHAR(100) COMMENT '会员姓名',
            phone VARCHAR(20) COMMENT '联系电话',
            level VARCHAR(20) DEFAULT '普通' COMMENT '会员等级',
            balance DECIMAL(10, 2) DEFAULT 0.00 COMMENT '余额',
            points INT DEFAULT 0 COMMENT '积分',
            create_date DATE COMMENT '创建日期',
            INDEX idx_phone (phone)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='会员表'
    )";
    
    if (!query.exec(createMembersTable)) {
        qCritical() << "创建members表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ members表创建成功";
    
    // 8. 卖家申诉表
    QString createSellerAppealsTable = R"(
        CREATE TABLE IF NOT EXISTS seller_appeals (
            appeal_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '申诉ID',
            seller_id INT NOT NULL COMMENT '商家ID',
            seller_name VARCHAR(50) NOT NULL COMMENT '商家名称',
            appeal_reason TEXT NOT NULL COMMENT '申诉理由',
            status VARCHAR(20) DEFAULT '待审核' COMMENT '申诉状态：待审核/已通过/未通过',
            submit_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '提交时间',
            review_time DATETIME COMMENT '审核时间',
            reviewer_id INT COMMENT '审核人ID',
            review_comment TEXT COMMENT '审核意见',
            INDEX idx_seller_id (seller_id),
            INDEX idx_status (status)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='卖家申诉表'
    )";
    
    if (!query.exec(createSellerAppealsTable)) {
        qCritical() << "创建seller_appeals表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ seller_appeals表创建成功";
    
    // 9. 聊天消息表
    QString createChatMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            message_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '消息ID',
            sender_id INT NOT NULL COMMENT '发送者ID',
            sender_type VARCHAR(20) NOT NULL COMMENT '发送者类型：buyer/seller/admin',
            receiver_id INT COMMENT '接收者ID（NULL表示发送给客服/管理员）',
            receiver_type VARCHAR(20) COMMENT '接收者类型：buyer/seller/admin',
            message_content TEXT NOT NULL COMMENT '消息内容',
            send_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '发送时间',
            is_read TINYINT DEFAULT 0 COMMENT '是否已读：0-未读，1-已读',
            INDEX idx_sender (sender_id, sender_type),
            INDEX idx_receiver (receiver_id, receiver_type),
            INDEX idx_send_time (send_time)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='聊天消息表'
    )";
    
    if (!query.exec(createChatMessagesTable)) {
        qCritical() << "创建chat_messages表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ chat_messages表创建成功";
    
    // 10. 商品评论表
    QString createReviewsTable = R"(
        CREATE TABLE IF NOT EXISTS reviews (
            review_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '评论ID',
            user_id INT NOT NULL COMMENT '用户ID',
            book_id VARCHAR(50) NOT NULL COMMENT '商品ID（ISBN）',
            rating INT NOT NULL COMMENT '评分（1-5分）',
            comment TEXT COMMENT '评论内容',
            review_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '评论时间',
            INDEX idx_book_id (book_id),
            INDEX idx_user_id (user_id),
            INDEX idx_review_time (review_time),
            UNIQUE KEY unique_user_book (user_id, book_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='商品评论表'
    )";
    
    if (!query.exec(createReviewsTable)) {
        qCritical() << "创建reviews表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ reviews表创建成功";
    
    // 11. 用户优惠券表
    QString createUserCouponsTable = R"(
        CREATE TABLE IF NOT EXISTS user_coupons (
            coupon_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '优惠券ID',
            user_id INT NOT NULL COMMENT '用户ID',
            coupon_type VARCHAR(50) NOT NULL COMMENT '优惠券类型：30元优惠券、50元优惠券',
            coupon_value DECIMAL(10, 2) NOT NULL COMMENT '优惠券面额',
            status VARCHAR(20) DEFAULT '未使用' COMMENT '状态：未使用/已使用/已过期',
            obtain_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '获得时间',
            expire_time DATETIME COMMENT '过期时间',
            use_time DATETIME COMMENT '使用时间',
            order_id VARCHAR(100) COMMENT '使用的订单ID',
            INDEX idx_user_id (user_id),
            INDEX idx_status (status)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户优惠券表'
    )";
    
    if (!query.exec(createUserCouponsTable)) {
        qCritical() << "创建user_coupons表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✓ user_coupons表创建成功";
    
    qDebug() << "========================================";
    qDebug() << "所有数据库表创建成功！";
    qDebug() << "========================================";
    
    // 注意：示例图书数据初始化已移至窗口显示后异步执行，避免阻塞UI
    
    return true;
}

// ==========================================
// 请求日志功能
// ==========================================

bool Database::logRequest(const QString& clientIp, quint16 clientPort, 
                         const QString& action, const QJsonObject& requestData,
                         const QJsonObject& responseData, bool success, 
                         const QString& category)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO request_logs (client_ip, client_port, action, request_data, "
                 "response_data, success, category) VALUES (?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(clientIp);
    query.addBindValue(clientPort);
    query.addBindValue(action);
    query.addBindValue(QString(QJsonDocument(requestData).toJson(QJsonDocument::Compact)));
    query.addBindValue(QString(QJsonDocument(responseData).toJson(QJsonDocument::Compact)));
    query.addBindValue(success);
    query.addBindValue(category);
    
    if (!query.exec()) {
        qWarning() << "记录请求日志失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QJsonArray Database::getRequestLogs(int limit, const QString& category)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray logs;
    
    if (!isConnected()) {
        return logs;
    }
    
    QSqlQuery query(m_db);
    QString sql = "SELECT * FROM request_logs ";
    if (!category.isEmpty()) {
        sql += "WHERE category = '" + category + "' ";
    }
    sql += "ORDER BY timestamp DESC LIMIT " + QString::number(limit);
    
    if (!query.exec(sql)) {
        qWarning() << "获取请求日志失败:" << query.lastError().text();
        return logs;
    }
    
    while (query.next()) {
        QJsonObject log;
        log["id"] = query.value("id").toInt();
        log["timestamp"] = query.value("timestamp").toString();
        log["clientIp"] = query.value("client_ip").toString();
        log["clientPort"] = query.value("client_port").toInt();
        log["action"] = query.value("action").toString();
        log["success"] = query.value("success").toBool();
        log["category"] = query.value("category").toString();
        logs.append(log);
    }
    
    return logs;
}

// ==========================================
// 用户相关
// ==========================================

bool Database::registerUser(const QString& username, const QString& password, const QString& email)
{
    QMutexLocker locker(&m_mutex);
    
    // #region agent log
    writeDebugLog("data.cpp:330", "数据库注册函数入口", QJsonObject{{"username", username}, {"email", email}}, "G");
    // #endregion
    
    if (!isConnected()) {
        // #region agent log
        writeDebugLog("data.cpp:335", "数据库未连接", QJsonObject{}, "G");
        // #endregion
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, password, email, phone_number, address, register_date, license_image_base64, role) "
                 "VALUES (?, ?, ?, NULL, NULL, CURDATE(), NULL, 1)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(email.isEmpty() ? QString("%1@example.com").arg(username) : email);
    
    // #region agent log
    writeDebugLog("data.cpp:345", "执行数据库插入前", QJsonObject{{"username", username}}, "G");
    // #endregion
    
    if (!query.exec()) {
        // #region agent log
        writeDebugLog("data.cpp:347", "数据库插入失败", QJsonObject{{"username", username}, {"error", query.lastError().text()}}, "G");
        // #endregion
        qWarning() << "注册用户失败:" << query.lastError().text();
        return false;
    }
    
    // #region agent log
    writeDebugLog("data.cpp:351", "数据库插入成功", QJsonObject{{"username", username}}, "G");
    // #endregion
    qDebug() << "用户注册成功:" << username;
    return true;
}

QJsonObject Database::loginUser(const QString& username, const QString& password)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject result;
    
    // #region agent log
    writeDebugLog("data.cpp:354", "数据库登录函数入口", QJsonObject{{"username", username}}, "H");
    // #endregion
    
    if (!isConnected()) {
        // #region agent log
        writeDebugLog("data.cpp:360", "数据库未连接", QJsonObject{}, "H");
        // #endregion
        result["success"] = false;
        result["message"] = "数据库未连接";
        return result;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    
    // #region agent log
    writeDebugLog("data.cpp:369", "执行数据库查询前", QJsonObject{{"username", username}}, "H");
    // #endregion
    
    if (!query.exec()) {
        // #region agent log
        writeDebugLog("data.cpp:371", "数据库查询失败", QJsonObject{{"username", username}, {"error", query.lastError().text()}}, "H");
        // #endregion
        qWarning() << "查询用户失败:" << query.lastError().text();
        result["success"] = false;
        result["message"] = "查询失败";
        return result;
    }
    
    if (query.next()) {
        // #region agent log
        writeDebugLog("data.cpp:377", "数据库查询成功，找到用户", QJsonObject{{"username", username}, {"userId", query.value("user_id").toInt()}}, "H");
        // #endregion
        
        // 允许被封禁的用户登录，但不限制其登录功能
        // 封禁状态将在购买图书等功能中检查
        
        result["success"] = true;
        result["message"] = "登录成功";
        result["userId"] = query.value("user_id").toInt();
        result["username"] = query.value("username").toString();
        result["email"] = query.value("email").toString();
        result["balance"] = query.value("balance").toDouble();
        result["role"] = query.value("role").toInt();
        result["userType"] = "buyer";
        // 返回电话和地址信息（直接读取字段值，即使为NULL也会返回空字符串）
        QString phone = query.value("phone_number").toString();
        QString address = query.value("address").toString();
        if (!phone.isEmpty()) {
            result["phone"] = phone;
        }
        if (!address.isEmpty()) {
            result["address"] = address;
        }
        qDebug() << "登录返回用户信息 - 用户ID:" << query.value("user_id").toInt() << "电话:" << phone << "地址:" << address;
        
        // 读取会员等级、累计充值总额和积分
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        int points = 0;
        if (query.record().indexOf("member_level") >= 0) {
            memberLevel = query.value("member_level").toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        result["memberLevel"] = memberLevel;
        result["totalRecharge"] = totalRecharge;
        result["points"] = points;
        result["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        result["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
        
        // 从user_coupons表获取优惠券数量（直接查询，避免死锁）
        int userId = query.value("user_id").toInt();
        int coupon30 = 0;
        int coupon50 = 0;
        
        // 查询30元优惠券数量
        QSqlQuery coupon30Query(m_db);
        coupon30Query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 30.0 AND status = '未使用'");
        coupon30Query.addBindValue(userId);
        if (coupon30Query.exec() && coupon30Query.next()) {
            coupon30 = coupon30Query.value("count").toInt();
        }
        
        // 查询50元优惠券数量
        QSqlQuery coupon50Query(m_db);
        coupon50Query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 50.0 AND status = '未使用'");
        coupon50Query.addBindValue(userId);
        if (coupon50Query.exec() && coupon50Query.next()) {
            coupon50 = coupon50Query.value("count").toInt();
        }
        
        result["coupon30"] = coupon30;
        result["coupon50"] = coupon50;
        
        // 检查是否有营业执照图片
        QString licenseImage = query.value("license_image_base64").toString();
        if (!licenseImage.isEmpty()) {
            result["hasLicenseImage"] = true;
        }
        // 获取用户收藏的书籍列表（直接查询，避免死锁）
        QJsonArray favoriteBooks;
        QSqlQuery favoriteQuery(m_db);
        favoriteQuery.prepare("SELECT book_id FROM favorites WHERE user_id = ? ORDER BY add_time DESC");
        favoriteQuery.addBindValue(userId);
        if (favoriteQuery.exec()) {
            while (favoriteQuery.next()) {
                favoriteBooks.append(favoriteQuery.value("book_id").toString());
            }
        }
        result["favoriteBooks"] = favoriteBooks;
        qDebug() << "✓ 买家登录成功:" << username << "ID:" << userId << "Role:" << query.value("role").toInt() << "收藏数:" << favoriteBooks.size();
    } else {
        // #region agent log
        writeDebugLog("data.cpp:388", "数据库查询成功，但未找到用户", QJsonObject{{"username", username}}, "H");
        // #endregion
        result["success"] = false;
        result["message"] = "用户名或密码错误";
    }
    
    return result;
}

bool Database::changePassword(int userId, const QString& oldPassword, const QString& newPassword)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接";
        return false;
    }
    
    QSqlQuery query(m_db);
    
    // 首先验证旧密码是否正确
    query.prepare("SELECT user_id FROM users WHERE user_id = ? AND password = ?");
    query.addBindValue(userId);
    query.addBindValue(oldPassword);
    
    if (!query.exec()) {
        qWarning() << "验证旧密码失败:" << query.lastError().text();
        return false;
    }
    
    if (!query.next()) {
        qDebug() << "旧密码验证失败，用户ID:" << userId;
        return false;  // 旧密码不正确
    }
    
    // 更新密码
    query.prepare("UPDATE users SET password = ? WHERE user_id = ?");
    query.addBindValue(newPassword);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新密码失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "✓ 密码修改成功，用户ID:" << userId;
    return true;
}

QJsonArray Database::getAllUsers()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray users;
    
    if (!isConnected()) {
        return users;
    }
    
    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM users ORDER BY user_id")) {
        qWarning() << "查询用户列表失败:" << query.lastError().text();
        return users;
    }
    
    while (query.next()) {
        QJsonObject user;
        user["userId"] = query.value("user_id").toInt();
        user["username"] = query.value("username").toString();
        user["email"] = query.value("email").toString();
        user["balance"] = query.value("balance").toDouble();
        user["registerDate"] = query.value("register_date").toString();
        user["status"] = query.value("status").toString();
        user["role"] = query.value("role").toInt();
        
        // 读取会员等级和累计充值总额
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        if (query.record().indexOf("member_level") >= 0) {
            QVariant memberLevelVar = query.value("member_level");
            if (!memberLevelVar.isNull() && memberLevelVar.isValid()) {
                memberLevel = memberLevelVar.toString();
            }
            // 如果memberLevel为空或NULL，使用默认值
            if (memberLevel.isEmpty() || memberLevel.isNull()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        // 读取积分
        int points = 0;
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        
        user["memberLevel"] = memberLevel;
        user["totalRecharge"] = totalRecharge;
        user["points"] = points;
        user["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        user["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
        
        // 读取会员等级（TINYINT类型，1-5）- 保留向后兼容
        int membershipLevel = 1;
        if (query.record().indexOf("membership_level") >= 0) {
            membershipLevel = query.value("membership_level").toInt();
            if (membershipLevel < 1 || membershipLevel > 5) {
                membershipLevel = 1;  // 确保值在有效范围内
            }
        }
        user["membershipLevel"] = membershipLevel;
        // 检查是否有营业执照图片
        QString licenseImage = query.value("license_image_base64").toString();
        if (!licenseImage.isEmpty()) {
            user["hasLicenseImage"] = true;
        }
        users.append(user);
    }
    
    return users;
}

QJsonObject Database::getUserById(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject user;
    
    if (!isConnected()) {
        qWarning() << "getUserById: 数据库未连接，用户ID:" << userId;
        return user;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "getUserById: 查询用户失败，用户ID:" << userId << "错误:" << query.lastError().text();
        return user;
    }
    
    if (query.next()) {
        qDebug() << "getUserById: 找到用户，用户ID:" << userId;
        user["userId"] = query.value("user_id").toInt();
        user["username"] = query.value("username").toString();
        user["email"] = query.value("email").toString();
        // 确保balance字段总是存在，即使为NULL也返回0.0
        QVariant balanceValue = query.value("balance");
        if (balanceValue.isNull() || !balanceValue.isValid()) {
            user["balance"] = 0.0;
            qWarning() << "用户余额为NULL，用户ID:" << query.value("user_id").toInt() << "设置为0.0";
        } else {
            user["balance"] = balanceValue.toDouble();
        }
        user["registerDate"] = query.value("register_date").toString();
        user["status"] = query.value("status").toString();
        user["role"] = query.value("role").toInt();
        
        // 读取会员等级、累计充值总额和积分
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        int points = 0;
        if (query.record().indexOf("member_level") >= 0) {
            memberLevel = query.value("member_level").toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        user["memberLevel"] = memberLevel;
        user["totalRecharge"] = totalRecharge;
        user["points"] = points;
        user["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        user["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
        
        // 从user_coupons表获取优惠券数量（直接查询，避免死锁）
        int coupon30 = 0;
        int coupon50 = 0;
        
        // 查询30元优惠券数量
        QSqlQuery coupon30Query(m_db);
        coupon30Query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 30.0 AND status = '未使用'");
        coupon30Query.addBindValue(userId);
        if (coupon30Query.exec() && coupon30Query.next()) {
            coupon30 = coupon30Query.value("count").toInt();
        }
        
        // 查询50元优惠券数量
        QSqlQuery coupon50Query(m_db);
        coupon50Query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 50.0 AND status = '未使用'");
        coupon50Query.addBindValue(userId);
        if (coupon50Query.exec() && coupon50Query.next()) {
            coupon50 = coupon50Query.value("count").toInt();
        }
        
        user["coupon30"] = coupon30;
        user["coupon50"] = coupon50;
        
        // 保留向后兼容
        int membershipLevel = 1;
        if (query.record().indexOf("membership_level") >= 0) {
            membershipLevel = query.value("membership_level").toInt();
            if (membershipLevel < 1 || membershipLevel > 5) {
                membershipLevel = 1;
            }
        }
        user["membershipLevel"] = membershipLevel;
        
        qDebug() << "getUserById: 成功获取用户信息，用户ID:" << userId << "余额:" << user.value("balance").toDouble();
    } else {
        qWarning() << "getUserById: 未找到用户，用户ID:" << userId;
        // 检查数据库中是否存在该用户
        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT COUNT(*) as count FROM users WHERE user_id = ?");
        checkQuery.addBindValue(userId);
        if (checkQuery.exec() && checkQuery.next()) {
            int count = checkQuery.value("count").toInt();
            qWarning() << "getUserById: 数据库中用户ID" << userId << "的记录数:" << count;
        }
    }
    
    qDebug() << "getUserById: 返回用户对象，是否为空:" << user.isEmpty() << "包含的字段:" << user.keys();
    return user;
}

bool Database::deleteUser(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "删除用户失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::updateUserStatus(int userId, const QString& status)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 先获取用户信息，检查是否为商家
    QSqlQuery userQuery(m_db);
    userQuery.prepare("SELECT username, role FROM users WHERE user_id = ?");
    userQuery.addBindValue(userId);
    
    QString username;
    int role = 1;
    if (userQuery.exec() && userQuery.next()) {
        username = userQuery.value("username").toString();
        role = userQuery.value("role").toInt();
    }
    
    // 更新用户状态
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET status = ? WHERE user_id = ?");
    query.addBindValue(status);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新用户状态失败:" << query.lastError().text();
        return false;
    }
    
    // 如果用户是商家（role = 2），同步更新商家状态
    if (role == 2 && !username.isEmpty()) {
        qDebug() << "检测到用户" << userId << "(" << username << ")是商家，开始同步更新商家状态";
        
        // 通过username找到对应的seller_id
        QSqlQuery sellerQuery(m_db);
        sellerQuery.prepare("SELECT seller_id FROM sellers WHERE seller_name = ?");
        sellerQuery.addBindValue(username);
        
        if (sellerQuery.exec()) {
            if (sellerQuery.next()) {
                int sellerId = sellerQuery.value("seller_id").toInt();
                qDebug() << "找到对应的商家ID:" << sellerId << "，准备更新状态为:" << status;
                
                // 直接更新商家状态（mutex已经锁定，不需要再次锁定）
                QSqlQuery updateSellerQuery(m_db);
                updateSellerQuery.prepare("UPDATE sellers SET status = ? WHERE seller_id = ?");
                updateSellerQuery.addBindValue(status);
                updateSellerQuery.addBindValue(sellerId);
                
                if (updateSellerQuery.exec()) {
                    qDebug() << "商家状态更新成功，sellerId:" << sellerId << "status:" << status;
                    
                    // 如果封禁用户，将该商家的所有图书下架
                    if (status == "封禁") {
                        QSqlQuery updateBooksQuery(m_db);
                        updateBooksQuery.prepare("UPDATE books SET status = ? WHERE merchant_id = ?");
                        updateBooksQuery.addBindValue("下架");
                        updateBooksQuery.addBindValue(sellerId);
                        
                        if (updateBooksQuery.exec()) {
                            int affectedRows = updateBooksQuery.numRowsAffected();
                            qDebug() << "用户" << userId << "(" << username << ")被封禁，同步封禁商家ID" << sellerId << "，其" << affectedRows << "本图书已下架";
                        } else {
                            qWarning() << "批量更新商家图书状态失败:" << updateBooksQuery.lastError().text();
                        }
                    } else if (status == "正常") {
                        // 解封用户时，恢复该商家的所有图书上架
                        QSqlQuery updateBooksQuery(m_db);
                        updateBooksQuery.prepare("UPDATE books SET status = ? WHERE merchant_id = ?");
                        updateBooksQuery.addBindValue("正常");
                        updateBooksQuery.addBindValue(sellerId);
                        
                        if (updateBooksQuery.exec()) {
                            int affectedRows = updateBooksQuery.numRowsAffected();
                            qDebug() << "用户" << userId << "(" << username << ")已解封，商家ID" << sellerId << "的" << affectedRows << "本图书已恢复上架";
                        } else {
                            qWarning() << "恢复商家图书上架失败:" << updateBooksQuery.lastError().text();
                        }
                    }
                } else {
                    qWarning() << "更新商家状态失败，商家ID:" << sellerId << "错误:" << updateSellerQuery.lastError().text();
                }
            } else {
                qWarning() << "未找到对应的商家，username:" << username;
            }
        } else {
            qWarning() << "查询商家失败:" << sellerQuery.lastError().text();
        }
    } else {
        if (role != 2) {
            qDebug() << "用户" << userId << "不是商家（role=" << role << "），无需同步更新商家状态";
        } else if (username.isEmpty()) {
            qWarning() << "用户" << userId << "的username为空，无法查找对应商家";
        }
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::updateUserBalance(int userId, double balance)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET balance = ? WHERE user_id = ?");
    query.addBindValue(balance);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新用户余额失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "用户余额已更新，用户ID:" << userId << "新余额:" << balance;
    return true;
}

bool Database::updateUserInfo(int userId, const QString& phone, const QString& email, const QString& address)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "更新用户信息失败：数据库未连接";
        return false;
    }
    
    // 先检查用户是否存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT user_id FROM users WHERE user_id = ?");
    checkQuery.addBindValue(userId);
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "更新用户信息失败：用户不存在，用户ID:" << userId;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET phone_number = ?, email = ?, address = ? WHERE user_id = ?");
    query.addBindValue(phone.isEmpty() ? QVariant() : phone);  // 空字符串转为NULL
    query.addBindValue(email.isEmpty() ? QVariant() : email);
    query.addBindValue(address.isEmpty() ? QVariant() : address);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新用户信息失败:" << query.lastError().text();
        qWarning() << "SQL错误详情:" << query.lastError().databaseText();
        return false;
    }
    
    // 检查是否有行被更新
    if (query.numRowsAffected() == 0) {
        qWarning() << "更新用户信息失败：没有行被更新，用户ID:" << userId;
        return false;
    }
    
    qDebug() << "更新用户信息成功，用户ID:" << userId << "电话:" << phone << "邮箱:" << email << "地址:" << address;
    qDebug() << "受影响的行数:" << query.numRowsAffected();
    return true;
}

bool Database::updateUserMemberLevel(int userId, const QString& memberLevel)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "更新会员等级失败：数据库未连接";
        return false;
    }
    
    // 先检查用户是否存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT user_id FROM users WHERE user_id = ?");
    checkQuery.addBindValue(userId);
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "更新会员等级失败：用户不存在，用户ID:" << userId;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET member_level = ? WHERE user_id = ?");
    query.addBindValue(memberLevel);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新会员等级失败:" << query.lastError().text();
        return false;
    }
    
    // 检查是否有行被更新
    if (query.numRowsAffected() == 0) {
        qWarning() << "更新会员等级失败：没有行被更新，用户ID:" << userId;
        return false;
    }
    
    qDebug() << "更新会员等级成功，用户ID:" << userId << "会员等级:" << memberLevel;
    return true;
}

bool Database::rechargeUserBalance(int userId, double amount)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    if (amount <= 0) {
        qWarning() << "充值金额必须大于0，用户ID:" << userId << "金额:" << amount;
        return false;
    }
    
    // 计算本次充值获得的积分（每100元1积分）
    int pointsToAdd = calculatePoints(amount);
    
    QSqlQuery query(m_db);
    // 同时更新余额、累计充值总额和积分
    query.prepare("UPDATE users SET balance = balance + ?, total_recharge = total_recharge + ?, points = points + ? WHERE user_id = ?");
    query.addBindValue(amount);
    query.addBindValue(amount);
    query.addBindValue(pointsToAdd);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "充值用户余额失败:" << query.lastError().text();
        return false;
    }
    
    // 更新会员等级（使用不获取锁的版本，因为已经持有锁）
    updateMemberLevelUnlocked(userId);
    
    // 获取更新后的余额
    QSqlQuery selectQuery(m_db);
    selectQuery.prepare("SELECT balance, points FROM users WHERE user_id = ?");
    selectQuery.addBindValue(userId);
    if (selectQuery.exec() && selectQuery.next()) {
        double newBalance = selectQuery.value("balance").toDouble();
        int newPoints = selectQuery.value("points").toInt();
        qDebug() << "用户余额充值成功，用户ID:" << userId << "充值金额:" << amount 
                 << "新余额:" << newBalance << "获得积分:" << pointsToAdd << "总积分:" << newPoints;
    }
    
    return true;
}

bool Database::deductUserBalance(int userId, double amount)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    if (amount <= 0) {
        qWarning() << "扣除金额必须大于0，用户ID:" << userId << "金额:" << amount;
        return false;
    }
    
    // 先检查余额是否足够
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT balance FROM users WHERE user_id = ?");
    checkQuery.addBindValue(userId);
    
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "查询用户余额失败，用户ID:" << userId;
        return false;
    }
    
    double currentBalance = checkQuery.value("balance").toDouble();
    if (currentBalance < amount) {
        qWarning() << "用户余额不足，用户ID:" << userId << "当前余额:" << currentBalance << "需要扣除:" << amount;
        return false;
    }
    
    // 扣除余额
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET balance = balance - ? WHERE user_id = ?");
    query.addBindValue(amount);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "扣除用户余额失败:" << query.lastError().text();
        return false;
    }
    
    double newBalance = currentBalance - amount;
    qDebug() << "用户余额扣除成功，用户ID:" << userId << "扣除金额:" << amount << "新余额:" << newBalance;
    return true;
}

// ==========================================
// 商家相关
// ==========================================

bool Database::registerSeller(const QString& sellerName, const QString& password, const QString& email)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO sellers (seller_name, password, email, phone_number, address, balance, register_date) "
                 "VALUES (?, ?, ?, NULL, NULL, 0.00, CURDATE())");
    query.addBindValue(sellerName);
    query.addBindValue(password);
    query.addBindValue(email);
    
    if (!query.exec()) {
        qWarning() << "注册商家失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QJsonObject Database::loginSeller(const QString& sellerName, const QString& password)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject result;
    
    if (!isConnected()) {
        result["success"] = false;
        result["message"] = "数据库未连接";
        return result;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM sellers WHERE seller_name = ? AND password = ?");
    query.addBindValue(sellerName);
    query.addBindValue(password);
    
    qDebug() << "=== 商家登录查询 ===";
    qDebug() << "商家名称:" << sellerName;
    qDebug() << "SQL查询准备完成";
    
    if (!query.exec()) {
        qWarning() << "查询商家失败:" << query.lastError().text();
        qWarning() << "SQL错误详情:" << query.lastError().databaseText();
        result["success"] = false;
        result["message"] = "查询失败: " + query.lastError().text();
        return result;
    }
    
    if (query.next()) {
        QVariant sellerIdVar = query.value("seller_id");
        int sellerId = sellerIdVar.toInt();
        QString sellerNameFromDb = query.value("seller_name").toString();
        QString emailFromDb = query.value("email").toString();
        
        qDebug() << "✓ 查询到商家记录";
        qDebug() << "seller_id (原始值):" << sellerIdVar.toString();
        qDebug() << "seller_id (整数):" << sellerId;
        qDebug() << "seller_name:" << sellerNameFromDb;
        qDebug() << "email:" << emailFromDb;
        
        // 确保seller_id有效
        if (sellerId <= 0) {
            qWarning() << "✗ 警告：商家ID无效:" << sellerId << "（原始值:" << sellerIdVar.toString() << ")";
            result["success"] = false;
            result["message"] = QString("商家ID无效（ID: %1），请联系管理员检查数据库").arg(sellerId);
            return result;
        }
        
        result["success"] = true;
        result["message"] = "登录成功";
        result["userId"] = sellerId;  // 直接使用整数，JSON会自动序列化
        result["username"] = query.value("seller_name").toString();
        result["email"] = query.value("email").toString();
        result["userType"] = "seller";
        
        // 读取会员等级、累计充值总额和积分
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        int points = 0;
        if (query.record().indexOf("member_level") >= 0) {
            memberLevel = query.value("member_level").toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        result["memberLevel"] = memberLevel;
        result["totalRecharge"] = totalRecharge;
        result["points"] = points;
        result["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        result["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
        
        qDebug() << "✓ 返回登录结果，userId:" << sellerId << "会员等级:" << memberLevel << "积分:" << points;
    } else {
        result["success"] = false;
        result["message"] = "用户名或密码错误";
    }
    
    return result;
}

QJsonArray Database::getAllSellers()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray sellers;
    
    if (!isConnected()) {
        return sellers;
    }
    
    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM sellers ORDER BY seller_id")) {
        qWarning() << "查询商家列表失败:" << query.lastError().text();
        return sellers;
    }
    
    while (query.next()) {
        QJsonObject seller;
        seller["sellerId"] = query.value("seller_id").toInt();
        seller["sellerName"] = query.value("seller_name").toString();
        seller["email"] = query.value("email").toString();
        seller["phoneNumber"] = query.value("phone_number").toString();
        seller["address"] = query.value("address").toString();
        seller["balance"] = query.value("balance").toDouble();
        seller["registerDate"] = query.value("register_date").toString();
        seller["status"] = query.value("status").toString();
        
        // 读取会员等级、累计充值总额和积分
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        int points = 0;
        if (query.record().indexOf("member_level") >= 0) {
            QVariant memberLevelVar = query.value("member_level");
            if (!memberLevelVar.isNull() && memberLevelVar.isValid()) {
                memberLevel = memberLevelVar.toString();
            }
            // 如果memberLevel为空或NULL，使用默认值
            if (memberLevel.isEmpty() || memberLevel.isNull()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        seller["memberLevel"] = memberLevel;
        seller["totalRecharge"] = totalRecharge;
        seller["points"] = points;
        seller["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        seller["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
        
        // 检查是否有营业执照图片
        QString licenseImage = query.value("license_image_base64").toString();
        if (!licenseImage.isEmpty()) {
            seller["hasLicenseImage"] = true;
        }
        sellers.append(seller);
    }
    
    return sellers;
}

QJsonObject Database::getSellerById(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject seller;
    
    if (!isConnected()) {
        return seller;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM sellers WHERE seller_id = ?");
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "查询商家失败:" << query.lastError().text();
        return seller;
    }
    
    if (query.next()) {
        seller["sellerId"] = query.value("seller_id").toInt();
        seller["sellerName"] = query.value("seller_name").toString();
        seller["email"] = query.value("email").toString();
        seller["phoneNumber"] = query.value("phone_number").toString();
        seller["address"] = query.value("address").toString();
        seller["balance"] = query.value("balance").toDouble();
        seller["registerDate"] = query.value("register_date").toString();
        seller["status"] = query.value("status").toString();
        
        // 读取会员等级、累计充值总额和积分
        QString memberLevel = "普通会员";
        double totalRecharge = 0.0;
        int points = 0;
        if (query.record().indexOf("member_level") >= 0) {
            memberLevel = query.value("member_level").toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
        }
        if (query.record().indexOf("total_recharge") >= 0) {
            totalRecharge = query.value("total_recharge").toDouble();
        }
        if (query.record().indexOf("points") >= 0) {
            points = query.value("points").toInt();
        }
        seller["memberLevel"] = memberLevel;
        seller["totalRecharge"] = totalRecharge;
        seller["points"] = points;
        seller["memberDiscount"] = getMemberDiscount(memberLevel);  // 折扣率
        seller["canParticipateLottery"] = (points >= 3);  // 是否可以参与抽奖（累计满3积分）
    }
    
    return seller;
}

bool Database::deleteSeller(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM sellers WHERE seller_id = ?");
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "删除商家失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::updateSellerStatus(int sellerId, const QString& status)
{
    // 注意：如果从 updateUserStatus 调用，mutex 已经被锁定，这里不需要再次锁定
    // 但为了保持函数独立性，仍然使用 QMutexLocker（QMutex 应该是递归的）
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE sellers SET status = ? WHERE seller_id = ?");
    query.addBindValue(status);
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "更新商家状态失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "商家状态更新成功，sellerId:" << sellerId << "status:" << status;
    
    // 如果解封商家，同步解封对应的用户，并恢复图书上架
    if (status == "正常") {
        // 通过seller_id找到对应的seller_name，然后找到对应的用户
        QSqlQuery sellerQuery(m_db);
        sellerQuery.prepare("SELECT seller_name FROM sellers WHERE seller_id = ?");
        sellerQuery.addBindValue(sellerId);
        
        if (sellerQuery.exec() && sellerQuery.next()) {
            QString sellerName = sellerQuery.value("seller_name").toString();
            
            // 通过seller_name（对应users表的username）找到对应的用户并解封
            QSqlQuery userQuery(m_db);
            userQuery.prepare("UPDATE users SET status = ? WHERE username = ? AND role = 2");
            userQuery.addBindValue("正常");
            userQuery.addBindValue(sellerName);
            
            if (userQuery.exec()) {
                int affectedRows = userQuery.numRowsAffected();
                if (affectedRows > 0) {
                    qDebug() << "商家ID" << sellerId << "(" << sellerName << ")已解封，同步解封对应的用户";
                } else {
                    qDebug() << "商家ID" << sellerId << "(" << sellerName << ")解封，但未找到对应的用户（可能用户不存在或不是商家）";
                }
            } else {
                qWarning() << "解封用户失败:" << userQuery.lastError().text();
            }
            
            // 恢复该商家的所有图书上架
            QSqlQuery updateBooksQuery(m_db);
            updateBooksQuery.prepare("UPDATE books SET status = ? WHERE merchant_id = ?");
            updateBooksQuery.addBindValue("正常");
            updateBooksQuery.addBindValue(sellerId);
            
            if (updateBooksQuery.exec()) {
                int affectedRows = updateBooksQuery.numRowsAffected();
                qDebug() << "商家ID" << sellerId << "已解封，其" << affectedRows << "本图书已恢复上架";
            } else {
                qWarning() << "恢复商家图书上架失败:" << updateBooksQuery.lastError().text();
            }
        }
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::updateBooksStatusBySellerId(int sellerId, const QString& status)
{
    // 注意：如果从 updateUserStatus 或 updateSellerStatus 调用，mutex 已经被锁定
    // 但为了保持函数独立性，仍然使用 QMutexLocker（QMutex 应该是递归的）
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE books SET status = ? WHERE merchant_id = ?");
    query.addBindValue(status);
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "批量更新商家图书状态失败:" << query.lastError().text();
        return false;
    }
    
    int affectedRows = query.numRowsAffected();
    qDebug() << "已更新商家ID" << sellerId << "的" << affectedRows << "本图书状态为" << status;
    return true;
}

// ==========================================
// 图书相关
// ==========================================

bool Database::addBook(const QJsonObject& book)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO books (isbn, title, author, category1, category2, merchant_id, price, stock, status, cover_image, description) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(book["isbn"].toString());
    query.addBindValue(book["title"].toString());
    query.addBindValue(book["author"].toString());
    // 兼容旧数据：如果book中有category字段，使用它作为category1
    QString category1 = book.contains("category1") ? book["category1"].toString() : 
                        (book.contains("category") ? book["category"].toString() : "");
    QString category2 = book.contains("category2") ? book["category2"].toString() : "";
    query.addBindValue(category1);
    query.addBindValue(category2);
    query.addBindValue(book.contains("merchantId") ? book["merchantId"].toInt() : 
                       (book.contains("merchant_id") ? book["merchant_id"].toInt() : QVariant(QVariant::Int)));
    query.addBindValue(book["price"].toDouble());
    query.addBindValue(book["stock"].toInt());
    // 新添加的书籍状态默认为"待审核"，如果book中已指定status则使用指定的
    QString bookStatus = book.contains("status") ? book["status"].toString() : "待审核";
    query.addBindValue(bookStatus);
    // 封面图片：如果有则使用，否则为空（前端会使用默认图片）
    query.addBindValue(book.contains("coverImage") ? book["coverImage"].toString() : 
                       (book.contains("cover_image") ? book["cover_image"].toString() : QString()));
    // 书籍描述：如果有则使用，否则为空
    query.addBindValue(book.contains("description") ? book["description"].toString() : QString());
    
    if (!query.exec()) {
        qWarning() << "添加图书失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::updateBook(const QString& isbn, const QJsonObject& book)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QStringList updates;
    QVariantList values;
    
    if (book.contains("title")) {
        updates << "title = ?";
        values << book["title"].toString();
    }
    if (book.contains("author")) {
        updates << "author = ?";
        values << book["author"].toString();
    }
    if (book.contains("category1")) {
        updates << "category1 = ?";
        values << book["category1"].toString();
    } else if (book.contains("category")) {
        // 兼容旧数据：将category映射到category1
        updates << "category1 = ?";
        values << book["category"].toString();
    }
    if (book.contains("category2")) {
        updates << "category2 = ?";
        values << book["category2"].toString();
    }
    if (book.contains("merchantId") || book.contains("merchant_id")) {
        updates << "merchant_id = ?";
        values << (book.contains("merchantId") ? book["merchantId"].toInt() : book["merchant_id"].toInt());
    }
    if (book.contains("price")) {
        updates << "price = ?";
        values << book["price"].toDouble();
    }
    if (book.contains("stock")) {
        updates << "stock = ?";
        values << book["stock"].toInt();
    }
    if (book.contains("status")) {
        updates << "status = ?";
        values << book["status"].toString();
    }
    if (book.contains("coverImage") || book.contains("cover_image")) {
        updates << "cover_image = ?";
        values << (book.contains("coverImage") ? book["coverImage"].toString() : book["cover_image"].toString());
    }
    if (book.contains("description")) {
        updates << "description = ?";
        values << book["description"].toString();
    }
    
    if (updates.isEmpty()) {
        return true;
    }
    
    QString sql = "UPDATE books SET " + updates.join(", ") + " WHERE isbn = ?";
    values << isbn;
    
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant& value : values) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        qWarning() << "更新图书失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::deleteBook(const QString& isbn)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM books WHERE isbn = ?");
    query.addBindValue(isbn);
    
    if (!query.exec()) {
        qWarning() << "删除图书失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QJsonArray Database::getAllBooks()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray books;
    
    if (!isConnected()) {
        return books;
    }
    
    QSqlQuery query(m_db);
    // 买家只能看到状态为"正常"的书籍（已审核通过的）
    if (!query.exec("SELECT * FROM books WHERE status = '正常' ORDER BY isbn")) {
        qWarning() << "查询图书列表失败:" << query.lastError().text();
        return books;
    }
    
    // 先收集所有书籍ID和书籍数据
    QList<QString> bookIds;
    QMap<QString, QJsonObject> bookMap;
    
    while (query.next()) {
        QJsonObject book;
        QString bookId = query.value("isbn").toString();
        book["isbn"] = bookId;
        book["title"] = query.value("title").toString();
        book["author"] = query.value("author").toString();
        book["category1"] = query.value("category1").toString();
        book["category2"] = query.value("category2").toString();
        book["merchantId"] = query.value("merchant_id").toInt();
        book["price"] = query.value("price").toDouble();
        book["stock"] = query.value("stock").toInt();
        book["status"] = query.value("status").toString();
        book["coverImage"] = query.value("cover_image").toString();
        book["description"] = query.value("description").toString();  // 书籍描述
        // 兼容旧数据：同时提供category字段（使用category1的值）
        book["category"] = query.value("category1").toString();
        book["favoriteCount"] = 0;  // 初始化为0
        
        bookIds.append(bookId);
        bookMap[bookId] = book;
    }
    
    // 批量查询收藏量（使用单个查询优化性能）
    if (!bookIds.isEmpty()) {
        // 使用单个查询获取所有书籍的收藏数
        QString countSql = "SELECT book_id, COUNT(*) as count FROM favorites WHERE book_id IN (";
        QStringList placeholders;
        for (int i = 0; i < bookIds.size(); ++i) {
            placeholders.append("?");
        }
        countSql += placeholders.join(",") + ") GROUP BY book_id";
        
        QSqlQuery countQuery(m_db);
        countQuery.prepare(countSql);
        for (const QString &bookId : bookIds) {
            countQuery.addBindValue(bookId);
        }
        
        if (countQuery.exec()) {
            while (countQuery.next()) {
                QString bookId = countQuery.value("book_id").toString();
                int count = countQuery.value("count").toInt();
                if (bookMap.contains(bookId)) {
                    bookMap[bookId]["favoriteCount"] = count;
                }
            }
        } else {
            qWarning() << "批量查询收藏量失败:" << countQuery.lastError().text();
        }
        
        // 批量查询评分统计（使用单个查询优化性能）
        QString ratingSql = "SELECT book_id, AVG(rating) as avg_rating, COUNT(*) as review_count "
                           "FROM reviews WHERE book_id IN (";
        ratingSql += placeholders.join(",") + ") GROUP BY book_id";
        
        QSqlQuery ratingQuery(m_db);
        ratingQuery.prepare(ratingSql);
        for (const QString &bookId : bookIds) {
            ratingQuery.addBindValue(bookId);
        }
        
        if (ratingQuery.exec()) {
            while (ratingQuery.next()) {
                QString bookId = ratingQuery.value("book_id").toString();
                double avgRating = ratingQuery.value("avg_rating").toDouble();
                int reviewCount = ratingQuery.value("review_count").toInt();
                if (bookMap.contains(bookId)) {
                    bookMap[bookId]["averageRating"] = avgRating;
                    bookMap[bookId]["reviewCount"] = reviewCount;
                    bookMap[bookId]["hasRating"] = true;
                    bookMap[bookId]["score"] = avgRating;  // 兼容score字段
                }
            }
        } else {
            qWarning() << "批量查询评分统计失败:" << ratingQuery.lastError().text();
        }
        
        // 为没有评分的商品设置默认值
        for (const QString &bookId : bookIds) {
            if (bookMap.contains(bookId) && !bookMap[bookId].contains("hasRating")) {
                bookMap[bookId]["averageRating"] = 0.0;
                bookMap[bookId]["reviewCount"] = 0;
                bookMap[bookId]["hasRating"] = false;
                bookMap[bookId]["score"] = 0.0;  // 兼容score字段
            }
        }
    }
    
    // 将书籍添加到结果数组（保持原有顺序）
    for (const QString &bookId : bookIds) {
        books.append(bookMap[bookId]);
    }
    
    qDebug() << "getAllBooks: 查询完成，书籍数量:" << books.size();
    
    return books;
}

QJsonArray Database::getAllBooksForSeller(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray books;
    
    if (!isConnected()) {
        return books;
    }
    
    QSqlQuery query(m_db);
    // 查询该卖家的所有书籍（不限制状态）
    query.prepare("SELECT * FROM books WHERE merchant_id = ? ORDER BY isbn");
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "查询卖家图书列表失败:" << query.lastError().text();
        return books;
    }
    
    // 先收集所有书籍ID和书籍数据
    QList<QString> bookIds;
    QMap<QString, QJsonObject> bookMap;
    
    while (query.next()) {
        QJsonObject book;
        QString bookId = query.value("isbn").toString();
        book["isbn"] = bookId;
        book["title"] = query.value("title").toString();
        book["author"] = query.value("author").toString();
        book["category1"] = query.value("category1").toString();
        book["category2"] = query.value("category2").toString();
        book["merchantId"] = query.value("merchant_id").toInt();
        book["price"] = query.value("price").toDouble();
        book["stock"] = query.value("stock").toInt();
        // 包含状态信息，如果为空则默认为"待审核"
        QString status = query.value("status").toString();
        if (status.isEmpty()) {
            status = "待审核";
        }
        book["status"] = status;
        book["coverImage"] = query.value("cover_image").toString();
        book["description"] = query.value("description").toString();  // 书籍描述
        book["category"] = query.value("category1").toString();  // 兼容旧数据
        book["favoriteCount"] = 0;  // 初始化为0
        
        bookIds.append(bookId);
        bookMap[bookId] = book;
    }
    
    // 批量查询收藏量（使用单个查询优化性能）
    if (!bookIds.isEmpty()) {
        QString countSql = "SELECT book_id, COUNT(*) as count FROM favorites WHERE book_id IN (";
        QStringList placeholders;
        for (int i = 0; i < bookIds.size(); ++i) {
            placeholders.append("?");
        }
        countSql += placeholders.join(",") + ") GROUP BY book_id";
        
        QSqlQuery countQuery(m_db);
        countQuery.prepare(countSql);
        for (const QString &bookId : bookIds) {
            countQuery.addBindValue(bookId);
        }
        
        if (countQuery.exec()) {
            while (countQuery.next()) {
                QString bookId = countQuery.value("book_id").toString();
                int count = countQuery.value("count").toInt();
                if (bookMap.contains(bookId)) {
                    bookMap[bookId]["favoriteCount"] = count;
                }
            }
        } else {
            qWarning() << "批量查询收藏量失败:" << countQuery.lastError().text();
        }
        
        // 批量查询销量（从订单中统计）
        // 查询该卖家的所有已支付订单，统计每个商品的销量
        QSqlQuery salesQuery(m_db);
        salesQuery.prepare("SELECT items FROM orders WHERE status IN ('已支付', '已发货', '已完成')");
        
        if (salesQuery.exec()) {
            // 统计每个商品的销量
            QMap<QString, int> salesMap;  // bookId -> 销量
            
            while (salesQuery.next()) {
                QString itemsJson = salesQuery.value("items").toString();
                if (itemsJson.isEmpty()) {
                    continue;
                }
                
                QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
                if (!doc.isArray()) {
                    continue;
                }
                
                QJsonArray items = doc.array();
                for (const QJsonValue &itemVal : items) {
                    QJsonObject item = itemVal.toObject();
                    QString itemBookId = item["bookId"].toString();
                    int quantity = item["quantity"].toInt();
                    int merchantId = item["merchantId"].toInt();
                    
                    // 只统计该卖家的商品
                    if (merchantId == sellerId && !itemBookId.isEmpty()) {
                        salesMap[itemBookId] += quantity;
                    }
                }
            }
            
            // 将销量添加到bookMap
            for (auto it = salesMap.begin(); it != salesMap.end(); ++it) {
                QString bookId = it.key();
                int sales = it.value();
                if (bookMap.contains(bookId)) {
                    bookMap[bookId]["sales"] = sales;
                }
            }
            
            // 为没有销量的商品设置默认值0
            for (const QString &bookId : bookIds) {
                if (bookMap.contains(bookId) && !bookMap[bookId].contains("sales")) {
                    bookMap[bookId]["sales"] = 0;
                }
            }
        } else {
            qWarning() << "批量查询销量失败:" << salesQuery.lastError().text();
            // 如果查询失败，为所有商品设置默认销量0
            for (const QString &bookId : bookIds) {
                if (bookMap.contains(bookId)) {
                    bookMap[bookId]["sales"] = 0;
                }
            }
        }
    }
    
    // 将书籍添加到结果数组（保持原有顺序）
    for (const QString &bookId : bookIds) {
        books.append(bookMap[bookId]);
    }
    
    qDebug() << "getAllBooksForSeller: 查询完成，书籍数量:" << books.size();
    
    return books;
}

QJsonArray Database::getPendingBooks()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray books;
    
    if (!isConnected()) {
        return books;
    }
    
    QSqlQuery query(m_db);
    // 查询状态为"待审核"的书籍
    if (!query.exec("SELECT * FROM books WHERE status = '待审核' ORDER BY isbn")) {
        qWarning() << "查询待审核图书列表失败:" << query.lastError().text();
        return books;
    }
    
    while (query.next()) {
        QJsonObject book;
        book["isbn"] = query.value("isbn").toString();
        book["title"] = query.value("title").toString();
        book["author"] = query.value("author").toString();
        book["category1"] = query.value("category1").toString();
        book["category2"] = query.value("category2").toString();
        book["merchantId"] = query.value("merchant_id").toInt();
        book["price"] = query.value("price").toDouble();
        book["stock"] = query.value("stock").toInt();
        // 确保状态字段正确返回（待审核书籍的状态应该是"待审核"）
        QString status = query.value("status").toString();
        if (status.isEmpty()) {
            status = "待审核";
        }
        book["status"] = status;
        book["coverImage"] = query.value("cover_image").toString();
        book["description"] = query.value("description").toString();  // 书籍描述
        book["category"] = query.value("category1").toString();  // 兼容旧数据
        books.append(book);
    }
    
    return books;
}

bool Database::approveBook(const QString& isbn)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "approveBook: 数据库未连接";
        return false;
    }
    
    // 先检查书籍是否存在且状态为"待审核"
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT status FROM books WHERE isbn = ?");
    checkQuery.addBindValue(isbn);
    
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "approveBook: 未找到书籍，ISBN:" << isbn;
        return false;
    }
    
    QString currentStatus = checkQuery.value("status").toString();
    if (currentStatus != "待审核") {
        qWarning() << "approveBook: 书籍状态不是'待审核'，当前状态:" << currentStatus << "ISBN:" << isbn;
        return false;
    }
    
    // 更新状态为"正常"（上架）
    QSqlQuery query(m_db);
    query.prepare("UPDATE books SET status = '正常' WHERE isbn = ?");
    query.addBindValue(isbn);
    
    if (!query.exec()) {
        qWarning() << "审核通过书籍失败:" << query.lastError().text() << "ISBN:" << isbn;
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        qWarning() << "approveBook: 更新失败，未影响任何行，ISBN:" << isbn;
        return false;
    }
    
    qDebug() << "approveBook: 审核通过成功，书籍已上架，ISBN:" << isbn;
    return true;
}

bool Database::rejectBook(const QString& isbn)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "rejectBook: 数据库未连接";
        return false;
    }
    
    // 先检查书籍是否存在且状态为"待审核"
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT status FROM books WHERE isbn = ?");
    checkQuery.addBindValue(isbn);
    
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "rejectBook: 未找到书籍，ISBN:" << isbn;
        return false;
    }
    
    QString currentStatus = checkQuery.value("status").toString();
    if (currentStatus != "待审核") {
        qWarning() << "rejectBook: 书籍状态不是'待审核'，当前状态:" << currentStatus << "ISBN:" << isbn;
        return false;
    }
    
    // 更新状态为"已拒绝"（不上架）
    QSqlQuery query(m_db);
    query.prepare("UPDATE books SET status = '已拒绝' WHERE isbn = ?");
    query.addBindValue(isbn);
    
    if (!query.exec()) {
        qWarning() << "审核拒绝书籍失败:" << query.lastError().text() << "ISBN:" << isbn;
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        qWarning() << "rejectBook: 更新失败，未影响任何行，ISBN:" << isbn;
        return false;
    }
    
    qDebug() << "rejectBook: 审核拒绝成功，书籍不上架，ISBN:" << isbn;
    return true;
}

QJsonObject Database::getBook(const QString& isbn)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject book;
    
    if (!isConnected()) {
        return book;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM books WHERE isbn = ?");
    query.addBindValue(isbn);
    
    if (!query.exec()) {
        qWarning() << "查询图书失败:" << query.lastError().text();
        return book;
    }
    
    if (query.next()) {
        book["isbn"] = query.value("isbn").toString();
        book["title"] = query.value("title").toString();
        book["author"] = query.value("author").toString();
        book["category1"] = query.value("category1").toString();
        book["category2"] = query.value("category2").toString();
        book["merchantId"] = query.value("merchant_id").toInt();
        book["price"] = query.value("price").toDouble();
        book["stock"] = query.value("stock").toInt();
        book["status"] = query.value("status").toString();
        book["coverImage"] = query.value("cover_image").toString();
        book["description"] = query.value("description").toString();  // 书籍描述
        // 兼容旧数据：同时提供category字段（使用category1的值）
        book["category"] = query.value("category1").toString();
    }
    
    return book;
}

QJsonArray Database::searchBooks(const QString& keyword)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray books;
    
    if (!isConnected()) {
        return books;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM books WHERE title LIKE ? OR author LIKE ? OR category1 LIKE ? OR category2 LIKE ?");
    QString searchPattern = "%" + keyword + "%";
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    
    if (!query.exec()) {
        qWarning() << "搜索图书失败:" << query.lastError().text();
        return books;
    }
    
    while (query.next()) {
        QJsonObject book;
        book["isbn"] = query.value("isbn").toString();
        book["title"] = query.value("title").toString();
        book["author"] = query.value("author").toString();
        book["category1"] = query.value("category1").toString();
        book["category2"] = query.value("category2").toString();
        book["merchantId"] = query.value("merchant_id").toInt();
        book["price"] = query.value("price").toDouble();
        book["stock"] = query.value("stock").toInt();
        book["status"] = query.value("status").toString();
        book["coverImage"] = query.value("cover_image").toString();
        // 兼容旧数据：同时提供category字段（使用category1的值）
        book["category"] = query.value("category1").toString();
        books.append(book);
    }
    
    return books;
}

// ==========================================
// 订单相关
// ==========================================

QString Database::createOrder(const QJsonObject& order)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return QString();
    }
    
    QString orderId = order["orderId"].toString();
    
    // 从订单项中提取merchant_id（取第一个订单项的merchant_id作为主要商家）
    int merchantId = -1;
    if (order.contains("items")) {
        QJsonArray items = order["items"].toArray();
        if (!items.isEmpty()) {
            QJsonObject firstItem = items[0].toObject();
            // 支持多种字段名：merchantId, merchant_id, sellerId
            if (firstItem.contains("merchantId")) {
                merchantId = firstItem["merchantId"].toInt();
            } else if (firstItem.contains("merchant_id")) {
                merchantId = firstItem["merchant_id"].toInt();
            } else if (firstItem.contains("sellerId")) {
                merchantId = firstItem["sellerId"].toInt();
            }
            
            qDebug() << "createOrder: 从订单项提取merchant_id:" << merchantId << "订单ID:" << orderId;
        }
    }
    
    // 如果从items中无法提取merchant_id，尝试从订单对象本身获取
    if (merchantId <= 0 && order.contains("merchantId")) {
        merchantId = order["merchantId"].toInt();
        qDebug() << "createOrder: 从订单对象获取merchant_id:" << merchantId;
    }
    
    if (merchantId <= 0) {
        qWarning() << "createOrder: 警告 - 无法提取merchant_id，订单ID:" << orderId;
        qWarning() << "createOrder: 订单items内容:" << QJsonDocument(order["items"].toArray()).toJson(QJsonDocument::Compact);
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO orders (order_id, user_id, merchant_id, customer, phone, total_amount, status, "
                 "payment_method, order_date, address, operator, remark, items) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(orderId);
    // 确保userId是整数类型，如果不存在或无效则使用NULL
    int userId = 0;
    if (order.contains("userId")) {
        if (order["userId"].isDouble()) {
            userId = order["userId"].toInt();
        } else if (order["userId"].isString()) {
            bool ok;
            userId = order["userId"].toString().toInt(&ok);
            if (!ok) {
                qWarning() << "createOrder: userId字符串无法转换为整数:" << order["userId"].toString();
                userId = 0;
            }
        } else {
            userId = order["userId"].toInt();
        }
    } else {
        qWarning() << "createOrder: 订单对象中不包含userId字段";
    }
    
    if (userId <= 0) {
        qWarning() << "createOrder: 无效的userId:" << userId << "订单ID:" << orderId;
        qWarning() << "createOrder: 订单对象内容:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "createOrder: 保存订单，订单ID:" << orderId << "用户ID:" << userId;
    }
    
    query.addBindValue(userId > 0 ? userId : QVariant());
    query.addBindValue(merchantId > 0 ? merchantId : QVariant());
    
    // 处理customer字段，如果为空则使用默认值
    QString customer = order["customer"].toString();
    if (customer.isEmpty()) {
        customer = "客户";
    }
    query.addBindValue(customer);
    
    // 处理phone字段，如果为空则使用默认值
    QString phone = order["phone"].toString();
    if (phone.isEmpty()) {
        phone = "";
    }
    query.addBindValue(phone);
    
    query.addBindValue(order["totalAmount"].toDouble());
    query.addBindValue(order["status"].toString("待支付"));
    query.addBindValue(order["paymentMethod"].toString());
    
    // 处理订单日期：如果为空或格式不正确，使用当前时间
    QString orderDate = order["orderDate"].toString();
    if (orderDate.isEmpty()) {
        orderDate = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }
    query.addBindValue(orderDate);
    
    // 处理address字段，如果为空则使用默认值
    QString address = order["address"].toString();
    if (address.isEmpty()) {
        address = "";
    }
    query.addBindValue(address);
    
    query.addBindValue(order["operator"].toString("系统"));
    query.addBindValue(order["remark"].toString());
    query.addBindValue(QString(QJsonDocument(order["items"].toArray()).toJson(QJsonDocument::Compact)));
    
    if (!query.exec()) {
        qWarning() << "创建订单失败:" << query.lastError().text();
        qWarning() << "订单ID:" << orderId;
        qWarning() << "用户ID:" << order["userId"].toString();
        qWarning() << "订单数据:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
        return QString();
    }
    
    qDebug() << "订单成功插入数据库，订单ID:" << orderId << "，用户ID:" << order["userId"].toString() << "，影响行数:" << query.numRowsAffected();
    return orderId;
}

bool Database::updateOrderStatus(const QString& orderId, const QString& status, const QString& paymentMethod, const QString& cancelReason, const QString& trackingNumber, double totalAmount)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法更新订单状态";
        return false;
    }
    
    QSqlQuery query(m_db);
    QString sql = "UPDATE orders SET status = ?";
    QList<QVariant> bindValues;
    bindValues.append(status);
    
    if (status == "已支付") {
        sql += ", pay_time = NOW()";
        if (!paymentMethod.isEmpty()) {
            sql += ", payment_method = ?";
            bindValues.append(paymentMethod);
        }
        // 如果提供了总金额（使用优惠券后的金额），更新订单金额
        if (totalAmount >= 0) {
            sql += ", total_amount = ?";
            bindValues.append(totalAmount);
            qDebug() << "更新订单金额，订单ID:" << orderId << "新金额:" << totalAmount;
        }
    } else if (status == "已发货") {
        sql += ", ship_time = NOW()";
        if (!trackingNumber.isEmpty()) {
            sql += ", tracking_number = ?";
            bindValues.append(trackingNumber);
        }
    } else if (status == "已取消") {
        sql += ", cancel_time = NOW()";
        if (!cancelReason.isEmpty()) {
            sql += ", cancel_reason = ?";
            bindValues.append(cancelReason);
        }
    }
    
    sql += " WHERE order_id = ?";
    bindValues.append(orderId);
    
    query.prepare(sql);
    for (const QVariant &value : bindValues) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        qWarning() << "更新订单状态失败，订单ID:" << orderId << "状态:" << status << "错误:" << query.lastError().text();
        return false;
    }
    
    int affectedRows = query.numRowsAffected();
    if (affectedRows > 0) {
        qDebug() << "订单状态更新成功，订单ID:" << orderId << "状态:" << status << "影响行数:" << affectedRows;
        if (status == "已发货" && !trackingNumber.isEmpty()) {
            qDebug() << "物流单号已更新:" << trackingNumber;
        }
    } else {
        qWarning() << "订单状态更新失败：未找到订单，订单ID:" << orderId;
    }
    
    return affectedRows > 0;
}

QJsonArray Database::getUserOrders(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray orders;
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法查询用户订单";
        return orders;
    }
    
    QSqlQuery query(m_db);
    // 使用标准的查询条件，查询user_id匹配的订单
    // 如果userId有效（>0），只查询匹配的订单；如果userId为0或无效，不查询任何订单
    if (userId > 0) {
        query.prepare("SELECT * FROM orders WHERE user_id = ? ORDER BY order_date DESC");
        query.addBindValue(userId);
    } else {
        qWarning() << "getUserOrders: 无效的userId，不查询任何订单，userId:" << userId;
        return orders;
    }
    
    qDebug() << "getUserOrders: 开始查询用户订单，用户ID:" << userId;
    
    if (!query.exec()) {
        qWarning() << "查询用户订单失败:" << query.lastError().text();
        qWarning() << "SQL错误详情:" << query.lastError().databaseText();
        qWarning() << "用户ID:" << userId;
        return orders;
    }
    
    int count = 0;
    while (query.next()) {
        QVariant userIdValue = query.value("user_id");
        int orderUserId = 0;
        if (!userIdValue.isNull() && userIdValue.isValid()) {
            orderUserId = userIdValue.toInt();
        }
        
        qDebug() << "getUserOrders: 找到订单，订单ID:" << query.value("order_id").toString() 
                 << "订单用户ID:" << (orderUserId > 0 ? QString::number(orderUserId) : "NULL/0") << "请求用户ID:" << userId;
        
        // 由于SQL查询已经使用WHERE user_id = ?过滤，理论上所有查询到的订单都应该匹配
        // 但如果orderUserId为0或NULL，可能是旧数据，使用请求的userId
        // 如果orderUserId不为0且与userId不匹配，记录警告但不跳过（因为SQL已经过滤了，可能是数据类型转换问题）
        if (orderUserId > 0 && orderUserId != userId) {
            qWarning() << "getUserOrders: 警告 - 订单用户ID与请求不匹配，但SQL查询已匹配。订单ID:" << query.value("order_id").toString()
                       << "订单用户ID:" << orderUserId << "请求用户ID:" << userId;
            // 不跳过，因为SQL查询已经过滤了，可能是数据类型转换问题
        }
        
        // 如果orderUserId为0或NULL，说明可能是旧数据，使用请求的userId
        if (orderUserId == 0) {
            qDebug() << "getUserOrders: 订单user_id为0或NULL，使用请求的userId:" << userId;
        }
        
        // 检查order_id是否为空
        QString orderId = query.value("order_id").toString();
        if (orderId.isEmpty()) {
            qWarning() << "getUserOrders: 订单order_id为空，跳过该订单。用户ID:" << userId;
            continue;
        }
        
        QJsonObject order;
        order["orderId"] = orderId;
        // 如果订单中user_id为NULL或0，使用请求的userId
        order["userId"] = orderUserId > 0 ? orderUserId : userId;
        QVariant merchantIdValue = query.value("merchant_id");
        order["merchantId"] = (!merchantIdValue.isNull() && merchantIdValue.isValid()) ? merchantIdValue.toInt() : 0;
        order["customer"] = query.value("customer").toString();
        order["phone"] = query.value("phone").toString();
        order["totalAmount"] = query.value("total_amount").toDouble();
        order["status"] = query.value("status").toString();
        order["paymentMethod"] = query.value("payment_method").toString();
        order["orderDate"] = query.value("order_date").toString();
        order["payTime"] = query.value("pay_time").toString();
        order["shipTime"] = query.value("ship_time").toString();
        order["cancelTime"] = query.value("cancel_time").toString();
        order["cancelReason"] = query.value("cancel_reason").toString();
        order["trackingNumber"] = query.value("tracking_number").toString();
        order["address"] = query.value("address").toString();
        order["operator"] = query.value("operator").toString();
        order["remark"] = query.value("remark").toString();
        
        // 解析items JSON
        QString itemsJson = query.value("items").toString();
        if (!itemsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            if (doc.isArray()) {
                order["items"] = doc.array();
            } else {
                qWarning() << "getUserOrders: items JSON解析失败，订单ID:" << query.value("order_id").toString();
                order["items"] = QJsonArray();
            }
        } else {
            order["items"] = QJsonArray();
        }
        
        orders.append(order);
        count++;
        qDebug() << "getUserOrders: 已添加订单到结果，订单ID:" << order["orderId"].toString() << "订单数:" << count;
    }
    
    qDebug() << "getUserOrders: 查询用户订单完成，用户ID:" << userId << "，找到订单数:" << count << "返回数组大小:" << orders.size();
    
    // 如果没有找到订单，检查数据库中是否有该用户的订单（使用不同的查询方式）
    if (count == 0) {
        QSqlQuery checkQuery(m_db);
        
        // 检查精确匹配
        checkQuery.prepare("SELECT COUNT(*) as count FROM orders WHERE user_id = ?");
        checkQuery.addBindValue(userId);
        int exactCount = 0;
        if (checkQuery.exec() && checkQuery.next()) {
            exactCount = checkQuery.value("count").toInt();
        }
        
        // 检查所有订单
        checkQuery.prepare("SELECT COUNT(*) as count FROM orders");
        int totalCount = 0;
        if (checkQuery.exec() && checkQuery.next()) {
            totalCount = checkQuery.value("count").toInt();
        }
        
        qDebug() << "getUserOrders: 诊断信息 - 精确匹配订单数:" << exactCount << "总订单数:" << totalCount << "用户ID:" << userId;
        
        if (exactCount > 0) {
            qWarning() << "getUserOrders: 数据库中存在" << exactCount << "个订单，但查询结果为空，用户ID:" << userId;
            // 尝试直接查询，看看是否有类型问题
            checkQuery.prepare("SELECT order_id, user_id, CAST(user_id AS CHAR) as user_id_str FROM orders WHERE user_id = ? LIMIT 5");
            checkQuery.addBindValue(userId);
            if (checkQuery.exec()) {
                qDebug() << "getUserOrders: 直接查询结果:";
                while (checkQuery.next()) {
                    QVariant uid = checkQuery.value("user_id");
                    QString uidStr = checkQuery.value("user_id_str").toString();
                    qDebug() << "  订单ID:" << checkQuery.value("order_id").toString() 
                             << "用户ID(数字):" << (uid.isNull() ? "NULL" : QString::number(uid.toInt()))
                             << "用户ID(字符串):" << uidStr;
                }
            }
        } else if (totalCount > 0) {
            // 检查是否有user_id为NULL的订单
            checkQuery.prepare("SELECT COUNT(*) as count FROM orders WHERE user_id IS NULL");
            int nullCount = 0;
            if (checkQuery.exec() && checkQuery.next()) {
                nullCount = checkQuery.value("count").toInt();
            }
            qDebug() << "getUserOrders: 数据库中user_id为NULL的订单数:" << nullCount;
            
            // 尝试查询所有订单，看看user_id的分布
            checkQuery.prepare("SELECT DISTINCT user_id FROM orders LIMIT 10");
            if (checkQuery.exec()) {
                qDebug() << "getUserOrders: 数据库中的user_id分布:";
                while (checkQuery.next()) {
                    QVariant uid = checkQuery.value("user_id");
                    qDebug() << "  user_id:" << (uid.isNull() ? "NULL" : QString::number(uid.toInt()));
                }
            }
        } else {
            qDebug() << "getUserOrders: 数据库中确实没有订单";
        }
    }
    
    qDebug() << "getUserOrders: 最终返回订单数:" << orders.size();
    return orders;
}

QJsonArray Database::getAllOrders()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray orders;
    
    if (!isConnected()) {
        return orders;
    }
    
    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM orders ORDER BY order_date DESC")) {
        qWarning() << "查询订单列表失败:" << query.lastError().text();
        return orders;
    }
    
    while (query.next()) {
        QJsonObject order;
        order["orderId"] = query.value("order_id").toString();
        order["userId"] = query.value("user_id").toInt();
        order["merchantId"] = query.value("merchant_id").toInt();
        order["customer"] = query.value("customer").toString();
        order["phone"] = query.value("phone").toString();
        order["totalAmount"] = query.value("total_amount").toDouble();
        order["status"] = query.value("status").toString();
        order["paymentMethod"] = query.value("payment_method").toString();
        order["orderDate"] = query.value("order_date").toString();
        order["payTime"] = query.value("pay_time").toString();
        order["shipTime"] = query.value("ship_time").toString();
        order["cancelTime"] = query.value("cancel_time").toString();
        order["cancelReason"] = query.value("cancel_reason").toString();
        order["trackingNumber"] = query.value("tracking_number").toString();
        order["address"] = query.value("address").toString();
        order["operator"] = query.value("operator").toString();
        order["remark"] = query.value("remark").toString();
        
        // 解析items JSON
        QString itemsJson = query.value("items").toString();
        if (!itemsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            order["items"] = doc.array();
        }
        
        orders.append(order);
    }
    
    return orders;
}

QJsonArray Database::getSellerOrders(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray orders;
    
    if (!isConnected()) {
        qWarning() << "getSellerOrders: 数据库未连接";
        return orders;
    }
    
    qDebug() << "getSellerOrders: 开始查询商家订单，商家ID:" << sellerId;
    
    // 使用集合来避免重复添加订单
    QSet<QString> addedOrderIds;
    
    // 方法1：先通过merchant_id字段快速筛选（如果订单只有一个商家）
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM orders WHERE merchant_id = ? ORDER BY order_date DESC");
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "getSellerOrders: 查询商家订单失败:" << query.lastError().text();
        return orders;
    }
    
    int method1Count = 0;
    while (query.next()) {
        QString orderId = query.value("order_id").toString();
        if (addedOrderIds.contains(orderId)) {
            continue;  // 避免重复添加
        }
        
        QJsonObject order;
        order["orderId"] = orderId;
        order["userId"] = query.value("user_id").toInt();
        order["merchantId"] = query.value("merchant_id").toInt();
        order["customer"] = query.value("customer").toString();
        order["phone"] = query.value("phone").toString();
        order["totalAmount"] = query.value("total_amount").toDouble();
        order["status"] = query.value("status").toString();
        order["paymentMethod"] = query.value("payment_method").toString();
        order["orderDate"] = query.value("order_date").toString();
        order["payTime"] = query.value("pay_time").toString();
        order["shipTime"] = query.value("ship_time").toString();
        order["cancelTime"] = query.value("cancel_time").toString();
        order["cancelReason"] = query.value("cancel_reason").toString();
        order["trackingNumber"] = query.value("tracking_number").toString();
        order["address"] = query.value("address").toString();
        order["operator"] = query.value("operator").toString();
        order["remark"] = query.value("remark").toString();
        
        // 解析items JSON
        QString itemsJson = query.value("items").toString();
        if (!itemsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            if (doc.isArray()) {
                order["items"] = doc.array();
            } else {
                order["items"] = QJsonArray();
            }
        } else {
            order["items"] = QJsonArray();
        }
        
        orders.append(order);
        addedOrderIds.insert(orderId);
        method1Count++;
    }
    
    qDebug() << "getSellerOrders: 方法1找到" << method1Count << "个订单（通过merchant_id字段）";
    
    // 方法2：检查所有订单的items JSON，找出包含该商家商品但merchant_id字段不匹配或为NULL的订单
    // （处理一个订单包含多个商家商品的情况，或者merchant_id未正确设置的情况）
    QSqlQuery query2(m_db);
    query2.prepare("SELECT * FROM orders WHERE merchant_id != ? OR merchant_id IS NULL ORDER BY order_date DESC");
    query2.addBindValue(sellerId);
    
    if (!query2.exec()) {
        qWarning() << "getSellerOrders: 查询混合订单失败:" << query2.lastError().text();
        qDebug() << "getSellerOrders: 方法1已找到" << orders.size() << "个订单，返回结果";
        return orders;
    }
    
    int method2Count = 0;
    int method2MatchedCount = 0;
    while (query2.next()) {
        method2Count++;
        QString orderId = query2.value("order_id").toString();
        
        // 跳过已经添加的订单
        if (addedOrderIds.contains(orderId)) {
            continue;
        }
        
        // 解析items JSON，检查是否包含该商家的商品
        QString itemsJson = query2.value("items").toString();
        if (itemsJson.isEmpty()) {
            continue;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
        if (!doc.isArray()) {
            continue;
        }
        
        QJsonArray items = doc.array();
        
        // 检查订单项中是否有该商家的商品
        bool hasSellerItem = false;
        for (const QJsonValue &itemVal : items) {
            QJsonObject item = itemVal.toObject();
            // 检查多种可能的字段名：merchantId, merchant_id, sellerId
            int itemMerchantId = -1;
            if (item.contains("merchantId")) {
                itemMerchantId = item["merchantId"].toInt();
            } else if (item.contains("merchant_id")) {
                itemMerchantId = item["merchant_id"].toInt();
            } else if (item.contains("sellerId")) {
                itemMerchantId = item["sellerId"].toInt();
            }
            
            if (itemMerchantId == sellerId) {
                hasSellerItem = true;
                qDebug() << "getSellerOrders: 订单" << orderId << "包含商家" << sellerId << "的商品";
                break;
            }
        }
        
        // 如果订单包含该商家的商品，添加到结果列表
        if (hasSellerItem) {
            QJsonObject order;
            order["orderId"] = orderId;
            order["userId"] = query2.value("user_id").toInt();
            QVariant merchantIdValue = query2.value("merchant_id");
            order["merchantId"] = (!merchantIdValue.isNull() && merchantIdValue.isValid()) ? merchantIdValue.toInt() : sellerId;
            order["customer"] = query2.value("customer").toString();
            order["phone"] = query2.value("phone").toString();
            order["totalAmount"] = query2.value("total_amount").toDouble();
            order["status"] = query2.value("status").toString();
            order["paymentMethod"] = query2.value("payment_method").toString();
            order["orderDate"] = query2.value("order_date").toString();
            order["payTime"] = query2.value("pay_time").toString();
            order["shipTime"] = query2.value("ship_time").toString();
            order["cancelTime"] = query2.value("cancel_time").toString();
            order["cancelReason"] = query2.value("cancel_reason").toString();
            order["trackingNumber"] = query2.value("tracking_number").toString();
            order["address"] = query2.value("address").toString();
            order["operator"] = query2.value("operator").toString();
            order["remark"] = query2.value("remark").toString();
            order["items"] = items;
            
            orders.append(order);
            addedOrderIds.insert(orderId);
            method2MatchedCount++;
        }
    }
    
    qDebug() << "getSellerOrders: 方法2检查了" << method2Count << "个订单，匹配" << method2MatchedCount << "个订单（通过items JSON）";
    qDebug() << "getSellerOrders: 总共找到" << orders.size() << "个订单";
    
    // 如果没有找到订单，进行诊断
    if (orders.isEmpty()) {
        qDebug() << "getSellerOrders: 未找到订单，进行诊断查询";
        
        // 检查数据库中是否有订单
        QSqlQuery diagQuery(m_db);
        if (diagQuery.exec("SELECT COUNT(*) as count FROM orders")) {
            if (diagQuery.next()) {
                int totalOrders = diagQuery.value("count").toInt();
                qDebug() << "getSellerOrders: 数据库中总订单数:" << totalOrders;
            }
        }
        
        // 检查merchant_id的分布
        QSqlQuery merchantQuery(m_db);
        if (merchantQuery.exec("SELECT DISTINCT merchant_id FROM orders WHERE merchant_id IS NOT NULL LIMIT 10")) {
            qDebug() << "getSellerOrders: 数据库中的merchant_id分布:";
            while (merchantQuery.next()) {
                qDebug() << "  merchant_id:" << merchantQuery.value("merchant_id").toInt();
            }
        }
    }
    
    return orders;
}

bool Database::deleteOrder(const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM orders WHERE order_id = ?");
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        qWarning() << "删除订单失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QJsonObject Database::getOrder(const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject order;
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法查询订单:" << orderId;
        return order;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM orders WHERE order_id = ?");
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        qWarning() << "查询订单失败，订单ID:" << orderId << "错误:" << query.lastError().text();
        return order;
    }
    
    if (query.next()) {
        order["orderId"] = query.value("order_id").toString();
        order["userId"] = query.value("user_id").toInt();
        order["merchantId"] = query.value("merchant_id").toInt();
        order["customer"] = query.value("customer").toString();
        order["phone"] = query.value("phone").toString();
        order["totalAmount"] = query.value("total_amount").toDouble();
        order["status"] = query.value("status").toString();
        order["paymentMethod"] = query.value("payment_method").toString();
        order["orderDate"] = query.value("order_date").toString();
        order["payTime"] = query.value("pay_time").toString();
        order["shipTime"] = query.value("ship_time").toString();
        order["cancelTime"] = query.value("cancel_time").toString();
        order["cancelReason"] = query.value("cancel_reason").toString();
        order["trackingNumber"] = query.value("tracking_number").toString();
        order["address"] = query.value("address").toString();
        order["operator"] = query.value("operator").toString();
        order["remark"] = query.value("remark").toString();
        
        // 解析items JSON
        QString itemsJson = query.value("items").toString();
        if (!itemsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            order["items"] = doc.array();
        }
        
        qDebug() << "成功查询到订单，订单ID:" << orderId << "状态:" << order["status"].toString();
    } else {
        qWarning() << "订单不存在，订单ID:" << orderId;
    }
    
    return order;
}

// ==========================================
// 购物车相关
// ==========================================

bool Database::addToCart(int userId, const QString& bookId, int quantity)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO cart (user_id, book_id, quantity) VALUES (?, ?, ?) "
                 "ON DUPLICATE KEY UPDATE quantity = quantity + ?");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    query.addBindValue(quantity);
    query.addBindValue(quantity);
    
    if (!query.exec()) {
        qWarning() << "添加到购物车失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QJsonArray Database::getCart(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray cart;
    
    if (!isConnected()) {
        return cart;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT c.*, b.title, b.price FROM cart c "
                 "LEFT JOIN books b ON c.book_id = b.isbn "
                 "WHERE c.user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "查询购物车失败:" << query.lastError().text();
        return cart;
    }
    
    while (query.next()) {
        QJsonObject item;
        item["cartId"] = query.value("cart_id").toInt();
        item["bookId"] = query.value("book_id").toString();
        item["bookName"] = query.value("title").toString();
        item["price"] = query.value("price").toDouble();
        item["quantity"] = query.value("quantity").toInt();
        cart.append(item);
    }
    
    return cart;
}

bool Database::updateCartQuantity(int userId, const QString& bookId, int quantity)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 如果数量为0或负数，则删除该记录
    if (quantity <= 0) {
        return removeFromCart(userId, bookId);
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE cart SET quantity = ? WHERE user_id = ? AND book_id = ?");
    query.addBindValue(quantity);
    query.addBindValue(userId);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "更新购物车数量失败:" << query.lastError().text();
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        qWarning() << "未找到要更新的购物车项，用户ID:" << userId << "图书ID:" << bookId;
        return false;
    }
    
    qDebug() << "购物车数量更新成功，用户ID:" << userId << "图书ID:" << bookId << "新数量:" << quantity;
    return true;
}

bool Database::removeFromCart(int userId, const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM cart WHERE user_id = ? AND book_id = ?");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "从购物车移除失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "从购物车移除成功，用户ID:" << userId << "图书ID:" << bookId;
    return true;
}

bool Database::clearCart(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM cart WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "清空购物车失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

// ==========================================
// 收藏相关
// ==========================================

bool Database::addFavorite(int userId, const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO favorites (user_id, book_id) VALUES (?, ?) "
                 "ON DUPLICATE KEY UPDATE add_time = CURRENT_TIMESTAMP");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "添加到收藏失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "添加到收藏成功，用户ID:" << userId << "图书ID:" << bookId;
    return true;
}

bool Database::removeFavorite(int userId, const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM favorites WHERE user_id = ? AND book_id = ?");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "从收藏移除失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "从收藏移除成功，用户ID:" << userId << "图书ID:" << bookId;
    return true;
}

QJsonArray Database::getUserFavorites(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray favorites;
    
    if (!isConnected()) {
        return favorites;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT book_id FROM favorites WHERE user_id = ? ORDER BY add_time DESC");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "查询收藏列表失败:" << query.lastError().text();
        return favorites;
    }
    
    while (query.next()) {
        favorites.append(query.value("book_id").toString());
    }
    
    return favorites;
}

int Database::getBookFavoriteCount(const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return 0;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) as count FROM favorites WHERE book_id = ?");
    query.addBindValue(bookId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "查询收藏量失败:" << query.lastError().text();
        return 0;
    }
    
    return query.value("count").toInt();
}

// ==========================================
// 会员相关
// ==========================================

bool Database::addMember(const QJsonObject& member)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO members (card_no, name, phone, level, balance, points, create_date) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(member["cardNo"].toString());
    query.addBindValue(member["name"].toString());
    query.addBindValue(member["phone"].toString());
    query.addBindValue(member["level"].toString("普通"));
    query.addBindValue(member["balance"].toDouble());
    query.addBindValue(member["points"].toInt());
    query.addBindValue(member["createDate"].toString());
    
    if (!query.exec()) {
        qWarning() << "添加会员失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::updateMember(const QString& cardNo, const QJsonObject& member)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QStringList updates;
    QVariantList values;
    
    if (member.contains("name")) {
        updates << "name = ?";
        values << member["name"].toString();
    }
    if (member.contains("phone")) {
        updates << "phone = ?";
        values << member["phone"].toString();
    }
    if (member.contains("level")) {
        updates << "level = ?";
        values << member["level"].toString();
    }
    if (member.contains("balance")) {
        updates << "balance = ?";
        values << member["balance"].toDouble();
    }
    if (member.contains("points")) {
        updates << "points = ?";
        values << member["points"].toInt();
    }
    
    if (updates.isEmpty()) {
        return true;
    }
    
    QString sql = "UPDATE members SET " + updates.join(", ") + " WHERE card_no = ?";
    values << cardNo;
    
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant& value : values) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        qWarning() << "更新会员失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::deleteMember(const QString& cardNo)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM members WHERE card_no = ?");
    query.addBindValue(cardNo);
    
    if (!query.exec()) {
        qWarning() << "删除会员失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QJsonArray Database::getAllMembers()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray members;
    
    if (!isConnected()) {
        return members;
    }
    
    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM members ORDER BY create_date DESC")) {
        qWarning() << "查询会员列表失败:" << query.lastError().text();
        return members;
    }
    
    while (query.next()) {
        QJsonObject member;
        member["cardNo"] = query.value("card_no").toString();
        member["name"] = query.value("name").toString();
        member["phone"] = query.value("phone").toString();
        member["level"] = query.value("level").toString();
        member["balance"] = query.value("balance").toDouble();
        member["points"] = query.value("points").toInt();
        member["createDate"] = query.value("create_date").toString();
        members.append(member);
    }
    
    return members;
}

bool Database::rechargeMember(const QString& cardNo, double amount)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE members SET balance = balance + ? WHERE card_no = ?");
    query.addBindValue(amount);
    query.addBindValue(cardNo);
    
    if (!query.exec()) {
        qWarning() << "会员充值失败:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

// ==========================================
// 统计相关
// ==========================================

// ===== 卖家申诉相关 =====
bool Database::submitSellerAppeal(int sellerId, const QString& sellerName, const QString& appealReason)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO seller_appeals (seller_id, seller_name, appeal_reason, status) "
                 "VALUES (?, ?, ?, '待审核')");
    query.addBindValue(sellerId);
    query.addBindValue(sellerName);
    query.addBindValue(appealReason);
    
    if (!query.exec()) {
        qWarning() << "提交申诉失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "卖家ID" << sellerId << "提交申诉成功";
    return true;
}

QJsonObject Database::getSellerAppeal(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject appeal;
    
    if (!isConnected()) {
        return appeal;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM seller_appeals WHERE seller_id = ? ORDER BY submit_time DESC LIMIT 1");
    query.addBindValue(sellerId);
    
    if (query.exec() && query.next()) {
        appeal["appealId"] = query.value("appeal_id").toInt();
        appeal["sellerId"] = query.value("seller_id").toInt();
        appeal["sellerName"] = query.value("seller_name").toString();
        appeal["appealReason"] = query.value("appeal_reason").toString();
        appeal["status"] = query.value("status").toString();
        appeal["submitTime"] = query.value("submit_time").toString();
        if (!query.value("review_time").isNull()) {
            appeal["reviewTime"] = query.value("review_time").toString();
        }
        if (!query.value("reviewer_id").isNull()) {
            appeal["reviewerId"] = query.value("reviewer_id").toInt();
        }
        if (!query.value("review_comment").isNull()) {
            appeal["reviewComment"] = query.value("review_comment").toString();
        }
    }
    
    return appeal;
}

QJsonArray Database::getAllAppeals(const QString& status)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray appeals;
    
    if (!isConnected()) {
        return appeals;
    }
    
    QSqlQuery query(m_db);
    QString sql = "SELECT * FROM seller_appeals";
    if (!status.isEmpty()) {
        sql += " WHERE status = ?";
    }
    sql += " ORDER BY submit_time DESC";
    
    query.prepare(sql);
    if (!status.isEmpty()) {
        query.addBindValue(status);
    }
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject appeal;
            appeal["appealId"] = query.value("appeal_id").toInt();
            appeal["sellerId"] = query.value("seller_id").toInt();
            appeal["sellerName"] = query.value("seller_name").toString();
            appeal["appealReason"] = query.value("appeal_reason").toString();
            appeal["status"] = query.value("status").toString();
            appeal["submitTime"] = query.value("submit_time").toString();
            if (!query.value("review_time").isNull()) {
                appeal["reviewTime"] = query.value("review_time").toString();
            }
            if (!query.value("reviewer_id").isNull()) {
                appeal["reviewerId"] = query.value("reviewer_id").toInt();
            }
            if (!query.value("review_comment").isNull()) {
                appeal["reviewComment"] = query.value("review_comment").toString();
            }
            appeals.append(appeal);
        }
    }
    
    return appeals;
}

bool Database::reviewSellerAppeal(int appealId, int reviewerId, const QString& status, const QString& reviewComment)
{
    int sellerId = -1;
    
    // 第一步：获取seller_id（如果需要解封），并更新申诉状态
    {
        QMutexLocker locker(&m_mutex);
        
        if (!isConnected()) {
            return false;
        }
        
        // 先获取seller_id（如果需要解封）
        if (status == "已通过") {
            QSqlQuery selectQuery(m_db);
            selectQuery.prepare("SELECT seller_id FROM seller_appeals WHERE appeal_id = ?");
            selectQuery.addBindValue(appealId);
            if (selectQuery.exec() && selectQuery.next()) {
                sellerId = selectQuery.value("seller_id").toInt();
            } else {
                qWarning() << "获取申诉的seller_id失败:" << selectQuery.lastError().text();
                return false;
            }
        }
        
        // 更新申诉状态
        QSqlQuery query(m_db);
        query.prepare("UPDATE seller_appeals SET status = ?, reviewer_id = ?, review_time = NOW(), review_comment = ? "
                     "WHERE appeal_id = ?");
        query.addBindValue(status);
        query.addBindValue(reviewerId);
        query.addBindValue(reviewComment);
        query.addBindValue(appealId);
        
        if (!query.exec()) {
            qWarning() << "审核申诉失败:" << query.lastError().text();
            return false;
        }
        
        qDebug() << "申诉ID" << appealId << "状态已更新为:" << status;
    } // 释放锁，允许其他操作继续
    
    // 第二步：如果申诉通过，自动解封卖家（在不持有锁的情况下执行，避免阻塞其他操作）
    if (status == "已通过" && sellerId > 0) {
        // 注意：updateSellerStatus会自己获取锁，但此时我们已经释放了锁，不会阻塞其他操作
        bool unbanSuccess = updateSellerStatus(sellerId, "正常");
        if (unbanSuccess) {
            qDebug() << "申诉通过，自动解封卖家ID" << sellerId;
        } else {
            qWarning() << "申诉通过，但解封卖家ID" << sellerId << "失败，申诉状态已更新";
        }
    }
    
    qDebug() << "申诉ID" << appealId << "审核完成，状态:" << status;
    return true;
}

QJsonObject Database::getSystemStats()
{
    QMutexLocker locker(&m_mutex);
    QJsonObject stats;
    
    if (!isConnected()) {
        return stats;
    }
    
    QSqlQuery query(m_db);
    
    // 统计用户数
    if (query.exec("SELECT COUNT(*) FROM users")) {
        if (query.next()) {
            stats["totalUsers"] = query.value(0).toInt();
        }
    }
    
    // 统计买家数（role = 1）
    if (query.exec("SELECT COUNT(*) FROM users WHERE role = 1")) {
        if (query.next()) {
            stats["totalBuyers"] = query.value(0).toInt();
        }
    }
    
    // 统计商家数
    if (query.exec("SELECT COUNT(*) FROM sellers")) {
        if (query.next()) {
            stats["totalSellers"] = query.value(0).toInt();
        }
    }
    
    // 统计待审核商家数（role = 0 或 seller_certifications表中status='待审核'）
    int pendingSellers = 0;
    // 方法1：从users表中统计role=0的用户
    if (query.exec("SELECT COUNT(*) FROM users WHERE role = 0")) {
        if (query.next()) {
            pendingSellers += query.value(0).toInt();
        }
    }
    // 方法2：从seller_certifications表中统计status='待审核'的记录
    if (query.exec("SELECT COUNT(*) FROM seller_certifications WHERE status = '待审核'")) {
        if (query.next()) {
            int certPending = query.value(0).toInt();
            // 取较大值，避免重复计算
            if (certPending > pendingSellers) {
                pendingSellers = certPending;
            }
        }
    }
    stats["pendingSellers"] = pendingSellers;
    
    // 统计图书数
    if (query.exec("SELECT COUNT(*) FROM books")) {
        if (query.next()) {
            stats["totalBooks"] = query.value(0).toInt();
        }
    }
    
    return stats;
}

QJsonObject Database::getSellerDashboardStats(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject stats;
    
    if (!isConnected()) {
        return stats;
    }
    
    if (sellerId <= 0) {
        return stats;
    }
    
    QSqlQuery query(m_db);
    
    // 1. 计算总销售额和总订单数：该卖家的所有订单
    // 方法1：通过merchant_id字段快速统计
    double totalSales = 0.0;
    int totalOrders = 0;
    QSet<QString> countedOrderIds;  // 已统计的订单ID（避免重复）
    
    query.prepare("SELECT order_id, total_amount FROM orders WHERE merchant_id = ?");
    query.addBindValue(sellerId);
    if (query.exec()) {
        while (query.next()) {
            QString orderId = query.value("order_id").toString();
            countedOrderIds.insert(orderId);
            totalOrders++;
            totalSales += query.value("total_amount").toDouble();
        }
    }
    
    // 方法2：检查items JSON中包含该卖家商品的订单（处理一个订单包含多个商家商品的情况）
    QSqlQuery query2(m_db);
    query2.prepare("SELECT order_id, total_amount, items FROM orders WHERE merchant_id != ? OR merchant_id IS NULL");
    query2.addBindValue(sellerId);
    if (query2.exec()) {
        while (query2.next()) {
            QString orderId = query2.value("order_id").toString();
            if (countedOrderIds.contains(orderId)) {
                continue;  // 已统计过，跳过
            }
            
            QString itemsJson = query2.value("items").toString();
            if (itemsJson.isEmpty()) {
                continue;
            }
            
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            if (!doc.isArray()) {
                continue;
            }
            
            QJsonArray items = doc.array();
            bool hasSellerItem = false;
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                int itemMerchantId = -1;
                if (item.contains("merchantId")) {
                    itemMerchantId = item["merchantId"].toInt();
                } else if (item.contains("merchant_id")) {
                    itemMerchantId = item["merchant_id"].toInt();
                } else if (item.contains("sellerId")) {
                    itemMerchantId = item["sellerId"].toInt();
                }
                
                if (itemMerchantId == sellerId) {
                    hasSellerItem = true;
                    break;
                }
            }
            
            if (hasSellerItem) {
                totalOrders++;
                // 注意：对于包含多个商家商品的订单，这里统计的是订单总金额
                // 如果需要更精确的统计，可以只统计该卖家商品的部分金额
                totalSales += query2.value("total_amount").toDouble();
                countedOrderIds.insert(orderId);
            }
        }
    }
    
    // 3. 统计总图书数：该卖家的图书数量（状态为"正常"）
    int totalBooks = 0;
    query.prepare("SELECT COUNT(*) as count FROM books WHERE merchant_id = ? AND (status IS NULL OR status = '' OR status = '正常')");
    query.addBindValue(sellerId);
    if (query.exec() && query.next()) {
        totalBooks = query.value("count").toInt();
    }
    
    // 4. 统计总会员数：所有买家用户数量（role=1，状态为正常）
    int totalMembers = 0;
    query.prepare("SELECT COUNT(*) as count FROM users WHERE role = 1 AND (status IS NULL OR status = '' OR status = '正常')");
    if (query.exec() && query.next()) {
        totalMembers = query.value("count").toInt();
    }
    
    stats["totalSales"] = totalSales;
    stats["totalOrders"] = totalOrders;
    stats["totalBooks"] = totalBooks;
    stats["totalMembers"] = totalMembers;
    
    return stats;
}

QJsonArray Database::getSellerSalesReport(int sellerId, const QString& startDate, const QString& endDate)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray result;
    
    if (!isConnected() || sellerId <= 0) {
        return result;
    }
    
    QSqlQuery query(m_db);
    
    // 查询指定日期范围内的订单，按日期分组统计
    QString sql = "SELECT DATE(order_date) as date, COUNT(*) as count, COALESCE(SUM(total_amount), 0) as amount "
                  "FROM orders WHERE merchant_id = ? AND DATE(order_date) >= ? AND DATE(order_date) <= ? "
                  "GROUP BY DATE(order_date) ORDER BY date ASC";
    query.prepare(sql);
    query.addBindValue(sellerId);
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject item;
            item["date"] = query.value("date").toString();
            item["count"] = query.value("count").toInt();
            item["amount"] = query.value("amount").toDouble();
            result.append(item);
        }
    } else {
        qWarning() << "查询销售报表失败:" << query.lastError().text();
    }
    
    // 如果merchant_id不匹配，还需要检查items JSON中包含该卖家商品的订单
    QSqlQuery query2(m_db);
    query2.prepare("SELECT order_id, DATE(order_date) as date, total_amount, items FROM orders "
                   "WHERE (merchant_id != ? OR merchant_id IS NULL) AND DATE(order_date) >= ? AND DATE(order_date) <= ?");
    query2.addBindValue(sellerId);
    query2.addBindValue(startDate);
    query2.addBindValue(endDate);
    
    if (query2.exec()) {
        QMap<QString, QJsonObject> dateMap;  // 按日期分组
        
        while (query2.next()) {
            QString orderId = query2.value("order_id").toString();
            QString date = query2.value("date").toString();
            double totalAmount = query2.value("total_amount").toDouble();
            QString itemsJson = query2.value("items").toString();
            
            if (itemsJson.isEmpty()) {
                continue;
            }
            
            QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
            if (!doc.isArray()) {
                continue;
            }
            
            QJsonArray items = doc.array();
            bool hasSellerItem = false;
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                int itemMerchantId = -1;
                if (item.contains("merchantId")) {
                    itemMerchantId = item["merchantId"].toInt();
                } else if (item.contains("merchant_id")) {
                    itemMerchantId = item["merchant_id"].toInt();
                } else if (item.contains("sellerId")) {
                    itemMerchantId = item["sellerId"].toInt();
                }
                
                if (itemMerchantId == sellerId) {
                    hasSellerItem = true;
                    break;
                }
            }
            
            if (hasSellerItem) {
                if (!dateMap.contains(date)) {
                    QJsonObject dateItem;
                    dateItem["date"] = date;
                    dateItem["count"] = 0;
                    dateItem["amount"] = 0.0;
                    dateMap[date] = dateItem;
                }
                dateMap[date]["count"] = dateMap[date]["count"].toInt() + 1;
                dateMap[date]["amount"] = dateMap[date]["amount"].toDouble() + totalAmount;
            }
        }
        
        // 合并到结果中
        for (auto it = dateMap.begin(); it != dateMap.end(); ++it) {
            QString date = it.key();
            bool found = false;
            for (int i = 0; i < result.size(); ++i) {
                if (result[i].toObject()["date"].toString() == date) {
                    QJsonObject existing = result[i].toObject();
                    existing["count"] = existing["count"].toInt() + it.value()["count"].toInt();
                    existing["amount"] = existing["amount"].toDouble() + it.value()["amount"].toDouble();
                    result[i] = existing;
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.append(it.value());
            }
        }
        
        // 重新排序
        QJsonArray sortedResult;
        QStringList dates;
        for (int i = 0; i < result.size(); ++i) {
            dates.append(result[i].toObject()["date"].toString());
        }
        dates.sort();
        for (const QString &date : dates) {
            for (int i = 0; i < result.size(); ++i) {
                if (result[i].toObject()["date"].toString() == date) {
                    sortedResult.append(result[i]);
                    break;
                }
            }
        }
        result = sortedResult;
    }
    
    return result;
}

QJsonArray Database::getSellerInventoryReport(int sellerId, const QString& startDate, const QString& endDate)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray result;
    
    if (!isConnected() || sellerId <= 0) {
        return result;
    }
    
    QSqlQuery query(m_db);
    
    // 查询指定日期范围内有订单的图书，按分类统计库存变化
    // 这里统计的是当前库存状态，以及在该日期范围内有销售的图书
    QString sql = "SELECT b.category1, b.category2, COUNT(DISTINCT b.isbn) as book_count, "
                  "SUM(b.stock) as total_stock, COUNT(DISTINCT o.order_id) as order_count "
                  "FROM books b "
                  "LEFT JOIN orders o ON DATE(o.order_date) >= ? AND DATE(o.order_date) <= ? "
                  "AND (o.merchant_id = ? OR JSON_CONTAINS(o.items, JSON_OBJECT('merchantId', ?))) "
                  "WHERE b.merchant_id = ? AND (b.status IS NULL OR b.status = '' OR b.status = '正常') "
                  "GROUP BY b.category1, b.category2 "
                  "ORDER BY b.category1, b.category2";
    query.prepare(sql);
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    query.addBindValue(sellerId);
    query.addBindValue(sellerId);
    query.addBindValue(sellerId);
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject item;
            QString category1 = query.value("category1").toString();
            QString category2 = query.value("category2").toString();
            QString category = category1;
            if (!category2.isEmpty()) {
                category += " - " + category2;
            }
            item["category"] = category;
            item["count"] = query.value("book_count").toInt();
            item["stock"] = query.value("total_stock").toInt();
            item["orderCount"] = query.value("order_count").toInt();
            result.append(item);
        }
    } else {
        // 如果JSON_CONTAINS不支持，使用简化查询
        qWarning() << "查询库存报表失败（尝试简化查询）:" << query.lastError().text();
        
        QSqlQuery query2(m_db);
        query2.prepare("SELECT category1, category2, COUNT(*) as book_count, SUM(stock) as total_stock "
                      "FROM books WHERE merchant_id = ? AND (status IS NULL OR status = '' OR status = '正常') "
                      "GROUP BY category1, category2 ORDER BY category1, category2");
        query2.addBindValue(sellerId);
        
        if (query2.exec()) {
            while (query2.next()) {
                QJsonObject item;
                QString category1 = query2.value("category1").toString();
                QString category2 = query2.value("category2").toString();
                QString category = category1;
                if (!category2.isEmpty()) {
                    category += " - " + category2;
                }
                item["category"] = category;
                item["count"] = query2.value("book_count").toInt();
                item["stock"] = query2.value("total_stock").toInt();
                item["orderCount"] = 0;  // 简化版本不统计订单数
                result.append(item);
            }
        }
    }
    
    return result;
}

QJsonArray Database::getSellerMemberReport(int sellerId, const QString& startDate, const QString& endDate)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray result;
    
    if (!isConnected() || sellerId <= 0) {
        return result;
    }
    
    QSqlQuery query(m_db);
    
    // 查询指定日期范围内注册的会员，按会员等级分组统计
    // 如果register_date为NULL，则包含在统计中（兼容旧数据）
    QString sql = "SELECT COALESCE(NULLIF(member_level, ''), '普通会员') as level, COUNT(*) as count "
                  "FROM users WHERE role = 1 AND (status IS NULL OR status = '' OR status = '正常') "
                  "AND (register_date IS NULL OR register_date = '' OR (DATE(register_date) >= ? AND DATE(register_date) <= ?)) "
                  "GROUP BY COALESCE(NULLIF(member_level, ''), '普通会员') ORDER BY level";
    query.prepare(sql);
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject item;
            QString level = query.value("level").toString();
            if (level.isEmpty()) {
                level = "普通会员";
            }
            item["level"] = level;
            item["count"] = query.value("count").toInt();
            result.append(item);
        }
    } else {
        qWarning() << "查询会员报表失败:" << query.lastError().text();
    }
    
    return result;
}

// 初始化示例图书数据
bool Database::initSampleBooks()
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 检查是否已有图书数据，如果有则跳过初始化
    QSqlQuery checkQuery(m_db);
    if (checkQuery.exec("SELECT COUNT(*) FROM books") && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if (count > 0) {
            qDebug() << "数据库中已有" << count << "本图书，跳过示例数据初始化";
            return true;
        }
    }
    
    qDebug() << "开始初始化示例图书数据...";
    
    // 创建示例图书数据（使用空白封面图片，即空字符串）
    QList<QJsonObject> sampleBooks;
    
    // 文学小说类
    QJsonObject book1;
    book1["isbn"] = "9787111544937";
    book1["title"] = "三体";
    book1["author"] = "刘慈欣";
    book1["category1"] = "文学小说";
    book1["category2"] = "科幻/奇幻";
    book1["merchant_id"] = 1;
    book1["price"] = 39.00;
    book1["stock"] = 100;
    book1["status"] = "正常";
    book1["coverImage"] = "";  // 空白封面
    sampleBooks.append(book1);
    
    QJsonObject book2;
    book2["isbn"] = "9787506365437";
    book2["title"] = "活着";
    book2["author"] = "余华";
    book2["category1"] = "文学小说";
    book2["category2"] = "当代小说";
    book2["merchant_id"] = 1;
    book2["price"] = 28.00;
    book2["stock"] = 80;
    book2["status"] = "正常";
    book2["coverImage"] = "";
    sampleBooks.append(book2);
    
    QJsonObject book3;
    book3["isbn"] = "9787020002207";
    book3["title"] = "红楼梦";
    book3["author"] = "曹雪芹";
    book3["category1"] = "文学小说";
    book3["category2"] = "中国古典文学";
    book3["merchant_id"] = 1;
    book3["price"] = 68.00;
    book3["stock"] = 50;
    book3["status"] = "正常";
    book3["coverImage"] = "";
    sampleBooks.append(book3);
    
    // 人文社科类
    QJsonObject book4;
    book4["isbn"] = "9787101003048";
    book4["title"] = "中国通史";
    book4["author"] = "吕思勉";
    book4["category1"] = "人文社科";
    book4["category2"] = "历史（中国史/世界史）";
    book4["merchant_id"] = 1;
    book4["price"] = 88.00;
    book4["stock"] = 60;
    book4["status"] = "正常";
    book4["coverImage"] = "";
    sampleBooks.append(book4);
    
    QJsonObject book5;
    book5["isbn"] = "9787108009821";
    book5["title"] = "人类简史";
    book5["author"] = "尤瓦尔·赫拉利";
    book5["category1"] = "人文社科";
    book5["category2"] = "历史（中国史/世界史）";
    book5["merchant_id"] = 1;
    book5["price"] = 68.00;
    book5["stock"] = 70;
    book5["status"] = "正常";
    book5["coverImage"] = "";
    sampleBooks.append(book5);
    
    // 经济管理类
    QJsonObject book6;
    book6["isbn"] = "9787508684031";
    book6["title"] = "经济学原理";
    book6["author"] = "曼昆";
    book6["category1"] = "经济管理";
    book6["category2"] = "经济学理论";
    book6["merchant_id"] = 1;
    book6["price"] = 128.00;
    book6["stock"] = 40;
    book6["status"] = "正常";
    book6["coverImage"] = "";
    sampleBooks.append(book6);
    
    QJsonObject book7;
    book7["isbn"] = "9787508684032";
    book7["title"] = "从0到1";
    book7["author"] = "彼得·蒂尔";
    book7["category1"] = "经济管理";
    book7["category2"] = "企业管理";
    book7["merchant_id"] = 1;
    book7["price"] = 45.00;
    book7["stock"] = 90;
    book7["status"] = "正常";
    book7["coverImage"] = "";
    sampleBooks.append(book7);
    
    // 科学技术类
    QJsonObject book8;
    book8["isbn"] = "9787111544938";
    book8["title"] = "深入理解计算机系统";
    book8["author"] = "Randal E. Bryant";
    book8["category1"] = "科学技术";
    book8["category2"] = "计算机/互联网";
    book8["merchant_id"] = 1;
    book8["price"] = 139.00;
    book8["stock"] = 55;
    book8["status"] = "正常";
    book8["coverImage"] = "";
    sampleBooks.append(book8);
    
    QJsonObject book9;
    book9["isbn"] = "9787111213826";
    book9["title"] = "算法导论";
    book9["author"] = "Thomas H. Cormen";
    book9["category1"] = "科学技术";
    book9["category2"] = "计算机/互联网";
    book9["merchant_id"] = 1;
    book9["price"] = 118.00;
    book9["stock"] = 45;
    book9["status"] = "正常";
    book9["coverImage"] = "";
    sampleBooks.append(book9);
    
    QJsonObject book10;
    book10["isbn"] = "9787508647357";
    book10["title"] = "时间简史";
    book10["author"] = "史蒂芬·霍金";
    book10["category1"] = "科学技术";
    book10["category2"] = "科普读物";
    book10["merchant_id"] = 1;
    book10["price"] = 45.00;
    book10["stock"] = 75;
    book10["status"] = "正常";
    book10["coverImage"] = "";
    sampleBooks.append(book10);
    
    // 教育考试类
    QJsonObject book11;
    book11["isbn"] = "9787107177264";
    book11["title"] = "新概念英语";
    book11["author"] = "L.G. Alexander";
    book11["category1"] = "教育考试";
    book11["category2"] = "外语学习";
    book11["merchant_id"] = 1;
    book11["price"] = 38.00;
    book11["stock"] = 120;
    book11["status"] = "正常";
    book11["coverImage"] = "";
    sampleBooks.append(book11);
    
    // 生活艺术类
    QJsonObject book12;
    book12["isbn"] = "9787508647358";
    book12["title"] = "舌尖上的中国";
    book12["author"] = "中央电视台";
    book12["category1"] = "生活艺术";
    book12["category2"] = "烹饪/美食";
    book12["merchant_id"] = 1;
    book12["price"] = 58.00;
    book12["stock"] = 65;
    book12["status"] = "正常";
    book12["coverImage"] = "";
    sampleBooks.append(book12);
    
    // 少儿童书类
    QJsonObject book13;
    book13["isbn"] = "9787533254789";
    book13["title"] = "小王子";
    book13["author"] = "安托万·德·圣埃克苏佩里";
    book13["category1"] = "少儿童书";
    book13["category2"] = "儿童文学";
    book13["merchant_id"] = 1;
    book13["price"] = 25.00;
    book13["stock"] = 150;
    book13["status"] = "正常";
    book13["coverImage"] = "";
    sampleBooks.append(book13);
    
    // 批量插入示例图书
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO books (isbn, title, author, category1, category2, merchant_id, price, stock, status, cover_image) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    int successCount = 0;
    for (const QJsonObject &book : sampleBooks) {
        insertQuery.addBindValue(book["isbn"].toString());
        insertQuery.addBindValue(book["title"].toString());
        insertQuery.addBindValue(book["author"].toString());
        insertQuery.addBindValue(book["category1"].toString());
        insertQuery.addBindValue(book["category2"].toString());
        insertQuery.addBindValue(book["merchant_id"].toInt());
        insertQuery.addBindValue(book["price"].toDouble());
        insertQuery.addBindValue(book["stock"].toInt());
        insertQuery.addBindValue(book["status"].toString());
        insertQuery.addBindValue(book["coverImage"].toString());
        
        if (insertQuery.exec()) {
            successCount++;
        } else {
            qWarning() << "插入示例图书失败:" << book["title"].toString() << insertQuery.lastError().text();
        }
    }
    
    qDebug() << "示例图书数据初始化完成，成功插入" << successCount << "本图书";
    return successCount > 0;
}

// ==========================================
// 卖家认证相关
// ==========================================

bool Database::applySellerCertification(int userId, const QString& username, const QString& password, 
                                       const QString& email, const QString& licenseImageBase64)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法保存营业执照图片";
        return false;
    }
    
    // 首先更新users表的license_image_base64字段和role字段
    // 将role设置为0表示"审核中"（买家申请成为卖家且在审核中）
    // 注意：无论用户之前是什么role，提交申请时都设置为0（审核中）
    QSqlQuery updateUserQuery(m_db);
    updateUserQuery.prepare("UPDATE users SET license_image_base64 = ?, role = 0 WHERE user_id = ?");
    updateUserQuery.addBindValue(licenseImageBase64);
    updateUserQuery.addBindValue(userId);
    
    if (!updateUserQuery.exec()) {
        qWarning() << "更新users表license_image_base64字段失败:" << updateUserQuery.lastError().text();
        qWarning() << "SQL错误:" << updateUserQuery.lastError().databaseText();
        qWarning() << "用户ID:" << userId << "图片大小:" << licenseImageBase64.size() << "字节";
        return false;
    }
    
    if (updateUserQuery.numRowsAffected() == 0) {
        qWarning() << "更新users表失败：未找到用户ID:" << userId;
        return false;
    }
    
    qDebug() << "✓ 已更新users表的license_image_base64字段和role=0（审核中），用户ID:" << userId;
    
    // 同时保存到seller_certifications表（用于审核流程）
    // 确保状态始终为"审核中"，即使之前有记录且状态是"已认证"
    QSqlQuery query(m_db);
    // 使用 INSERT ... ON DUPLICATE KEY UPDATE 来处理重复申请
    // 强制将status设置为"审核中"，确保不会因为之前的状态而错误显示
    query.prepare("INSERT INTO seller_certifications (user_id, username, password, email, license_image, status, apply_time) "
                 "VALUES (?, ?, ?, ?, ?, '审核中', CURRENT_TIMESTAMP) "
                 "ON DUPLICATE KEY UPDATE "
                 "username = VALUES(username), "
                 "password = VALUES(password), "
                 "email = VALUES(email), "
                 "license_image = VALUES(license_image), "
                 "status = '审核中', "
                 "apply_time = CURRENT_TIMESTAMP");
    query.addBindValue(userId);
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(email);
    query.addBindValue(licenseImageBase64);
    
    if (!query.exec()) {
        qWarning() << "保存到seller_certifications表失败:" << query.lastError().text();
        qWarning() << "SQL错误详情:" << query.lastError().databaseText();
        // 即使seller_certifications表保存失败，users表已经更新成功，所以返回true
        qDebug() << "注意：users表已更新，但seller_certifications表更新失败";
    } else {
        qDebug() << "✓ 已保存到seller_certifications表，状态强制设置为'审核中'";
    }
    
    qDebug() << "卖家认证申请已提交，用户ID:" << userId << "用户名:" << username << "状态:审核中";
    return true;
}

QJsonObject Database::getSellerCertification(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject result;
    
    if (!isConnected()) {
        return result;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM seller_certifications WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "查询卖家认证信息失败:" << query.lastError().text();
        return result;
    }
    
    if (query.next()) {
        result["certId"] = query.value("cert_id").toInt();
        result["userId"] = query.value("user_id").toInt();
        result["username"] = query.value("username").toString();
        result["email"] = query.value("email").toString();
        result["status"] = query.value("status").toString();
        result["applyTime"] = query.value("apply_time").toString();
        result["approveTime"] = query.value("approve_time").toString();
        result["message"] = "已申请认证";
        
        // 从users表获取license_image_base64
        QSqlQuery userQuery(m_db);
        userQuery.prepare("SELECT license_image_base64 FROM users WHERE user_id = ?");
        userQuery.addBindValue(userId);
        if (userQuery.exec() && userQuery.next()) {
            QString licenseImage = userQuery.value("license_image_base64").toString();
            if (!licenseImage.isEmpty()) {
                result["licenseImage"] = licenseImage;
            }
        }
    }
    
    return result;
}

QJsonArray Database::getPendingSellerCertifications()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray result;
    
    if (!isConnected()) {
        return result;
    }
    
    QSqlQuery query(m_db);
    // 查询状态为"审核中"或"待审核"的认证申请
    query.prepare("SELECT * FROM seller_certifications WHERE status = '审核中' OR status = '待审核' ORDER BY apply_time DESC");
    
    if (!query.exec()) {
        qWarning() << "查询待审核商家认证申请失败:" << query.lastError().text();
        return result;
    }
    
    while (query.next()) {
        QJsonObject cert;
        cert["certId"] = query.value("cert_id").toInt();
        cert["userId"] = query.value("user_id").toInt();
        cert["username"] = query.value("username").toString();
        cert["email"] = query.value("email").toString();
        cert["status"] = query.value("status").toString();
        cert["applyTime"] = query.value("apply_time").toString();
        
        // 从users表获取license_image_base64
        QSqlQuery userQuery(m_db);
        userQuery.prepare("SELECT license_image_base64 FROM users WHERE user_id = ?");
        userQuery.addBindValue(query.value("user_id").toInt());
        if (userQuery.exec() && userQuery.next()) {
            QString licenseImage = userQuery.value("license_image_base64").toString();
            if (!licenseImage.isEmpty()) {
                cert["licenseImage"] = licenseImage;
            }
        }
        
        result.append(cert);
    }
    
    return result;
}

bool Database::approveSellerCertification(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 先获取认证信息（状态为"审核中"或"待审核"）
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM seller_certifications WHERE user_id = ? AND (status = '审核中' OR status = '待审核')");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "未找到待审核的认证申请，用户ID:" << userId;
        return false;
    }
    
    QString username = query.value("username").toString();
    QString password = query.value("password").toString();
    QString email = query.value("email").toString();
    QString licenseImageBase64 = query.value("license_image").toString();
    
    // 从users表获取phone_number、address、balance、status
    QSqlQuery userQuery(m_db);
    userQuery.prepare("SELECT phone_number, address, balance, status FROM users WHERE user_id = ?");
    userQuery.addBindValue(userId);
    
    QString phoneNumber;
    QString address;
    double balance = 0.0;
    QString userStatus = "正常";
    
    if (userQuery.exec() && userQuery.next()) {
        phoneNumber = userQuery.value("phone_number").toString();
        address = userQuery.value("address").toString();
        balance = userQuery.value("balance").toDouble();
        userStatus = userQuery.value("status").toString();
    }
    
    // 更新users表的role为2（卖家）
    QSqlQuery updateRoleQuery(m_db);
    updateRoleQuery.prepare("UPDATE users SET role = 2 WHERE user_id = ?");
    updateRoleQuery.addBindValue(userId);
    
    if (!updateRoleQuery.exec()) {
        qWarning() << "更新用户role失败:" << updateRoleQuery.lastError().text();
        return false;
    }
    
    // 将用户添加到卖家表，包含所有字段（与users表除role外一一对应）
    // 如果用户被封禁，商家状态也应设为"封禁"
    QString sellerStatus = (userStatus == "封禁") ? "封禁" : "正常";
    
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO sellers (seller_name, password, email, phone_number, address, balance, license_image_base64, register_date, status) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?, CURDATE(), ?)");
    insertQuery.addBindValue(username);
    insertQuery.addBindValue(password);
    insertQuery.addBindValue(email);
    insertQuery.addBindValue(phoneNumber);
    insertQuery.addBindValue(address);
    insertQuery.addBindValue(balance);
    insertQuery.addBindValue(licenseImageBase64);
    insertQuery.addBindValue(sellerStatus);
    
    if (!insertQuery.exec()) {
        qWarning() << "添加用户到卖家表失败:" << insertQuery.lastError().text();
        // 如果是因为用户名已存在，可能是已经添加过了，需要更新状态
        if (insertQuery.lastError().text().contains("Duplicate")) {
            qDebug() << "用户可能已经是卖家，更新商家状态";
            QSqlQuery updateSellerQuery(m_db);
            updateSellerQuery.prepare("UPDATE sellers SET status = ? WHERE seller_name = ?");
            updateSellerQuery.addBindValue(sellerStatus);
            updateSellerQuery.addBindValue(username);
            if (!updateSellerQuery.exec()) {
                qWarning() << "更新商家状态失败:" << updateSellerQuery.lastError().text();
            } else {
                // 如果用户被封禁，商家图书也应下架
                if (userStatus == "封禁") {
                    QSqlQuery getSellerIdQuery(m_db);
                    getSellerIdQuery.prepare("SELECT seller_id FROM sellers WHERE seller_name = ?");
                    getSellerIdQuery.addBindValue(username);
                    if (getSellerIdQuery.exec() && getSellerIdQuery.next()) {
                        int sellerId = getSellerIdQuery.value("seller_id").toInt();
                        updateBooksStatusBySellerId(sellerId, "下架");
                    }
                }
            }
        } else {
            return false;
        }
    } else {
        // 如果用户被封禁，商家图书也应下架
        if (userStatus == "封禁") {
            int sellerId = insertQuery.lastInsertId().toInt();
            updateBooksStatusBySellerId(sellerId, "下架");
        }
    }
    
    // 更新认证状态为已认证
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE seller_certifications SET status = '已认证', approve_time = NOW() WHERE user_id = ?");
    updateQuery.addBindValue(userId);
    
    if (!updateQuery.exec()) {
        qWarning() << "更新认证状态失败:" << updateQuery.lastError().text();
        return false;
    }
    
    qDebug() << "卖家认证审核通过，用户ID:" << userId << "用户名:" << username;
    return true;
}

bool Database::rejectSellerCertification(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 更新认证状态为已拒绝
    QSqlQuery query(m_db);
    query.prepare("UPDATE seller_certifications SET status = '已拒绝', approve_time = NOW() WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新认证状态失败:" << query.lastError().text();
        return false;
    }
    
    // 将users表的role改回1（买家）
    QSqlQuery updateRoleQuery(m_db);
    updateRoleQuery.prepare("UPDATE users SET role = 1 WHERE user_id = ?");
    updateRoleQuery.addBindValue(userId);
    
    if (!updateRoleQuery.exec()) {
        qWarning() << "更新用户role失败:" << updateRoleQuery.lastError().text();
        return false;
    }
    
    qDebug() << "✓ 审核已拒绝，用户ID:" << userId << "，role已改回1（买家）";
    return true;
}

// ==========================================
// 聊天相关
// ==========================================

bool Database::saveChatMessage(int senderId, const QString& senderType, int receiverId, const QString& receiverType, const QString& message)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    if (message.trimmed().isEmpty()) {
        qWarning() << "保存聊天消息失败：消息内容为空";
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO chat_messages (sender_id, sender_type, receiver_id, receiver_type, message_content) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(senderId);
    query.addBindValue(senderType);
    // receiverId为-1或0时，表示发送给所有管理员/客服，设置为NULL
    if (receiverId > 0) {
        query.addBindValue(receiverId);
    } else {
        query.addBindValue(QVariant());
    }
    // receiverType为空时，表示发送给所有管理员/客服
    if (!receiverType.isEmpty()) {
        query.addBindValue(receiverType);
    } else {
        query.addBindValue(QVariant());
    }
    query.addBindValue(message);
    
    if (!query.exec()) {
        qWarning() << "保存聊天消息失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "聊天消息已保存，发送者ID:" << senderId << "类型:" << senderType 
             << "接收者ID:" << receiverId << "类型:" << receiverType;
    return true;
}

QJsonArray Database::getChatHistory(int userId, const QString& userType, int otherUserId, const QString& otherUserType)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray messages;
    
    if (!isConnected()) {
        return messages;
    }
    
    QSqlQuery query(m_db);
    QString sql;
    
    if (otherUserId > 0 && !otherUserType.isEmpty()) {
        // 获取与特定用户的聊天记录（双向）
        // 如果userId是管理员（999999），还需要包含对方用户发送给客服的消息（receiver_id IS NULL）
        if (userId == 999999 && userType == "admin") {
            // 管理员查看与特定用户的聊天：包含双向消息 + 用户发送给客服的消息
            sql = "SELECT * FROM chat_messages "
                  "WHERE ((sender_id = ? AND sender_type = ? AND receiver_id = ? AND receiver_type = ?) "
                  "   OR (sender_id = ? AND sender_type = ? AND receiver_id = ? AND receiver_type = ?) "
                  "   OR (sender_id = ? AND sender_type = ? AND receiver_id IS NULL AND receiver_type IS NULL)) "
                  "ORDER BY send_time ASC";
            query.prepare(sql);
            query.addBindValue(userId);
            query.addBindValue(userType);
            query.addBindValue(otherUserId);
            query.addBindValue(otherUserType);
            query.addBindValue(otherUserId);
            query.addBindValue(otherUserType);
            query.addBindValue(userId);
            query.addBindValue(userType);
            query.addBindValue(otherUserId);
            query.addBindValue(otherUserType);
        } else {
            // 普通用户之间的聊天记录（双向）
            sql = "SELECT * FROM chat_messages "
                  "WHERE ((sender_id = ? AND sender_type = ? AND receiver_id = ? AND receiver_type = ?) "
                  "   OR (sender_id = ? AND sender_type = ? AND receiver_id = ? AND receiver_type = ?)) "
                  "ORDER BY send_time ASC";
            query.prepare(sql);
            query.addBindValue(userId);
            query.addBindValue(userType);
            query.addBindValue(otherUserId);
            query.addBindValue(otherUserType);
            query.addBindValue(otherUserId);
            query.addBindValue(otherUserType);
            query.addBindValue(userId);
            query.addBindValue(userType);
        }
    } else {
        // 获取所有与当前用户相关的聊天记录（包括发送和接收）
        // 对于买家/卖家发送给客服的消息（receiver_id IS NULL），也要包含在内
        sql = "SELECT * FROM chat_messages "
              "WHERE (sender_id = ? AND sender_type = ?) "
              "   OR (receiver_id = ? AND receiver_type = ?) "
              "   OR (sender_id = ? AND sender_type = ? AND receiver_id IS NULL AND receiver_type IS NULL) "
              "ORDER BY send_time ASC";
        query.prepare(sql);
        query.addBindValue(userId);
        query.addBindValue(userType);
        query.addBindValue(userId);
        query.addBindValue(userType);
        query.addBindValue(userId);
        query.addBindValue(userType);
    }
    
    if (!query.exec()) {
        qWarning() << "获取聊天历史失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        QJsonObject msg;
        msg["messageId"] = query.value("message_id").toInt();
        msg["senderId"] = query.value("sender_id").toInt();
        msg["senderType"] = query.value("sender_type").toString();
        msg["receiverId"] = query.value("receiver_id").toInt();
        msg["receiverType"] = query.value("receiver_type").toString();
        msg["content"] = query.value("message_content").toString();
        msg["sendTime"] = query.value("send_time").toString();
        msg["isRead"] = query.value("is_read").toInt() == 1;
        
        messages.append(msg);
    }
    
    qDebug() << "获取聊天历史，用户ID:" << userId << "类型:" << userType << "消息数量:" << messages.size();
    return messages;
}

QJsonArray Database::getAllChatMessagesForAdmin()
{
    QMutexLocker locker(&m_mutex);
    QJsonArray messages;
    
    if (!isConnected()) {
        return messages;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chat_messages ORDER BY send_time DESC LIMIT 1000");
    
    if (!query.exec()) {
        qWarning() << "获取所有聊天消息失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        QJsonObject msg;
        msg["messageId"] = query.value("message_id").toInt();
        msg["senderId"] = query.value("sender_id").toInt();
        msg["senderType"] = query.value("sender_type").toString();
        msg["receiverId"] = query.value("receiver_id").toInt();
        msg["receiverType"] = query.value("receiver_type").toString();
        msg["content"] = query.value("message_content").toString();
        msg["sendTime"] = query.value("send_time").toString();
        msg["isRead"] = query.value("is_read").toInt() == 1;
        
        messages.append(msg);
    }
    
    qDebug() << "管理员获取所有聊天消息，数量:" << messages.size();
    return messages;
}

// ===== 会员等级相关函数 =====

QString Database::calculateMemberLevel(double totalRecharge)
{
    // 会员等级划分：
    // 普通会员：0元（免费注册），基础服务、积分1倍
    // 银卡会员：200元，9.5折
    // 金卡会员：1000元，9折
    // 铂金会员：3000元，8.5折
    // 钻石会员：8000元，8折
    // 黑钻会员：20000元，7.5折
    
    if (totalRecharge >= 20000.0) {
        return "黑钻会员";
    } else if (totalRecharge >= 8000.0) {
        return "钻石会员";
    } else if (totalRecharge >= 3000.0) {
        return "铂金会员";
    } else if (totalRecharge >= 1000.0) {
        return "金卡会员";
    } else if (totalRecharge >= 200.0) {
        return "银卡会员";
    } else {
        return "普通会员";
    }
}

double Database::getMemberDiscount(const QString& memberLevel)
{
    // 返回折扣率（0.75-1.0）
    if (memberLevel == "黑钻会员") {
        return 0.75;  // 7.5折
    } else if (memberLevel == "钻石会员") {
        return 0.80;  // 8折
    } else if (memberLevel == "铂金会员") {
        return 0.85;  // 8.5折
    } else if (memberLevel == "金卡会员") {
        return 0.90;  // 9折
    } else if (memberLevel == "银卡会员") {
        return 0.95;  // 9.5折
    } else {
        return 1.0;  // 普通会员无折扣
    }
}

bool Database::updateMemberLevel(int userId)
{
    QMutexLocker locker(&m_mutex);
    return updateMemberLevelUnlocked(userId);
}

bool Database::updateMemberLevelUnlocked(int userId)
{
    // 注意：此函数不获取锁，必须在已持有 m_mutex 锁的情况下调用
    
    if (!isConnected()) {
        return false;
    }
    
    // 获取用户的累计充值总额
    QSqlQuery query(m_db);
    query.prepare("SELECT total_recharge FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "获取用户累计充值总额失败，用户ID:" << userId;
        return false;
    }
    
    double totalRecharge = query.value("total_recharge").toDouble();
    
    // 计算新的会员等级
    QString newMemberLevel = calculateMemberLevel(totalRecharge);
    
    // 更新会员等级
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE users SET member_level = ? WHERE user_id = ?");
    updateQuery.addBindValue(newMemberLevel);
    updateQuery.addBindValue(userId);
    
    if (!updateQuery.exec()) {
        qWarning() << "更新会员等级失败，用户ID:" << userId << "错误:" << updateQuery.lastError().text();
        return false;
    }
    
    qDebug() << "会员等级已更新，用户ID:" << userId << "累计充值:" << totalRecharge << "会员等级:" << newMemberLevel;
    return true;
}

// ===== 积分相关函数 =====

int Database::calculatePoints(double rechargeAmount)
{
    // 每充值100元获得1积分
    return static_cast<int>(rechargeAmount / 100.0);
}

bool Database::addPoints(int userId, int points)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    if (points == 0) {
        return true;  // 没有积分变化，返回成功
    }
    
    // 如果是负数（扣除积分），需要检查积分是否足够（直接查询，避免死锁）
    if (points < 0) {
        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT points FROM users WHERE user_id = ?");
        checkQuery.addBindValue(userId);
        if (checkQuery.exec() && checkQuery.next()) {
            int currentPoints = checkQuery.value("points").toInt();
            if (currentPoints < -points) {
                qWarning() << "扣除积分失败，用户ID:" << userId << "当前积分:" << currentPoints << "需要扣除:" << -points;
                return false;
            }
        } else {
            qWarning() << "查询用户积分失败，用户ID:" << userId;
            return false;
        }
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET points = points + ? WHERE user_id = ?");
    query.addBindValue(points);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "更新用户积分失败，用户ID:" << userId << "积分变化:" << points << "错误:" << query.lastError().text();
        return false;
    }
    
    // 验证更新是否成功
    int affectedRows = query.numRowsAffected();
    if (affectedRows == 0) {
        qWarning() << "更新用户积分失败，用户ID:" << userId << "未找到用户";
        return false;
    }
    
    if (points > 0) {
        qDebug() << "用户积分已增加，用户ID:" << userId << "增加积分:" << points;
    } else {
        qDebug() << "用户积分已扣除，用户ID:" << userId << "扣除积分:" << -points;
    }
    return true;
}

int Database::getUserPoints(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return 0;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT points FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "获取用户积分失败，用户ID:" << userId;
        return 0;
    }
    
    return query.value("points").toInt();
}

bool Database::canParticipateLottery(int userId)
{
    // 累计满3积分就可以参与抽奖活动
    int points = getUserPoints(userId);
    return points >= 3;
}

// ===== 商家积分和会员等级相关函数 =====

bool Database::rechargeSellerBalance(int sellerId, double amount)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    if (amount <= 0) {
        qWarning() << "充值金额必须大于0，商家ID:" << sellerId << "金额:" << amount;
        return false;
    }
    
    // 计算本次充值获得的积分（每100元1积分）
    int pointsToAdd = calculatePoints(amount);
    
    QSqlQuery query(m_db);
    // 同时更新余额、累计充值总额和积分
    query.prepare("UPDATE sellers SET balance = balance + ?, total_recharge = total_recharge + ?, points = points + ? WHERE seller_id = ?");
    query.addBindValue(amount);
    query.addBindValue(amount);
    query.addBindValue(pointsToAdd);
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "充值商家余额失败:" << query.lastError().text();
        return false;
    }
    
    // 更新会员等级（使用不获取锁的版本，因为已经持有锁）
    updateSellerMemberLevelUnlocked(sellerId);
    
    // 获取更新后的余额和积分
    QSqlQuery selectQuery(m_db);
    selectQuery.prepare("SELECT balance, points FROM sellers WHERE seller_id = ?");
    selectQuery.addBindValue(sellerId);
    if (selectQuery.exec() && selectQuery.next()) {
        double newBalance = selectQuery.value("balance").toDouble();
        int newPoints = selectQuery.value("points").toInt();
        qDebug() << "商家余额充值成功，商家ID:" << sellerId << "充值金额:" << amount 
                 << "新余额:" << newBalance << "获得积分:" << pointsToAdd << "总积分:" << newPoints;
    }
    
    return true;
}

bool Database::updateSellerMemberLevel(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    return updateSellerMemberLevelUnlocked(sellerId);
}

bool Database::updateSellerMemberLevelUnlocked(int sellerId)
{
    // 注意：此函数不获取锁，必须在已持有 m_mutex 锁的情况下调用
    
    if (!isConnected()) {
        return false;
    }
    
    // 获取商家的累计充值总额
    QSqlQuery query(m_db);
    query.prepare("SELECT total_recharge FROM sellers WHERE seller_id = ?");
    query.addBindValue(sellerId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "获取商家累计充值总额失败，商家ID:" << sellerId;
        return false;
    }
    
    double totalRecharge = query.value("total_recharge").toDouble();
    
    // 计算新的会员等级
    QString newMemberLevel = calculateMemberLevel(totalRecharge);
    
    // 更新会员等级
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE sellers SET member_level = ? WHERE seller_id = ?");
    updateQuery.addBindValue(newMemberLevel);
    updateQuery.addBindValue(sellerId);
    
    if (!updateQuery.exec()) {
        qWarning() << "更新商家会员等级失败，商家ID:" << sellerId << "错误:" << updateQuery.lastError().text();
        return false;
    }
    
    qDebug() << "商家会员等级已更新，商家ID:" << sellerId << "累计充值:" << totalRecharge << "会员等级:" << newMemberLevel;
    return true;
}

// ===== 评论相关函数 =====

bool Database::addReview(int userId, const QString& bookId, int rating, const QString& comment)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法添加评论";
        return false;
    }
    
    // 验证评分范围（1-5）
    if (rating < 1 || rating > 5) {
        qWarning() << "评分超出范围，应为1-5分，实际:" << rating;
        return false;
    }
    
    QSqlQuery query(m_db);
    // 使用INSERT ... ON DUPLICATE KEY UPDATE，如果用户已评论过该商品，则更新评论
    query.prepare("INSERT INTO reviews (user_id, book_id, rating, comment, review_time) "
                  "VALUES (?, ?, ?, ?, NOW()) "
                  "ON DUPLICATE KEY UPDATE rating = ?, comment = ?, review_time = NOW()");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    query.addBindValue(rating);
    query.addBindValue(comment);
    query.addBindValue(rating);
    query.addBindValue(comment);
    
    if (!query.exec()) {
        qWarning() << "添加评论失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "评论添加成功，用户ID:" << userId << "商品ID:" << bookId << "评分:" << rating;
    return true;
}

QJsonArray Database::getBookReviews(const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray reviews;
    
    if (!isConnected()) {
        return reviews;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT r.*, u.username FROM reviews r "
                  "LEFT JOIN users u ON r.user_id = u.user_id "
                  "WHERE r.book_id = ? ORDER BY r.review_time DESC");
    query.addBindValue(bookId);
    
    if (!query.exec()) {
        qWarning() << "获取商品评论失败:" << query.lastError().text();
        return reviews;
    }
    
    while (query.next()) {
        QJsonObject review;
        review["reviewId"] = query.value("review_id").toInt();
        review["userId"] = query.value("user_id").toInt();
        review["username"] = query.value("username").toString();
        review["bookId"] = query.value("book_id").toString();
        review["rating"] = query.value("rating").toInt();
        review["comment"] = query.value("comment").toString();
        review["reviewTime"] = query.value("review_time").toString();
        
        reviews.append(review);
    }
    
    qDebug() << "获取商品评论成功，商品ID:" << bookId << "评论数:" << reviews.size();
    return reviews;
}

QJsonObject Database::getBookRatingStats(const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    QJsonObject stats;
    
    if (!isConnected()) {
        stats["averageRating"] = 0.0;
        stats["reviewCount"] = 0;
        stats["hasRating"] = false;
        return stats;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT AVG(rating) as avg_rating, COUNT(*) as review_count "
                  "FROM reviews WHERE book_id = ?");
    query.addBindValue(bookId);
    
    if (!query.exec() || !query.next()) {
        stats["averageRating"] = 0.0;
        stats["reviewCount"] = 0;
        stats["hasRating"] = false;
        return stats;
    }
    
    int reviewCount = query.value("review_count").toInt();
    double avgRating = query.value("avg_rating").toDouble();
    
    stats["averageRating"] = avgRating;
    stats["reviewCount"] = reviewCount;
    stats["hasRating"] = reviewCount > 0;
    
    qDebug() << "获取商品评分统计，商品ID:" << bookId << "平均分:" << avgRating << "评论数:" << reviewCount;
    return stats;
}

bool Database::hasUserReviewedBook(int userId, const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM reviews WHERE user_id = ? AND book_id = ?");
    query.addBindValue(userId);
    query.addBindValue(bookId);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

bool Database::hasUserPurchasedBook(int userId, const QString& bookId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return false;
    }
    
    // 查询用户的订单，检查订单中的items JSON是否包含该商品
    QSqlQuery query(m_db);
    query.prepare("SELECT items FROM orders WHERE user_id = ? AND status IN ('已支付', '已发货', '已完成')");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "查询用户订单失败:" << query.lastError().text();
        return false;
    }
    
    while (query.next()) {
        QString itemsJson = query.value("items").toString();
        if (itemsJson.isEmpty()) {
            continue;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(itemsJson.toUtf8());
        if (!doc.isArray()) {
            continue;
        }
        
        QJsonArray items = doc.array();
        for (const QJsonValue &itemVal : items) {
            QJsonObject item = itemVal.toObject();
            QString itemBookId = item["bookId"].toString();
            if (itemBookId == bookId) {
                qDebug() << "用户已购买该商品，用户ID:" << userId << "商品ID:" << bookId;
                return true;
            }
        }
    }
    
    return false;
}

QJsonArray Database::getSellerReviews(int sellerId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray reviews;
    
    if (!isConnected()) {
        return reviews;
    }
    
    // 通过JOIN查询：从reviews表关联books表，获取该卖家的所有商品评论
    QSqlQuery query(m_db);
    query.prepare("SELECT r.*, u.username, b.title as book_title, b.isbn as book_isbn "
                  "FROM reviews r "
                  "LEFT JOIN users u ON r.user_id = u.user_id "
                  "LEFT JOIN books b ON r.book_id = b.isbn "
                  "WHERE b.merchant_id = ? "
                  "ORDER BY r.review_time DESC");
    query.addBindValue(sellerId);
    
    if (!query.exec()) {
        qWarning() << "获取卖家评论失败:" << query.lastError().text();
        return reviews;
    }
    
    while (query.next()) {
        QJsonObject review;
        review["reviewId"] = query.value("review_id").toInt();
        review["userId"] = query.value("user_id").toInt();
        review["username"] = query.value("username").toString();
        review["bookId"] = query.value("book_isbn").toString();
        review["bookTitle"] = query.value("book_title").toString();
        review["rating"] = query.value("rating").toInt();
        review["comment"] = query.value("comment").toString();
        review["reviewTime"] = query.value("review_time").toString();
        
        reviews.append(review);
    }
    
    qDebug() << "获取卖家评论成功，卖家ID:" << sellerId << "评论数:" << reviews.size();
    return reviews;
}

// ===== 抽奖和奖品相关函数 =====


bool Database::addUserCoupon(int userId, double couponValue)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法添加优惠券";
        return false;
    }
    
    QSqlQuery query(m_db);
    QString couponType = QString("%1元优惠券").arg(couponValue, 0, 'f', 0);
    QDateTime expireTime = QDateTime::currentDateTime().addDays(30);  // 30天后过期
    
    query.prepare("INSERT INTO user_coupons (user_id, coupon_type, coupon_value, status, expire_time) "
                  "VALUES (?, ?, ?, '未使用', ?)");
    query.addBindValue(userId);
    query.addBindValue(couponType);
    query.addBindValue(couponValue);
    query.addBindValue(expireTime.toString("yyyy-MM-dd hh:mm:ss"));
    
    if (!query.exec()) {
        qWarning() << "添加用户优惠券失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "添加用户优惠券成功，用户ID:" << userId << "优惠券面额:" << couponValue;
    return true;
}

QJsonArray Database::getUserCoupons(int userId)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray coupons;
    
    if (!isConnected()) {
        return coupons;
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM user_coupons WHERE user_id = ? ORDER BY obtain_time DESC");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qWarning() << "获取用户优惠券失败:" << query.lastError().text();
        return coupons;
    }
    
    while (query.next()) {
        QJsonObject coupon;
        coupon["couponId"] = query.value("coupon_id").toInt();
        coupon["userId"] = query.value("user_id").toInt();
        coupon["couponType"] = query.value("coupon_type").toString();
        coupon["couponValue"] = query.value("coupon_value").toDouble();
        coupon["status"] = query.value("status").toString();
        coupon["obtainTime"] = query.value("obtain_time").toString();
        coupon["expireTime"] = query.value("expire_time").toString();
        coupon["useTime"] = query.value("use_time").toString();
        coupon["orderId"] = query.value("order_id").toString();
        
        coupons.append(coupon);
    }
    
    return coupons;
}


// ===== 优惠券相关函数（直接存储在users表中）=====

bool Database::addCoupon30(int userId, int count)
{
    // 将30元优惠券存入user_coupons表
    for (int i = 0; i < count; i++) {
        if (!addUserCoupon(userId, 30.0)) {
            qWarning() << "添加30元优惠券失败，用户ID:" << userId;
            return false;
        }
    }
    qDebug() << "添加30元优惠券成功，用户ID:" << userId << "数量:" << count;
    return true;
}

bool Database::addCoupon50(int userId, int count)
{
    // 将50元优惠券存入user_coupons表
    for (int i = 0; i < count; i++) {
        if (!addUserCoupon(userId, 50.0)) {
            qWarning() << "添加50元优惠券失败，用户ID:" << userId;
            return false;
        }
    }
    qDebug() << "添加50元优惠券成功，用户ID:" << userId << "数量:" << count;
    return true;
}

bool Database::useCoupon30(int userId, int count, const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法使用30元优惠券";
        return false;
    }
    
    // 先检查优惠券数量是否足够（直接查询，避免调用需要锁的函数）
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 30.0 AND status = '未使用'");
    checkQuery.addBindValue(userId);
    int currentCount = 0;
    if (checkQuery.exec() && checkQuery.next()) {
        currentCount = checkQuery.value("count").toInt();
    }
    if (currentCount < count) {
        qWarning() << "30元优惠券数量不足，当前:" << currentCount << "需要:" << count;
        return false;
    }
    
    // 从user_coupons表中选择未使用的30元优惠券并标记为已使用
    QSqlQuery query(m_db);
    QString sql = "UPDATE user_coupons SET status = '已使用', use_time = NOW()";
    if (!orderId.isEmpty()) {
        sql += ", order_id = ?";
    }
    sql += " WHERE user_id = ? AND coupon_value = 30.0 AND status = '未使用' LIMIT ?";
    
    query.prepare(sql);
    if (!orderId.isEmpty()) {
        query.addBindValue(orderId);
    }
    query.addBindValue(userId);
    query.addBindValue(count);
    
    if (!query.exec()) {
        qWarning() << "使用30元优惠券失败:" << query.lastError().text();
        return false;
    }
    
    // 检查是否真的更新了记录
    int affectedRows = query.numRowsAffected();
    if (affectedRows < count) {
        qWarning() << "使用30元优惠券失败：只更新了" << affectedRows << "条记录，需要" << count;
        return false;
    }
    
    qDebug() << "使用30元优惠券成功，用户ID:" << userId << "数量:" << count << "订单ID:" << orderId << "更新记录数:" << affectedRows;
    return true;
}

bool Database::useCoupon50(int userId, int count, const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法使用50元优惠券";
        return false;
    }
    
    // 先检查优惠券数量是否足够（直接查询，避免调用需要锁的函数）
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 50.0 AND status = '未使用'");
    checkQuery.addBindValue(userId);
    int currentCount = 0;
    if (checkQuery.exec() && checkQuery.next()) {
        currentCount = checkQuery.value("count").toInt();
    }
    if (currentCount < count) {
        qWarning() << "50元优惠券数量不足，当前:" << currentCount << "需要:" << count;
        return false;
    }
    
    // 从user_coupons表中选择未使用的50元优惠券并标记为已使用
    QSqlQuery query(m_db);
    QString sql = "UPDATE user_coupons SET status = '已使用', use_time = NOW()";
    if (!orderId.isEmpty()) {
        sql += ", order_id = ?";
    }
    sql += " WHERE user_id = ? AND coupon_value = 50.0 AND status = '未使用' LIMIT ?";
    
    query.prepare(sql);
    if (!orderId.isEmpty()) {
        query.addBindValue(orderId);
    }
    query.addBindValue(userId);
    query.addBindValue(count);
    
    if (!query.exec()) {
        qWarning() << "使用50元优惠券失败:" << query.lastError().text();
        return false;
    }
    
    // 检查是否真的更新了记录
    int affectedRows = query.numRowsAffected();
    if (affectedRows < count) {
        qWarning() << "使用50元优惠券失败：只更新了" << affectedRows << "条记录，需要" << count;
        return false;
    }
    
    qDebug() << "使用50元优惠券成功，用户ID:" << userId << "数量:" << count << "订单ID:" << orderId << "更新记录数:" << affectedRows;
    return true;
}

int Database::getCoupon30Count(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return 0;
    }
    
    // 从user_coupons表查询未使用的30元优惠券数量
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 30.0 AND status = '未使用'");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value("count").toInt();
}

int Database::getCoupon50Count(int userId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        return 0;
    }
    
    // 从user_coupons表查询未使用的50元优惠券数量
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) as count FROM user_coupons WHERE user_id = ? AND coupon_value = 50.0 AND status = '未使用'");
    query.addBindValue(userId);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value("count").toInt();
}

bool Database::rollbackCoupon30(int userId, const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法回滚30元优惠券";
        return false;
    }
    
    // 将已使用的优惠券改回未使用状态
    QSqlQuery query(m_db);
    query.prepare("UPDATE user_coupons SET status = '未使用', use_time = NULL, order_id = NULL "
                  "WHERE user_id = ? AND coupon_value = 30.0 AND status = '已使用' AND order_id = ? LIMIT 1");
    query.addBindValue(userId);
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        qWarning() << "回滚30元优惠券失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "回滚30元优惠券成功，用户ID:" << userId << "订单ID:" << orderId;
    return true;
}

bool Database::rollbackCoupon50(int userId, const QString& orderId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        qWarning() << "数据库未连接，无法回滚50元优惠券";
        return false;
    }
    
    // 将已使用的优惠券改回未使用状态
    QSqlQuery query(m_db);
    query.prepare("UPDATE user_coupons SET status = '未使用', use_time = NULL, order_id = NULL "
                  "WHERE user_id = ? AND coupon_value = 50.0 AND status = '已使用' AND order_id = ? LIMIT 1");
    query.addBindValue(userId);
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        qWarning() << "回滚50元优惠券失败:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "回滚50元优惠券成功，用户ID:" << userId << "订单ID:" << orderId;
    return true;
}

