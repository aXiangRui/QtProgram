#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>

// 网络连接基类
class NetworkConnection : public QObject
{
    Q_OBJECT
public:
    NetworkConnection(QObject *parent = nullptr);
    virtual ~NetworkConnection();

    virtual bool connectToServer(const QString& url, int port) = 0;
    virtual bool sendData(const QByteArray& data) = 0;
    virtual void disconnect() = 0;
    virtual QJsonObject sendRequest(const QJsonObject& request) = 0;

    void setServerUrl(const QString& url) { m_serverUrl = url; }
    void setServerPort(int port) { m_serverPort = port; }

signals:
    void dataReceived(const QByteArray& data);
    void connectionError(const QString& error);
    void connected();
    void disconnected();

protected:
    QString m_serverUrl;
    int m_serverPort;
    bool m_isConnected;
};

// 网络连接工厂类
class NetworkConnectionFactory
{
public:
    enum ConnectionType {
        HTTP,
        TCP,
        WEBSOCKET
    };

    static NetworkConnection* createConnection(ConnectionType type);
};

#endif // NETWORKCONNECTION_H