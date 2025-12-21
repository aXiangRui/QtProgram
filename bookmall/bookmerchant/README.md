# BookMerchant - 图书商家管理系统

## 项目概述
BookMerchant是一个基于Qt开发的图书商家管理系统客户端，通过TCP协议与服务器进行通信，实现图书管理、订单管理、会员管理等功能。

## 功能特性

### 1. 用户认证
- 商家登录功能
- 服务器连接管理

### 2. 图书管理
- 查看图书列表
- 添加新图书
- 编辑图书信息
- 删除图书
- 字段包括：ISBN、书名、作者、分类、价格、库存、状态等

### 3. 订单管理
- 查看所有订单
- 更新订单状态（待支付、已支付、已发货、已完成、已取消）
- 删除订单
- 查看订单详情

### 4. 会员管理
- 查看会员列表
- 添加新会员
- 编辑会员信息
- 删除会员
- 会员充值功能

### 5. 统计报表
- 仪表盘统计（总销售额、订单数、会员数、图书种类）
- 销售报表（按日期范围）
- 库存报表
- 会员报表

## 技术架构

### 通信协议
- **传输协议**: TCP
- **数据格式**: JSON
- **帧格式**: 长度前缀协议（4字节大端长度 + JSON payload）

### API接口

所有请求都使用以下格式：
```json
{
  "action": "操作名称",
  "参数1": "值1",
  "参数2": "值2"
}
```

#### 图书管理API
- `sellerGetBooks` - 获取图书列表
- `sellerAddBook` - 添加图书
- `sellerUpdateBook` - 更新图书
- `sellerDeleteBook` - 删除图书

#### 订单管理API
- `sellerGetOrders` - 获取订单列表
- `sellerCreateOrder` - 创建订单
- `sellerUpdateOrderStatus` - 更新订单状态
- `sellerDeleteOrder` - 删除订单

#### 会员管理API
- `sellerGetMembers` - 获取会员列表
- `sellerAddMember` - 添加会员
- `sellerUpdateMember` - 更新会员信息
- `sellerDeleteMember` - 删除会员
- `sellerRechargeMember` - 会员充值

#### 统计报表API
- `sellerDashboardStats` - 获取仪表盘统计
- `sellerGetReportSales` - 获取销售报表
- `sellerGetReportInventory` - 获取库存报表
- `sellerGetReportMember` - 获取会员报表

## 项目结构

```
bookmerchant/
├── main.cpp                 # 程序入口
├── bookmerchant.h/cpp       # 主窗口类
├── apiservice.h/cpp         # API服务层
├── tcpclient.h/cpp          # TCP客户端
├── bookmerchant.pro         # Qt项目配置
└── README.md                # 本文档
```

## 编译和运行

### 环境要求
- Qt 5.x 或更高版本
- C++11支持
- Qt模块：core, gui, widgets, network

### 编译步骤
1. 使用Qt Creator打开 `bookmerchant.pro`
2. 选择合适的构建套件
3. 点击"构建" -> "构建项目"
4. 运行程序

### 命令行编译
```bash
qmake bookmerchant.pro
make
```

## 使用说明

### 1. 连接服务器
- 启动程序后，在登录页面输入服务器地址和端口（默认：localhost:8888）
- 点击"连接服务器"按钮

### 2. 登录
- 输入商家用户名和密码
- 点击"登录"按钮

### 3. 功能导航
登录成功后，可以通过主页面的按钮访问各个功能模块：
- 图书管理
- 订单管理
- 会员管理
- 统计报表

## 与服务器的交互

### 服务器要求
服务器需要实现以下功能：
1. TCP监听（默认端口8888）
2. 支持长度前缀协议的JSON消息解析
3. 实现所有商家端API接口

### 数据同步
- 客户端通过TCP与服务器实时通信
- 所有数据操作都会发送到服务器并等待响应
- 超时时间：10秒

## 与Purchaser1的集成

BookMerchant和Purchaser1都是通过同一个服务器进行数据交互：
- Purchaser1：买家客户端，负责浏览图书、购物、下单
- BookMerchant：商家客户端，负责管理图书、订单、会员
- Server：中央服务器，处理所有客户端请求

### 交互流程示例
1. 商家在BookMerchant中添加图书 → 服务器更新图书数据库
2. 买家在Purchaser1中浏览图书 → 从服务器获取图书列表
3. 买家下单 → 服务器创建订单
4. 商家在BookMerchant中查看订单 → 从服务器获取订单列表
5. 商家更新订单状态 → 服务器更新订单状态

## 注意事项

1. **网络连接**：确保服务器已启动并监听正确的端口
2. **数据格式**：服务器返回的JSON必须包含`success`字段表示操作是否成功
3. **错误处理**：所有网络错误和业务错误都会通过消息框提示用户
4. **并发访问**：服务器端需要处理多客户端并发访问的线程安全问题

## 常见问题

### Q: 连接服务器失败
A: 检查服务器是否启动，防火墙是否阻止，地址和端口是否正确

### Q: 登录失败
A: 确认服务器端实现了login接口，用户名密码是否正确

### Q: 操作超时
A: 检查网络连接，服务器是否正常响应，可以增加超时时间

## 开发者信息

本项目遵循Purchaser1的代码风格和架构设计，使用TCP协议实现客户端-服务器通信。

