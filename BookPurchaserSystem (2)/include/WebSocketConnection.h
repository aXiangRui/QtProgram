#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include "NetworkConnection.h"
#include <QWebSocket>
#include <QJsonDocument>

// WebSocket连接实现
class WebSocketConnection : public NetworkConnection
{
    Q_OBJECT
public:
    WebSocketConnection(QObject *parent = nullptr);
    ~WebSocketConnection();

    bool connectToServer(const QString& url, int port) override;
    bool sendData(const QByteArray& data) override;
    void disconnect() override;
    QJsonObject sendRequest(const QJsonObject& request) override;

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket* m_socket;
    QByteArray m_receiveBuffer;
    QJsonObject m_lastResponse;
    bool m_responseReceived;
};

#endif // WEBSOCKETCONNECTION_H