#include "../include/HttpConnection.h"

HttpConnection::HttpConnection(QObject *parent)
    : NetworkConnection(parent), m_networkManager(nullptr), m_currentReply(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &HttpConnection::onReplyFinished);
}

HttpConnection::~HttpConnection()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

bool HttpConnection::connectToServer(const QString& url, int port)
{
    m_serverUrl = url;
    m_serverPort = port;
    m_isConnected = true;
    emit connected();
    return true;
}

bool HttpConnection::sendData(const QByteArray& data)
{
    if (!m_isConnected)
        return false;

    QNetworkRequest request;
    request.setUrl(QUrl(QString("%1:%2").arg(m_serverUrl).arg(m_serverPort)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_currentReply = m_networkManager->post(request, data);
    return true;
}

void HttpConnection::disconnect()
{
    m_isConnected = false;
    emit disconnected();
}

QJsonObject HttpConnection::sendRequest(const QJsonObject& request)
{
    QNetworkAccessManager manager;
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(QString("%1:%2").arg(m_serverUrl).arg(m_serverPort)));
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QNetworkReply* reply = manager.post(networkRequest, QJsonDocument(request).toJson());
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJsonObject response;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        response = QJsonDocument::fromJson(data).object();
    } else {
        response["success"] = false;
        response["error"] = reply->errorString();
    }

    reply->deleteLater();
    return response;
}

void HttpConnection::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        emit dataReceived(data);
    } else {
        emit connectionError(reply->errorString());
    }

    reply->deleteLater();
    if (m_currentReply == reply) {
        m_currentReply = nullptr;
    }
}