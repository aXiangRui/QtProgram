#include "httpserver.h"

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent)
{
    // 构造函数：初始化HTTP服务器，暂无额外逻辑
}

// 新客户端连接处理：创建HTTP任务并提交到线程池
void HttpServer::incomingConnection(qintptr socketDescriptor)
{
    HttpTask* task = new HttpTask(socketDescriptor);
    ThreadPool::getInstance().addTask(task);  // 线程池异步处理请求
}

// HTTP任务构造函数：保存客户端套接字描述符
HttpTask::HttpTask(qintptr socketDescriptor, QObject *parent)
    : Task(), m_socketDescriptor(socketDescriptor)
{
    Q_UNUSED(parent);  // 标记未使用的参数
}

// 任务执行逻辑：读取HTTP请求并处理
void HttpTask::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "HTTP套接字初始化失败：" << socket.errorString();
        return;
    }

    // 读取HTTP请求数据（超时1秒）
    QByteArray requestData;
    while (socket.waitForReadyRead(1000)) {
        requestData += socket.readAll();
    }

    qDebug() << "收到HTTP请求：" << requestData;
    handleHttpRequest(QString(requestData), socket);  // 处理请求

    socket.disconnectFromHost();  // 处理完成后断开连接
}

// 解析并处理HTTP请求
void HttpTask::handleHttpRequest(const QString& request, QTcpSocket& socket)
{
    // 解析HTTP请求路径（例如：GET /book HTTP/1.1 中的路径为/book）
    QString path = request.split(" ").at(1);
    // 构建HTTP响应头（200 OK表示成功）
    QString response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    // 【HTTP业务扩展位置】
    // 根据请求路径分发不同业务（后续需对接数据库实现具体逻辑）
    if (path == "/book") {
        // 图书浏览业务：查询数据库返回图书列表
        response += "<h1>图书浏览接口（后续实现）</h1>";
    } else if (path == "/user") {
        // 用户管理业务：注册/登录/信息修改
        response += "<h1>用户管理接口（后续实现）</h1>";
    } else if (path == "/cart") {
        // 购物车业务：添加/删除/查询购物车
        response += "<h1>购物车接口（后续实现）</h1>";
    } else if (path == "/order") {
        // 订单业务：创建/支付/查询订单
        response += "<h1>订单接口（后续实现）</h1>";
    } else if (path == "/collect") {
        // 收藏业务：添加/取消/查询收藏
        response += "<h1>收藏接口（后续实现）</h1>";
    } else {
        // 未知路径返回404
        response += "<h1>404 Not Found</h1>";
    }

    // 发送HTTP响应给客户端
    socket.write(response.toUtf8());
}
