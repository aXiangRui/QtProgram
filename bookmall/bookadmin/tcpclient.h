#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QByteArray>

// TCP客户端类 - 用于与服务端通信
class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    bool connectToServer(const QString &host = "localhost", quint16 port = 8888);
    void disconnectFromServer();
    bool isConnected() const;

    // 发送JSON请求并等待响应
    QJsonObject sendRequest(const QJsonObject &request, int timeout = 5000);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
    QJsonObject pendingResponse;
    bool responseReceived;
    QTimer *timeoutTimer;
    QMutex responseMutex;
    QWaitCondition responseCondition;
    QByteArray recvBuffer;  // 接收缓冲区，用于处理分片数据
};

#endif // TCPCLIENT_H

