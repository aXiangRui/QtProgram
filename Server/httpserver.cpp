#include "httpserver.h"

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent)
{
}

void HttpServer::incomingConnection(qintptr socketDescriptor)
{
    // 新HTTP请求，创建任务并加入线程池
    HttpTask* task = new HttpTask(socketDescriptor);
    ThreadPool::getInstance().addTask(task);
}

HttpTask::HttpTask(qintptr socketDescriptor, QObject *parent)
    : Task(), m_socketDescriptor(socketDescriptor)
{
    Q_UNUSED(parent); // 新增
}

void HttpTask::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "HTTP套接字初始化失败：" << socket.errorString();
        return;
    }

    // 读取HTTP请求
    QByteArray requestData;
    while (socket.waitForReadyRead(1000)) {
        requestData += socket.readAll();
    }

    qDebug() << "收到HTTP请求：" << requestData;
    handleHttpRequest(QString(requestData), socket);

    socket.disconnectFromHost();
}

void HttpTask::handleHttpRequest(const QString& request, QTcpSocket& socket)
{
    // 解析HTTP请求路径（示例：/book /user /cart /order /collect）
    QString path = request.split(" ").at(1);
    QString response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    // 【HTTP业务扩展位置】
    // 后续需实现各业务接口，结合DatabaseHelper操作数据库
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
        response += "<h1>404 Not Found</h1>";
    }

    // 发送HTTP响应
    socket.write(response.toUtf8());
}
