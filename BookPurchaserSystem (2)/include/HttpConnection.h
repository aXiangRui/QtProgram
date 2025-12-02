#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "NetworkConnection.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QJsonDocument>

// HTTP连接实现
class HttpConnection : public NetworkConnection
{
    Q_OBJECT
public:
    HttpConnection(QObject *parent = nullptr);
    ~HttpConnection();

    bool connectToServer(const QString& url, int port) override;
    bool sendData(const QByteArray& data) override;
    void disconnect() override;
    QJsonObject sendRequest(const QJsonObject& request) override;

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
};

#endif // HTTPCONNECTION_H