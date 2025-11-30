#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include "threadpool.h"

// TCP文件传输任务（每个客户端一个线程）
class TcpFileTask : public Task
{
public:
    explicit TcpFileTask(qintptr socketDescriptor, QObject *parent = nullptr);
    void run() override;

private:
    qintptr m_socketDescriptor;
};

// TCP服务器类
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // TCPSERVER_H
