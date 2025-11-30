#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include "threadpool.h"
#include "databasehelper.h"

// HTTP业务处理任务：处理单个HTTP请求（每个请求一个任务）
class HttpTask : public Task
{
public:
    // 构造函数：接收客户端套接字描述符
    explicit HttpTask(qintptr socketDescriptor, QObject *parent = nullptr);
    // 任务执行函数：解析HTTP请求并处理
    void run() override;

private:
    // 处理HTTP请求的核心逻辑
    void handleHttpRequest(const QString& request, QTcpSocket& socket);
    qintptr m_socketDescriptor;  // 客户端套接字描述符
};

// HTTP服务器类：监听并处理客户端的业务请求
class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);

protected:
    // 新客户端连接触发：创建任务并提交到线程池
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // HTTPSERVER_H
