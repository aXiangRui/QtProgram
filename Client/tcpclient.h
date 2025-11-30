#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QFile>
#include <QObject>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, int port);
    void sendFile(const QString& filePath);  // 发送大文件

signals:
    void logMessage(const QString& msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket* m_socket;
};

#endif // TCPCLIENT_H
