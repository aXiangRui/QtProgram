#include "websocketclient.h"
#include <QUrl>
#include <QDebug>

<<<<<<< HEAD
WebSocketClient::WebSocketClient(QObject *parent) : QObject(parent)
{
    m_socket = new QWebSocket();
    connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocketClient::onError);
}

void WebSocketClient::connectToServer(const QString &ip, int port)
{
=======
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
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
    QUrl url = QUrl(QString("ws://%1:%2").arg(ip).arg(port));
    m_socket->open(url);
}

<<<<<<< HEAD
void WebSocketClient::sendMessage(const QString &message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);
=======
// 向服务器发送消息
void WebSocketClient::sendMessage(const QString &message)
{
    // 检查是否已连接
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);  // 发送文本消息
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
        emit logMessage("发送WebSocket消息：" + message);
    } else {
        emit logMessage("未连接到WebSocket服务器，无法发送消息");
    }
}

<<<<<<< HEAD
=======
// 连接成功回调
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void WebSocketClient::onConnected()
{
    emit logMessage("WebSocket服务器连接成功");
}

<<<<<<< HEAD
=======
// 断开连接回调
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void WebSocketClient::onDisconnected()
{
    emit logMessage("WebSocket服务器断开连接");
}

<<<<<<< HEAD
=======
// 收到消息回调
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void WebSocketClient::onTextMessageReceived(const QString &message)
{
    emit messageReceived(message);
    emit logMessage("收到WebSocket消息：" + message);
}

<<<<<<< HEAD
=======
// 错误发生回调
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);  // 标记参数未使用，消除警告
    emit logMessage("WebSocket错误：" + m_socket->errorString());
}
