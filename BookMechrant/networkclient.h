#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = 0);
    ~NetworkClient();

    bool connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    // 业务方法
    bool login(const QString &username, const QString &password);
    bool sendRequest(const QString &action, const QByteArray &data);
    bool sendRequest(const QByteArray &data);  // 重载版本
    QString getLastError() const;

signals:
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed(const QString &error);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onSocketReadyRead();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void reconnect();

private:
    void processResponse(const QByteArray &data);

    QTcpSocket *socket;
    QString serverHost;
    quint16 serverPort;
    QString lastError;
    bool connectedState;
};

#endif // NETWORKCLIENT_H
