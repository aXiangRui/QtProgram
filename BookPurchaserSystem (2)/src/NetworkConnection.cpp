#include "../include/NetworkConnection.h"
#include "../include/HttpConnection.h"
#include "../include/TcpConnection.h"
#include "../include/WebSocketConnection.h"

NetworkConnection::NetworkConnection(QObject *parent)
    : QObject(parent), m_isConnected(false)
{
}

NetworkConnection::~NetworkConnection()
{
}

NetworkConnection* NetworkConnectionFactory::createConnection(NetworkConnectionFactory::ConnectionType type)
{
    switch (type) {
    case HTTP:
        return new HttpConnection();
    case TCP:
        return new TcpConnection();
    case WEBSOCKET:
        return new WebSocketConnection();
    default:
        return nullptr;
    }
}