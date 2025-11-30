#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    // 新客户端连接，创建文件传输任务并加入线程池
    TcpFileTask* task = new TcpFileTask(socketDescriptor);
    ThreadPool::getInstance().addTask(task);
}

TcpFileTask::TcpFileTask(qintptr socketDescriptor, QObject *parent)
    : Task(), m_socketDescriptor(socketDescriptor)
{
    Q_UNUSED(parent); // 新增：标记参数未使用
}

void TcpFileTask::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "TCP套接字初始化失败：" << socket.errorString();
        return;
    }

    qDebug() << "TCP客户端连接：" << socket.peerAddress().toString();

    // 【TCP大文件传输逻辑】
    // 协议约定：客户端先发送文件名+文件大小，再发送文件数据
    QByteArray buffer;
    while (socket.waitForReadyRead(30000)) {
        buffer += socket.readAll();
        // 后续需实现：解析文件头、接收文件数据、写入本地文件
        // 示例：简单回显（替换为文件写入逻辑）
        socket.write("收到数据：" + buffer);
        buffer.clear();
    }

    qDebug() << "TCP客户端断开连接：" << socket.peerAddress().toString();
    socket.disconnectFromHost();
}
