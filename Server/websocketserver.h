#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QDebug>
#include "threadpool.h"

// WebSocket实时通信任务
class WebSocketTask : public Task
{
public:
    explicit WebSocketTask(QWebSocket* socket, QObject *parent = nullptr);
    void run() override;

private:
    QWebSocket* m_socket;
};

// WebSocket服务器类
class WebSocketServer : public QWebSocketServer
{
    Q_OBJECT
public:
    explicit WebSocketServer(const QString& serverName, QWebSocketServer::SslMode secureMode, QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onSocketDisconnected();

private:
    QList<QWebSocket*> m_clients;  // 保存所有连接的客户端
};

#endif // WEBSOCKETSERVER_H
