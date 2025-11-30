#include "websocketserver.h"

// WebSocket服务器构造函数：初始化并连接新连接信号
WebSocketServer::WebSocketServer(const QString& serverName, QWebSocketServer::SslMode secureMode, QObject *parent)
    : QWebSocketServer(serverName, secureMode, parent)
{
    // 当有新客户端连接时，触发onNewConnection槽函数
    connect(this, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

// 新客户端连接处理
void WebSocketServer::onNewConnection()
{
    QWebSocket* socket = nextPendingConnection();  // 获取新连接的WebSocket对象
    // 客户端断开连接时，从列表中移除并清理资源
    connect(socket, &QWebSocket::disconnected, this, &WebSocketServer::onSocketDisconnected);
    m_clients.append(socket);  // 添加到客户端列表

    // 创建实时通信任务并加入线程池
    WebSocketTask* task = new WebSocketTask(socket);
    ThreadPool::getInstance().addTask(task);

    qDebug() << "WebSocket客户端连接：" << socket->peerAddress().toString();
}

// 客户端断开连接处理
void WebSocketServer::onSocketDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());  // 获取断开连接的客户端
    if (socket) {
        m_clients.removeOne(socket);  // 从列表移除
        socket->deleteLater();  // 延迟释放资源
        qDebug() << "WebSocket客户端断开连接";
    }
}

// WebSocket任务构造函数：保存WebSocket连接对象
WebSocketTask::WebSocketTask(QWebSocket* socket, QObject *parent)
    : Task(), m_socket(socket)
{
    Q_UNUSED(parent); // 新增
}

// 任务执行逻辑：处理实时消息
void WebSocketTask::run()
{
    // 【WebSocket实时通信扩展位置】
    // 后续需实现：
    // 1. 通知信息推送（系统通知、订单通知）
    // 2. 客服聊天（点对点/群聊）
    // 3. 心跳检测、消息重发等机制

    // 示例：接收客户端消息并回显
    QObject::connect(m_socket, &QWebSocket::textMessageReceived, m_socket, [=](const QString& message) {
        qDebug() << "收到WebSocket消息：" << message;
        m_socket->sendTextMessage("服务器收到：" + message);  // 回复客户端

        // 广播消息给所有客户端（群聊示例）
        // for (auto client : m_clients) { client->sendTextMessage(message); }
    });

    // 发送欢迎消息
    m_socket->sendTextMessage("欢迎连接WebSocket服务器");
}
