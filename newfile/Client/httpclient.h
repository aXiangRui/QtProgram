#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

// HTTP客户端：负责向服务器发送业务请求（图书、用户等）
class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
//    // 设置服务器地址和端口（动态更新）
//    void setBaseUrl(const QString& ip, int port);

    void getBooks();       // 图书浏览
    void getUserInfo();    // 用户管理
    void getCart();        // 购物车
    void getOrders();      // 订单
    void getCollects();    // 收藏

signals:
    // 发送日志信息（供UI显示）
    void logMessage(const QString& msg);
    // 发送服务器响应数据（供UI显示）
    void responseReceived(const QString& data);

private slots:
    // 请求完成回调（处理服务器响应）
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;  // Qt网络请求管理器
    const QString m_baseUrl = "http://127.0.0.1:8080";// 固定http端口，服务器基础地址（http://ip:port）
};

#endif // HTTPCLIENT_H
