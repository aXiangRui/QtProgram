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
    // Q_OBJECT    加了这个会报错，未知原因
public:
    // 构造函数：接收WebSocket连接对象
    explicit WebSocketTask(QWebSocket* socket, QObject *parent = nullptr);
    // 任务执行函数：处理实时消息交互
    void run() override;

private:
    QWebSocket* m_socket;  // 与客户端通信的WebSocket对象
};

// WebSocket服务器类：监听并管理实时通信连接
class WebSocketServer : public QWebSocketServer
{
    Q_OBJECT
public:
    // 构造函数：指定服务器名称和安全模式（非加密）
    explicit WebSocketServer(const QString& serverName, QWebSocketServer::SslMode secureMode, QObject *parent = nullptr);

private slots:
    // 新客户端连接触发
    void onNewConnection();
    // 客户端断开连接触发
    void onSocketDisconnected();

private:
    QList<QWebSocket*> m_clients;  // 保存所有连接的客户端（用于广播等功能）
};

#endif // WEBSOCKETSERVER_H
