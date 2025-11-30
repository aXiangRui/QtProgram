#include "websocketclient.h"
#include <QUrl>
#include <QDebug>

// WebSocket客户端构造函数：初始化WebSocket并连接信号槽
WebSocketClient::WebSocketClient(QObject *parent) : QObject(parent)
{
    m_socket = new QWebSocket();
    // 连接状态信号槽
    connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    // 消息接收信号槽
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
     // 错误信号槽
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocketClient::onError);
}

// 连接到WebSocket服务器
void WebSocketClient::connectToServer(const QString &ip, int port)
{
    // 若已连接则先断开
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->close();
    }
    // 构建WebSocket连接地址（ws://ip:port）
    QUrl url = QUrl(QString("ws://%1:%2").arg(ip).arg(port));
    m_socket->open(url);
}

// 向服务器发送消息
void WebSocketClient::sendMessage(const QString &message)
{
    // 检查是否已连接
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);  // 发送文本消息
        emit logMessage("发送WebSocket消息：" + message);
    } else {
        emit logMessage("未连接到WebSocket服务器，无法发送消息");
    }
}

// 连接成功回调
void WebSocketClient::onConnected()
{
    emit logMessage("WebSocket服务器连接成功");
}

// 断开连接回调
void WebSocketClient::onDisconnected()
{
    emit logMessage("WebSocket服务器断开连接");
}

// 收到消息回调
void WebSocketClient::onTextMessageReceived(const QString &message)
{
    emit messageReceived(message);
    emit logMessage("收到WebSocket消息：" + message);
}

// 错误发生回调
void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);  // 标记参数未使用，消除警告
    emit logMessage("WebSocket错误：" + m_socket->errorString());
}
