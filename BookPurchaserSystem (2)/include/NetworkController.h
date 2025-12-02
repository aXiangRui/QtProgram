#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include "NetworkConnection.h"

// 网络控制器类，负责管理网络连接和请求
class NetworkController : public QObject
{
    Q_OBJECT
signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
public:
    NetworkController(QObject *parent = nullptr);
    ~NetworkController();

    // 设置服务器地址和端口
    void setServerAddress(const QString& address, int port);
    
    // 设置连接类型
    void setConnectionType(NetworkConnectionFactory::ConnectionType type);
    
    // 连接到服务器
    bool connectToServer();
    
    // 断开连接
    void disconnectFromServer();
    
    // 发送请求
    QJsonObject sendRequest(const QJsonObject& request);
    
    // 发送数据
    bool sendData(const QByteArray& data);
    
    // 是否连接
    bool isConnected() const { return m_isConnected; }

private slots:
    void onConnectionError(const QString& error);
    void onDataReceived(const QByteArray& data);
    void onConnected();
    void onDisconnected();
    void onReconnectTimer();

private:
    NetworkConnection* m_connection;
    QString m_serverAddress;
    int m_serverPort;
    bool m_isConnected;
    NetworkConnectionFactory::ConnectionType m_connectionType;
    QTimer* m_reconnectTimer;
    int m_reconnectInterval; // 重连间隔（毫秒）
    int m_maxReconnectAttempts;
    int m_reconnectAttempts;
    
    // 创建网络连接
    void createConnection();
    
    // 开始重连
    void startReconnect();
};

#endif // NETWORKCONTROLLER_H
