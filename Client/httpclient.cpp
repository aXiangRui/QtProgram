#include "httpclient.h"
#include <QUrl>
#include <QDebug>

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

void HttpClient::getUserInfo()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/user"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：用户管理");
}

void HttpClient::getCart()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/cart"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：购物车");
}

void HttpClient::getOrders()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/order"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：订单");
}

void HttpClient::getCollects()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/collect"));
    m_manager->get(request);
    emit logMessage("发送HTTP请求：收藏");
}

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
}
