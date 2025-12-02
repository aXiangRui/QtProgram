#include "../include/NetworkController.h"
#include <QDebug>

NetworkController::NetworkController(QObject *parent)
    : QObject(parent), m_connection(nullptr), m_isConnected(false),
      m_connectionType(NetworkConnectionFactory::HTTP), m_reconnectTimer(nullptr),
      m_reconnectInterval(5000), m_maxReconnectAttempts(5), m_reconnectAttempts(0)
{
    // 创建重连定时器
    m_reconnectTimer = new QTimer(this);
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkController::onReconnectTimer);
    
    // 默认服务器地址和端口
    m_serverAddress = "localhost";
    m_serverPort = 8080;
    
    // 创建默认连接
    createConnection();
}

NetworkController::~NetworkController()
{
    if (m_connection) {
        m_connection->disconnect();
        delete m_connection;
    }
    
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
        delete m_reconnectTimer;
    }
}

void NetworkController::setServerAddress(const QString& address, int port)
{
    m_serverAddress = address;
    m_serverPort = port;
    
    // 如果已连接，重新连接
    if (m_isConnected) {
        disconnectFromServer();
        connectToServer();
    }
}

void NetworkController::setConnectionType(NetworkConnectionFactory::ConnectionType type)
{
    if (m_connectionType == type) {
        return;
    }
    
    m_connectionType = type;
    
    // 重新创建连接
    if (m_connection) {
        m_connection->disconnect();
        delete m_connection;
    }
    
    createConnection();
    
    // 如果之前已连接，重新连接
    if (m_isConnected) {
        connectToServer();
    }
}

bool NetworkController::connectToServer()
{
    if (!m_connection) {
        createConnection();
    }
    
    bool result = m_connection->connectToServer(m_serverAddress, m_serverPort);
    
    // 重置重连计数器
    m_reconnectAttempts = 0;
    
    return result;
}

void NetworkController::disconnectFromServer()
{
    if (m_connection) {
        m_connection->disconnect();
    }
    
    m_isConnected = false;
    m_reconnectTimer->stop();
}

QJsonObject NetworkController::sendRequest(const QJsonObject& request)
{
    if (!m_connection || !m_isConnected) {
        // 尝试重新连接
        if (connectToServer()) {
            return m_connection->sendRequest(request);
        } else {
            // 返回错误响应
            QJsonObject errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = "Not connected to server";
            return errorResponse;
        }
    }
    
    return m_connection->sendRequest(request);
}

bool NetworkController::sendData(const QByteArray& data)
{
    if (!m_connection || !m_isConnected) {
        // 尝试重新连接
        if (!connectToServer()) {
            return false;
        }
    }
    
    return m_connection->sendData(data);
}

void NetworkController::onConnectionError(const QString& error)
{
    qDebug() << "Connection error:" << error;
    
    // 如果连接断开，尝试重连
    if (m_isConnected) {
        m_isConnected = false;
        startReconnect();
    }
}

void NetworkController::onDataReceived(const QByteArray& data)
{
    qDebug() << "Data received:" << data;
    
    // 这里可以处理接收到的数据
    // 例如，解析JSON并分发到相应的处理函数
}

void NetworkController::onConnected()
{
    qDebug() << "Connected to server";
    m_isConnected = true;
    
    // 停止重连定时器
    m_reconnectTimer->stop();
}

void NetworkController::onDisconnected()
{
    qDebug() << "Disconnected from server";
    m_isConnected = false;
    
    // 开始重连
    startReconnect();
}

void NetworkController::onReconnectTimer()
{
    qDebug() << "Attempting to reconnect..." << m_reconnectAttempts + 1;
    
    if (connectToServer()) {
        m_isConnected = true;
        m_reconnectTimer->stop();
    } else {
        m_reconnectAttempts++;
        
        // 检查是否达到最大重连次数
        if (m_reconnectAttempts >= m_maxReconnectAttempts) {
            qDebug() << "Max reconnection attempts reached";
            m_reconnectTimer->stop();
        }
    }
}

void NetworkController::createConnection()
{
    m_connection = NetworkConnectionFactory::createConnection(m_connectionType);
    
    if (m_connection) {
        connect(m_connection, &NetworkConnection::connectionError, this, &NetworkController::onConnectionError);
        connect(m_connection, &NetworkConnection::dataReceived, this, &NetworkController::onDataReceived);
        connect(m_connection, &NetworkConnection::connected, this, &NetworkController::onConnected);
        connect(m_connection, &NetworkConnection::disconnected, this, &NetworkController::onDisconnected);
    }
}

void NetworkController::startReconnect()
{
    // 如果已经在重连，不需要再次启动
    if (m_reconnectTimer->isActive()) {
        return;
    }
    
    // 开始重连定时器
    m_reconnectTimer->start(m_reconnectInterval);
}