#include "../include/TcpConnection.h"

TcpConnection::TcpConnection(QObject *parent)
    : NetworkConnection(parent), m_socket(nullptr)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpConnection::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpConnection::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpConnection::onError);
}

TcpConnection::~TcpConnection()
{
    if (m_socket && m_socket->isOpen()) {
        m_socket->close();
    }
    delete m_socket;
}

bool TcpConnection::connectToServer(const QString& url, int port)
{
    m_serverUrl = url;
    m_serverPort = port;
    
    m_socket->connectToHost(url, port);
    return m_socket->waitForConnected(5000);
}

bool TcpConnection::sendData(const QByteArray& data)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return false;
    
    m_socket->write(data);
    return m_socket->waitForBytesWritten();
}

void TcpConnection::disconnect()
{
    if (m_socket && m_socket->isOpen()) {
        m_socket->close();
    }
}

QJsonObject TcpConnection::sendRequest(const QJsonObject& request)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = "Not connected to server";
        return response;
    }

    // 发送请求
    QByteArray data = QJsonDocument(request).toJson();
    m_socket->write(data);
    m_socket->waitForBytesWritten();

    // 等待响应
    m_receiveBuffer.clear();
    if (!m_socket->waitForReadyRead(5000)) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = "Timeout waiting for response";
        return response;
    }

    // 读取响应
    m_receiveBuffer = m_socket->readAll();
    while (m_socket->waitForReadyRead(100)) {
        m_receiveBuffer.append(m_socket->readAll());
    }

    // 解析响应
    QJsonDocument doc = QJsonDocument::fromJson(m_receiveBuffer);
    if (doc.isNull()) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = "Invalid response format";
        return response;
    }

    return doc.object();
}

void TcpConnection::onConnected()
{
    m_isConnected = true;
    emit connected();
}

void TcpConnection::onDisconnected()
{
    m_isConnected = false;
    emit disconnected();
}

void TcpConnection::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());
    emit dataReceived(m_receiveBuffer);
}

void TcpConnection::onError(QAbstractSocket::SocketError error)
{
    emit connectionError(m_socket->errorString());
}