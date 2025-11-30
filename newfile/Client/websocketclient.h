#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QWebSocket>
#include <QObject>

// WebSocket客户端：负责与服务器进行实时通信
class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    // 连接到WebSocket服务器
    void connectToServer(const QString& ip, int port);
    // 向服务器发送实时消息
    void sendMessage(const QString& message);

signals:
    // 发送日志信息（供UI显示）
    void logMessage(const QString& msg);
    // 发送收到的消息（供UI显示）
    void messageReceived(const QString& msg);

private slots:
    // 连接成功回调
    void onConnected();
    // 断开连接回调
    void onDisconnected();
    // 收到消息回调
    void onTextMessageReceived(const QString& message);
    // 错误发生回调
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket* m_socket;  // 与服务器通信的WebSocket对象
};

#endif // WEBSOCKETCLIENT_H
