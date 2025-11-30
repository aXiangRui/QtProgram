#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    // 业务请求接口
    void getBooks();       // 图书浏览
    void getUserInfo();    // 用户管理
    void getCart();        // 购物车
    void getOrders();      // 订单
    void getCollects();    // 收藏

signals:
    void logMessage(const QString& msg);
    void responseReceived(const QString& data);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
    const QString m_baseUrl = "http://127.0.0.1:8080";// 固定http端口
};

#endif // HTTPCLIENT_H
