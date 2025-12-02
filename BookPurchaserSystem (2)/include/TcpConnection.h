#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "NetworkConnection.h"
#include <QTcpSocket>
#include <QJsonDocument>

// TCP连接实现
class TcpConnection : public NetworkConnection
{
    Q_OBJECT
public:
    TcpConnection(QObject *parent = nullptr);
    ~TcpConnection();

    bool connectToServer(const QString& url, int port) override;
    bool sendData(const QByteArray& data) override;
    void disconnect() override;
    QJsonObject sendRequest(const QJsonObject& request) override;

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* m_socket;
    QByteArray m_receiveBuffer;
};

#endif // TCPCONNECTION_H