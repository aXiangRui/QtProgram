#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include "threadpool.h"

// TCP文件传输任务：处理单个客户端的大文件传输请求（每个客户端一个任务）
class TcpFileTask : public Task
{
public:
    // 构造函数：接收客户端套接字描述符
    explicit TcpFileTask(qintptr socketDescriptor, QObject *parent = nullptr);
    // 任务执行函数：实现文件接收逻辑
    void run() override;

private:
    qintptr m_socketDescriptor;  // 客户端套接字描述符（用于创建通信套接字）
};

// TCP服务器类：监听并处理客户端的文件传输连接
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

protected:
    // 新客户端连接触发：创建任务并提交到线程池
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // TCPSERVER_H
