#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include "threadpool.h"
#include "databasehelper.h"

// HTTP业务处理任务（每个请求一个线程）
class HttpTask : public Task
{
public:
    explicit HttpTask(qintptr socketDescriptor, QObject *parent = nullptr);
    void run() override;

private:
    void handleHttpRequest(const QString& request, QTcpSocket& socket);
    qintptr m_socketDescriptor;
};

// HTTP服务器类
class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // HTTPSERVER_H
