#include "httpclient.h"
#include <QUrl>
#include <QDebug>

<<<<<<< HEAD
HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::getBooks()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/book"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：图书浏览");
}

=======
// HTTP客户端构造函数：初始化网络管理器
HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    // 网络请求完成时触发回调
    connect(m_manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}

// 设置服务器地址和端口
//void HttpClient::setBaseUrl(const QString& ip, int port)
//{
//    m_baseUrl = QString("http://%1:%2").arg(ip).arg(port);
//}

// 发送图书浏览请求
void HttpClient::getBooks()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/book"));  // 拼接请求路径
    m_manager->get(request);  // 发送GET请求
    emit logMessage("发送HTTP请求：图书浏览");
}

// 发送用户管理请求
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void HttpClient::getUserInfo()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/user"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：用户管理");
}

<<<<<<< HEAD
=======
// 发送购物车请求
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void HttpClient::getCart()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/cart"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：购物车");
}

<<<<<<< HEAD
=======
// 发送订单请求
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void HttpClient::getOrders()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/order"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：订单");
}

<<<<<<< HEAD
=======
// 发送收藏请求
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
void HttpClient::getCollects()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/collect"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：收藏");
}

<<<<<<< HEAD
void HttpClient::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit logMessage("HTTP请求失败：" + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    emit responseReceived(QString(data));
    emit logMessage("HTTP响应接收完成：" + QString(data).left(100) + "...");
    reply->deleteLater();
=======
// 处理服务器响应
void HttpClient::onReplyFinished(QNetworkReply *reply)
{
    // 检查是否有错误
    if (reply->error() != QNetworkReply::NoError) {
        emit logMessage("HTTP请求失败：" + reply->errorString());
        reply->deleteLater();  // 释放资源
        return;
    }

    // 读取响应数据并发送信号给UI
    QByteArray data = reply->readAll();
    emit responseReceived(QString(data));
    emit logMessage("HTTP响应接收完成：" + QString(data).left(100) + "...");  // 显示前100字符
    reply->deleteLater();  // 释放资源
>>>>>>> a6e5f20c20c0bb1ed7935f4533c383be92d0e8ab
}
