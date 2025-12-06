#include "tcpserver.h"
#include "clientthreadfactory.h"  // 新增：包含线程工厂头文件

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    // 构造函数：初始化TCP服务器，暂无额外逻辑
}

// 新客户端连接处理
void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    // 调用线程工厂创建TCP线程任务
    Task* task = ClientThreadFactory::getInstance().createThread(ProtocolType::TCP, socketDescriptor);
    if (task != nullptr) {
        ThreadPool::getInstance().addTask(task);
    }
}

// TCP文件传输任务构造函数：保存客户端套接字描述符
TcpFileTask::TcpFileTask(qintptr socketDescriptor, QObject *parent)
    : Task(), m_socketDescriptor(socketDescriptor)
{
    Q_UNUSED(parent);  // 标记未使用的参数，消除编译器警告
}

// 任务执行逻辑：处理文件接收
void TcpFileTask::run()
{
    QTcpSocket socket;
    // 通过套接字描述符初始化通信套接字
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "TCP套接字初始化失败：" << socket.errorString();
        return;
    }

    qDebug() << "TCP客户端连接：" << socket.peerAddress().toString();

    // 【TCP大文件传输逻辑】
    // 协议约定：客户端先发送文件名+文件大小（格式自定义），再发送文件数据
    QByteArray buffer;
    // 等待客户端数据（超时30秒）
    while (socket.waitForReadyRead(30000)) {
        buffer += socket.readAll();
        // 后续需实现：
        // 1. 解析文件头（文件名、大小）
        // 2. 分块接收文件数据（避免内存溢出）
        // 3. 写入本地文件系统
        // 示例：简单回显（实际应替换为文件写入逻辑）
        socket.write("收到数据：" + buffer);
        buffer.clear();
    }

    qDebug() << "TCP客户端断开连接：" << socket.peerAddress().toString();
    socket.disconnectFromHost();  // 断开连接
}
