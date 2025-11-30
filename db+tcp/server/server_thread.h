#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// --- 负责处理单个客户端的线程类 ---
class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(qintptr socketDescriptor, QObject *parent = nullptr);
    void run() override; // 线程入口

signals:
    void error(QTcpSocket::SocketError socketError);

public slots:
    void onReadyRead();
    void onDisconnected();

private:
    qintptr m_socketDescriptor;  // 描述符，用于生成Socket
    QTcpSocket *m_socket;
    QString m_connectionName;    // 每个线程独立的数据库连接名
    QByteArray m_buffer;         // 用于缓存接收到的数据
    QString m_tableName;         // 当前客户端专属的表名
};

// --- 服务器类 ---
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);

protected:
    // 重写此函数以处理多线程
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // SERVER_THREAD_H
