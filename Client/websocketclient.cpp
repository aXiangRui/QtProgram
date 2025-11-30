#include "websocketclient.h"
#include <QUrl>
#include <QDebug>

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
    QUrl url = QUrl(QString("ws://%1:%2").arg(ip).arg(port));
    m_socket->open(url);
}

void WebSocketClient::sendMessage(const QString &message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);
        emit logMessage("发送WebSocket消息：" + message);
    } else {
        emit logMessage("未连接到WebSocket服务器，无法发送消息");
    }
}

void WebSocketClient::onConnected()
{
    emit logMessage("WebSocket服务器连接成功");
}

void WebSocketClient::onDisconnected()
{
    emit logMessage("WebSocket服务器断开连接");
}

void WebSocketClient::onTextMessageReceived(const QString &message)
{
    emit messageReceived(message);
    emit logMessage("收到WebSocket消息：" + message);
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);  // 标记参数未使用，消除警告
    emit logMessage("WebSocket错误：" + m_socket->errorString());
}
