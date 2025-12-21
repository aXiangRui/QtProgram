-- ==========================================
-- 图书管理系统数据库初始化脚本
-- ==========================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS bookstore CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE bookstore;

-- ==========================================
-- 1. 请求日志表 - 记录所有API请求（核心功能）
-- ==========================================
CREATE TABLE IF NOT EXISTS request_logs (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '日志ID',
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '请求时间',
    client_ip VARCHAR(50) COMMENT '客户端IP',
    client_port INT COMMENT '客户端端口',
    action VARCHAR(100) COMMENT '请求动作',
    request_data TEXT COMMENT '请求数据(JSON)',
    response_data TEXT COMMENT '响应数据(JSON)',
    success BOOLEAN COMMENT '是否成功',
    category VARCHAR(50) COMMENT '请求分类(user/admin/seller/book/order/cart)',
    INDEX idx_timestamp (timestamp),
    INDEX idx_action (action),
    INDEX idx_category (category)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='API请求日志表-记录所有请求';

-- ==========================================
-- 2. 用户表（买家）
-- ==========================================
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='买家用户表';

-- ==========================================
-- 3. 商家表
-- ==========================================
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='商家表';

-- 插入默认商家账号: seller/123456
-- 由于卖家都是由买家认证而成的，所以初始化sellers表的时候，同时也要将同样的用户信息加入users表
INSERT INTO users (username, password, email, phone_number, address, register_date, role) 
VALUES ('seller', '123456', 'seller@example.com', NULL, NULL, CURDATE(), 2)
ON DUPLICATE KEY UPDATE role = 2;

INSERT INTO sellers (seller_name, password, email, phone_number, address, balance, register_date, status) 
VALUES ('seller', '123456', 'seller@example.com', NULL, NULL, 0.00, CURDATE(), '正常')
ON DUPLICATE KEY UPDATE seller_name = seller_name;

-- ==========================================
-- 4. 图书表
-- ==========================================
CREATE TABLE IF NOT EXISTS books (
    isbn VARCHAR(50) PRIMARY KEY COMMENT '图书ISBN',
    title VARCHAR(200) NOT NULL COMMENT '书名',
    author VARCHAR(100) COMMENT '作者',
    category1 VARCHAR(50) COMMENT '一级分类',
    category2 VARCHAR(50) COMMENT '二级分类',
    merchant_id INT COMMENT '商家ID',
    price DECIMAL(10, 2) COMMENT '售价',
    cost DECIMAL(10, 2) COMMENT '成本价',
    stock INT DEFAULT 0 COMMENT '库存',
    status VARCHAR(20) DEFAULT '正常' COMMENT '状态',
    INDEX idx_title (title),
    INDEX idx_category1 (category1),
    INDEX idx_category2 (category2),
    INDEX idx_merchant_id (merchant_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='图书表';

-- ==========================================
-- 5. 订单表
-- ==========================================
CREATE TABLE IF NOT EXISTS orders (
    order_id VARCHAR(50) PRIMARY KEY COMMENT '订单ID',
    user_id INT COMMENT '用户ID',
    customer VARCHAR(100) COMMENT '客户姓名',
    phone VARCHAR(20) COMMENT '联系电话',
    total_amount DECIMAL(10, 2) COMMENT '订单总额',
    status VARCHAR(20) DEFAULT '待支付' COMMENT '订单状态',
    payment_method VARCHAR(50) COMMENT '支付方式',
    order_date DATETIME COMMENT '下单时间',
    pay_time DATETIME COMMENT '支付时间',
    ship_time DATETIME COMMENT '发货时间',
    cancel_time DATETIME COMMENT '取消时间',
    cancel_reason VARCHAR(200) COMMENT '取消原因',
    tracking_number VARCHAR(100) COMMENT '物流单号',
    address VARCHAR(300) COMMENT '收货地址',
    operator VARCHAR(50) COMMENT '操作员',
    remark TEXT COMMENT '备注',
    items TEXT COMMENT '订单项(JSON)',
    INDEX idx_user_id (user_id),
    INDEX idx_order_date (order_date),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='订单表';

-- ==========================================
-- 6. 购物车表
-- ==========================================
CREATE TABLE IF NOT EXISTS cart (
    cart_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '购物车ID',
    user_id INT NOT NULL COMMENT '用户ID',
    book_id VARCHAR(50) NOT NULL COMMENT '图书ID',
    quantity INT DEFAULT 1 COMMENT '数量',
    add_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
    INDEX idx_user_id (user_id),
    UNIQUE KEY unique_user_book (user_id, book_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='购物车表';

-- ==========================================
-- 7. 会员表
-- ==========================================
CREATE TABLE IF NOT EXISTS members (
    card_no VARCHAR(50) PRIMARY KEY COMMENT '会员卡号',
    name VARCHAR(100) COMMENT '会员姓名',
    phone VARCHAR(20) COMMENT '联系电话',
    level VARCHAR(20) DEFAULT '普通' COMMENT '会员等级',
    balance DECIMAL(10, 2) DEFAULT 0.00 COMMENT '余额',
    points INT DEFAULT 0 COMMENT '积分',
    create_date DATE COMMENT '创建日期',
    INDEX idx_phone (phone)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='会员表';

-- ==========================================
-- 初始化完成
-- ==========================================
SELECT '========================================' AS '';
SELECT '数据库初始化完成！' AS '状态';
SELECT '默认商家账号: seller / 123456' AS '提示';
SELECT '========================================' AS '';

-- 查看所有表
SHOW TABLES;
