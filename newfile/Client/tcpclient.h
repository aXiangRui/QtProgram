#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QFile>
#include <QObject>

// TCP客户端：负责与服务器进行大文件传输
class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    // 连接到TCP服务器
    void connectToServer(const QString& ip, int port);
    // 向服务器发送文件
    void sendFile(const QString& filePath);

signals:
    // 发送日志信息（供UI显示）
    void logMessage(const QString& msg);

private slots:
    // 连接成功回调
    void onConnected();
    // 断开连接回调
    void onDisconnected();
    // 错误发生回调
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* m_socket;  // 与服务器通信的TCP套接字
};

#endif // TCPCLIENT_H
