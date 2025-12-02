#include "../include/WebSocketConnection.h"
#include <QThread>
#include <QCoreApplication>

WebSocketConnection::WebSocketConnection(QObject *parent)
    : NetworkConnection(parent), m_socket(nullptr), m_responseReceived(false)
{
    m_socket = new QWebSocket();
    connect(m_socket, &QWebSocket::connected, this, &WebSocketConnection::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketConnection::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketConnection::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketConnection::onBinaryMessageReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WebSocketConnection::onError);
}

WebSocketConnection::~WebSocketConnection()
{
    if (m_socket && m_socket->isValid()) {
        m_socket->close();
    }
    delete m_socket;
}

bool WebSocketConnection::connectToServer(const QString& url, int port)
{
    m_serverUrl = url;
    m_serverPort = port;
    
    QString serverUrl = QString("ws://%1:%2").arg(url).arg(port);
    m_socket->open(QUrl(serverUrl));
    
    return true; // WebSocket是异步连接，这里只返回是否开始连接
}

bool WebSocketConnection::sendData(const QByteArray& data)
{
    if (m_socket && m_socket->isValid()) {
        m_socket->sendBinaryMessage(data);
        return true;
    }
    return false;
}

void WebSocketConnection::disconnect()
{
    if (m_socket && m_socket->isValid()) {
        m_socket->close();
    }
}

QJsonObject WebSocketConnection::sendRequest(const QJsonObject& request)
{
    if (!m_socket || !m_socket->isValid()) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = "Not connected to server";
        return response;
    }

    // 发送请求
    QByteArray data = QJsonDocument(request).toJson();
    m_socket->sendBinaryMessage(data);

    // 等待响应
    m_responseReceived = false;
    m_lastResponse = QJsonObject();

    // 等待响应，最多等待5秒
    int timeout = 5000;
    int interval = 100;
    int elapsed = 0;

    while (!m_responseReceived && elapsed < timeout) {
        QThread::msleep(interval);
        elapsed += interval;
        QCoreApplication::processEvents();
    }

    if (!m_responseReceived) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = "Timeout waiting for response";
        return response;
    }

    return m_lastResponse;
}

void WebSocketConnection::onConnected()
{
    m_isConnected = true;
    emit connected();
}

void WebSocketConnection::onDisconnected()
{
    m_isConnected = false;
    emit disconnected();
}

void WebSocketConnection::onTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull()) {
        m_lastResponse = doc.object();
        m_responseReceived = true;
    }
    emit dataReceived(message.toUtf8());
}

void WebSocketConnection::onBinaryMessageReceived(const QByteArray& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    if (!doc.isNull()) {
        m_lastResponse = doc.object();
        m_responseReceived = true;
    }
    emit dataReceived(message);
}

void WebSocketConnection::onError(QAbstractSocket::SocketError error)
{
    emit connectionError(m_socket->errorString());
}