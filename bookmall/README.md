# 图书商城系统 (Book Mall System)

一个基于 Qt 框架和 C++ 开发的分布式图书商城系统，采用客户端-服务器（C/S）架构，支持买家、卖家和管理员三种角色。

## 📋 项目概述

本项目是一个完整的图书商城系统，包含四个独立的子项目：

- **purchaser1** - 买家客户端
- **bookmerchant** - 卖家客户端  
- **bookadmin** - 管理员客户端
- **server** - 服务器端

系统支持用户注册登录、图书浏览购买、订单管理、商家管理、数据统计等完整功能。

## 🏗️ 系统架构

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Purchaser │     │ BookMerchant│     │  BookAdmin  │
│ (买家客户端) │     │ (卖家客户端) │     │(管理员客户端)│
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                    │                    │
       │      TCP Socket (JSON)                 │
       │                    │                    │
       └────────────────────┼────────────────────┘
                            │
                    ┌───────▼────────┐
                    │   TCP Server   │
                    │   (服务器端)    │
                    └───────┬────────┘
                            │
                    ┌───────▼────────┐
                    │  MySQL Database│
                    │   (数据库)      │
                    └────────────────┘
```

## 🛠️ 技术栈

- **开发框架**: Qt 5.9
- **编程语言**: C++11
- **数据库**: MySQL 5.1.51
- **通信协议**: TCP Socket
- **数据格式**: JSON
- **开发工具**: Qt Creator
- **操作系统**: Windows/Linux

## 📦 项目结构

```
bookmall/
├── purchaser1/          # 买家客户端
│   ├── purchaser.cpp    # 主窗口实现
│   ├── purchaser.h      # 主窗口头文件
│   ├── apiservice.cpp   # API服务实现
│   ├── apiservice.h     # API服务头文件
│   ├── tcpclient.cpp   # TCP客户端实现
│   ├── tcpclient.h     # TCP客户端头文件
│   ├── book.cpp        # 图书数据模型
│   ├── book.h          # 图书数据模型头文件
│   ├── user.cpp        # 用户数据模型
│   ├── user.h          # 用户数据模型头文件
│   ├── purchaser1.pro  # 项目配置文件
│   └── main.cpp        # 程序入口
│
├── bookmerchant/        # 卖家客户端
│   ├── bookmerchant.cpp # 主窗口实现
│   ├── bookmerchant.h  # 主窗口头文件
│   ├── apiservice.cpp  # API服务实现
│   ├── apiservice.h    # API服务头文件
│   ├── tcpclient.cpp   # TCP客户端实现
│   ├── tcpclient.h     # TCP客户端头文件
│   ├── bookmerchant.pro # 项目配置文件
│   └── main.cpp        # 程序入口
│
├── bookadmin/           # 管理员客户端
│   ├── bookadmin.cpp   # 主窗口实现
│   ├── bookadmin.h     # 主窗口头文件
│   ├── apiservice.cpp  # API服务实现
│   ├── apiservice.h    # API服务头文件
│   ├── tcpclient.cpp   # TCP客户端实现
│   ├── tcpclient.h     # TCP客户端头文件
│   ├── bookadmin.pro   # 项目配置文件
│   └── main.cpp        # 程序入口
│
└── server/              # 服务器端
    ├── tcpserver.cpp   # TCP服务器实现
    ├── tcpserver.h     # TCP服务器头文件
    ├── data.cpp        # 数据库操作实现
    ├── data.h          # 数据库操作头文件
    ├── serverwindow.cpp # 服务器窗口实现
    ├── serverwindow.h  # 服务器窗口头文件
    ├── serverwindow.ui # 服务器窗口UI文件
    ├── threadpool.cpp  # 线程池实现
    ├── threadpool.h    # 线程池头文件
    ├── clientthreadfactory.cpp # 客户端线程工厂
    ├── clientthreadfactory.h   # 客户端线程工厂头文件
    ├── Server.pro      # 项目配置文件
    ├── init_database.sql # 数据库初始化脚本
    └── main.cpp        # 程序入口
```

## 🚀 快速开始

### 环境要求

1. **Qt 开发环境**
   - Qt 5.12+ 或 Qt 6.x
   - Qt Creator IDE
   - 确保已安装以下 Qt 模块：
     - `Qt Core`
     - `Qt GUI`
     - `Qt Widgets`
     - `Qt Network`
     - `Qt SQL` (仅服务器端需要)

2. **数据库**
   - MySQL 5.7+ 
   - MySQL 客户端工具（可选，用于管理数据库）

3. **编译器**
   - Windows: MinGW 或 MSVC
   - Linux: GCC 4.8+

### 安装步骤

#### 1. 克隆或下载项目

```bash
# 如果使用 Git
git clone <repository-url>
cd bookmall
```

#### 2. 配置数据库

**步骤 1**: 安装并启动 MySQL 服务

**步骤 2**: 创建数据库

```bash
# 方式1: 使用命令行
mysql -u root -p < server/init_database.sql

# 方式2: 手动创建
mysql -u root -p
CREATE DATABASE bookstore CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE bookstore;
SOURCE server/init_database.sql;
```

**步骤 3**: 配置数据库连接

编辑 `server/main.cpp`，找到数据库初始化代码：

```cpp
Database::getInstance().initConnection(
    "localhost",    // 数据库主机
    3306,           // 数据库端口
    "bookstore",    // 数据库名称
    "root",         // 用户名
    ""              // 密码（修改为你的MySQL密码）
);
```

#### 3. 编译项目

**使用 Qt Creator:**

1. 打开 Qt Creator
2. 打开项目文件：
   - `server/Server.pro` - 先编译服务器
   - `purchaser1/purchaser1.pro` - 买家客户端
   - `bookmerchant/bookmerchant.pro` - 卖家客户端
   - `bookadmin/bookadmin.pro` - 管理员客户端
3. 选择构建套件（Kit）
4. 点击"构建"按钮编译项目

**使用命令行:**

```bash
# 编译服务器
cd server
qmake Server.pro
make  # Windows: mingw32-make 或 nmake

# 编译买家客户端
cd ../purchaser1
qmake purchaser1.pro
make

# 编译卖家客户端
cd ../bookmerchant
qmake bookmerchant.pro
make

# 编译管理员客户端
cd ../bookadmin
qmake bookadmin.pro
make
```

#### 4. 运行项目

**运行顺序:**

1. **首先启动服务器** (`server/Server.exe` 或 `server/Server`)
   - 确认服务器监听端口 8888
   - 查看控制台输出，确认数据库连接成功

2. **启动客户端**
   - 买家客户端: `purchaser1/purchaser.exe`
   - 卖家客户端: `bookmerchant/bookmerchant.exe`
   - 管理员客户端: `bookadmin/bookadmin.exe`

**默认服务器配置:**

- 服务器地址: `127.0.0.1` (本地)
- 服务器端口: `8888`

如需修改服务器地址，请编辑各客户端的源码：
- `purchaser1/purchaser.cpp` (约第28-29行)
- `bookmerchant/bookmerchant.cpp` (约第39-40行)
- `bookadmin/bookadmin.cpp` (相应位置)

## 👥 用户角色与功能

### 1. 买家客户端 (Purchaser)

**主要功能:**
- ✅ 用户注册与登录
- ✅ 图书浏览与搜索
- ✅ 图书分类筛选
- ✅ 购物车管理
- ✅ 订单创建与支付
- ✅ 订单查询与管理
- ✅ 确认收货功能
- ✅ 个人信息管理
- ✅ 账户余额管理

**默认测试账号:**
- 需要注册新账号

### 2. 卖家客户端 (BookMerchant)

**主要功能:**
- ✅ 商家登录
- ✅ 图书管理（增删改查）
- ✅ 图书库存管理
- ✅ 订单管理（查看、发货、完成）
- ✅ 订单状态统计
- ✅ 销售数据统计
- ✅ 近7天销售额折线图
- ✅ 会员管理
- ✅ 客服聊天功能
- ✅ 数据仪表板（实时更新）

**默认测试账号:**
- 用户名: `seller`
- 密码: `123456`

### 3. 管理员客户端 (BookAdmin)

**主要功能:**
- ✅ 管理员登录
- ✅ 用户管理（查看、禁用、启用）
- ✅ 商家管理
- ✅ 图书管理
- ✅ 订单管理
- ✅ 系统统计
- ✅ 请求日志查看
- ✅ 数据报表

**默认测试账号:**
- 用户名: `admin`
- 密码: `admin123`

### 4. 服务器端 (Server)

**主要功能:**
- ✅ TCP 服务器监听
- ✅ 多线程客户端处理
- ✅ 数据库连接管理
- ✅ 请求路由与处理
- ✅ 请求日志记录
- ✅ 数据持久化
- ✅ 服务器状态监控

## 📊 数据库设计

### 核心数据表

1. **users** - 买家用户表
   - 存储买家用户信息
   - 支持用户注册、登录、信息管理

2. **sellers** - 商家表
   - 存储商家信息
   - 默认账号: seller/123456

3. **books** - 图书表
   - 存储图书信息（ISBN、书名、作者、价格、库存等）
   - 支持分类管理

4. **orders** - 订单表
   - 存储订单信息
   - 订单状态: 待支付、已支付、已发货、已完成

5. **cart** - 购物车表
   - 存储用户购物车信息

6. **request_logs** - 请求日志表 ⭐
   - 自动记录所有客户端请求
   - 包含请求时间、客户端信息、请求内容、响应结果等

### 数据库初始化

运行 `server/init_database.sql` 脚本即可创建所有表结构和初始数据。

## 🔧 配置说明

### 服务器配置

**数据库配置** (`server/main.cpp`):
```cpp
Database::getInstance().initConnection(
    "localhost",    // 数据库主机
    3306,           // 端口
    "bookstore",    // 数据库名
    "root",         // 用户名
    "your_password" // 密码
);
```

**TCP 端口配置** (`server/serverwindow.h`):
```cpp
#define TCP_PORT 8888  // 默认端口
```

### 客户端配置

**服务器地址配置** (各客户端主文件):
```cpp
serverIp("127.0.0.1")  // 服务器IP
serverPort(8888)       // 服务器端口
```

## 📡 通信协议

### 数据格式

所有通信使用 JSON 格式：

**请求格式:**
```json
{
    "action": "login",
    "username": "user123",
    "password": "password123"
}
```

**响应格式:**
```json
{
    "success": true,
    "message": "登录成功",
    "data": { ... }
}
```

### 主要 API 接口

#### 用户相关
- `login` - 用户登录
- `register` - 用户注册
- `getUserInfo` - 获取用户信息
- `updateUserInfo` - 更新用户信息

#### 图书相关
- `getAllBooks` - 获取所有图书
- `searchBooks` - 搜索图书
- `getBookByISBN` - 根据ISBN获取图书

#### 订单相关
- `createOrder` - 创建订单
- `payOrder` - 支付订单
- `getUserOrders` - 获取用户订单
- `confirmReceiveOrder` - 确认收货
- `cancelOrder` - 取消订单

#### 购物车相关
- `addToCart` - 添加到购物车
- `getCart` - 获取购物车
- `removeFromCart` - 从购物车移除
- `updateCartItem` - 更新购物车项

#### 商家相关
- `sellerLogin` - 商家登录
- `sellerGetBooks` - 获取商家图书
- `sellerAddBook` - 添加图书
- `sellerUpdateBook` - 更新图书
- `sellerDeleteBook` - 删除图书
- `sellerGetOrders` - 获取商家订单
- `sellerUpdateOrderStatus` - 更新订单状态
- `getSellerDashboardStats` - 获取商家仪表板统计
- `getSellerSalesReport` - 获取销售报表

#### 管理员相关
- `adminLogin` - 管理员登录
- `adminGetUsers` - 获取用户列表
- `adminGetSellers` - 获取商家列表
- `adminGetOrders` - 获取订单列表
- `adminGetRequestLogs` - 获取请求日志

## 🎨 界面特性

- **统一的设计风格**: 所有客户端采用统一的 UI 风格
- **现代化界面**: 卡片式布局、渐变背景、圆角按钮
- **响应式设计**: 适配不同窗口大小
- **数据可视化**: 折线图展示销售趋势
- **实时更新**: 仪表板数据自动刷新

## 🔍 调试与日志

### 服务器日志

服务器控制台会输出：
- 客户端连接信息
- 请求处理日志
- 数据库操作日志
- 错误信息

### 请求日志

所有客户端请求自动记录到 `request_logs` 表，可通过以下方式查看：

**SQL 查询:**
```sql
-- 查看最近100条请求
SELECT * FROM request_logs ORDER BY timestamp DESC LIMIT 100;

-- 按分类查看
SELECT * FROM request_logs WHERE category = 'user' ORDER BY timestamp DESC;

-- 查看失败的请求
SELECT * FROM request_logs WHERE success = 0 ORDER BY timestamp DESC;
```

**API 查询** (管理员客户端):
```json
{
    "action": "adminGetRequestLogs",
    "limit": 100,
    "category": "order"
}
```

## ⚠️ 常见问题

### 1. 数据库连接失败

**问题**: 服务器启动时提示数据库连接失败

**解决方案**:
- 检查 MySQL 服务是否启动
- 确认数据库名称、用户名、密码正确
- 检查防火墙设置
- 确认数据库已创建

**注意**: 如果数据库连接失败，服务器会自动切换到内存模式，功能正常但数据不会持久化。

### 2. 客户端无法连接服务器

**问题**: 客户端提示连接失败

**解决方案**:
- 确认服务器已启动
- 检查服务器地址和端口配置
- 检查防火墙是否阻止 8888 端口
- 尝试 ping 服务器地址

### 3. 编译错误

**问题**: Qt 模块未找到

**解决方案**:
- 确认 Qt 已正确安装
- 检查 `.pro` 文件中的模块配置
- 在 Qt Creator 中选择正确的构建套件（Kit）

### 4. 中文乱码

**问题**: 界面或数据库中文显示乱码

**解决方案**:
- 确保数据库使用 `utf8mb4` 字符集
- 检查源代码文件编码为 UTF-8
- 确认 Qt 应用程序使用 UTF-8 编码

## 📝 开发说明

### 代码结构

- **客户端**: 采用 MVC 模式，分离 UI、业务逻辑和数据模型
- **服务器**: 采用多线程架构，每个客户端连接使用独立线程处理
- **数据库**: 使用单例模式的 Database 类，确保线程安全

### 扩展开发

1. **添加新的 API 接口**:
   - 在客户端 `apiservice.cpp` 中添加方法
   - 在服务器 `tcpserver.cpp` 中添加处理函数
   - 在 `processJsonRequest` 中注册路由

2. **添加新的数据表**:
   - 在 `init_database.sql` 中添加表结构
   - 在 `data.cpp` 中添加相应的操作方法

3. **修改 UI 样式**:
   - 编辑各客户端的 `applyStyle()` 函数
   - 使用 QSS (Qt Style Sheets) 进行样式定制

## 📄 许可证

本项目为实训项目，仅供学习和研究使用。

## 👨‍💻 开发团队

本项目为小组合作实训项目，基于 Qt 框架和 C++ 开发。

## 📚 相关文档

- `server/数据库使用说明.txt` - 数据库详细使用说明
- `server/功能总结.txt` - 功能实现总结
- `purchaser1/服务器地址配置说明.txt` - 服务器配置说明
- `server/init_database.sql` - 数据库初始化脚本

## 🔄 更新日志

### 最新版本

- ✅ 实现买家确认收货功能
- ✅ 完善商家仪表板统计功能
- ✅ 实现近7天销售额折线图
- ✅ 统一所有客户端 UI 风格
- ✅ 优化订单状态管理
- ✅ 实现实时数据更新

### 历史版本

- 2024-12-14: 完成 MySQL 数据库集成
- 2024-12-14: 实现请求日志记录系统
- 2024-12-14: 添加数据分类功能

## 🎯 未来计划

- [ ] 添加商品图片上传功能
- [ ] 实现支付系统集成
- [ ] 添加商品评价功能
- [ ] 实现消息推送系统
- [ ] 添加数据导出功能
- [ ] 优化数据库查询性能
- [ ] 实现负载均衡

## 📞 技术支持

如有问题，请检查：
1. 服务器控制台输出（有详细的日志信息）
2. MySQL 错误日志
3. Qt Creator 输出窗口
4. 数据库请求日志表

---

**注意**: 本项目为教学实训项目，生产环境使用前请进行充分的安全测试和性能优化。
