#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QWebSocket>
#include <QObject>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, int port);
    void sendMessage(const QString& message);  // 发送实时消息

signals:
    void logMessage(const QString& msg);
    void messageReceived(const QString& msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket* m_socket;
};

#endif // WEBSOCKETCLIENT_H
