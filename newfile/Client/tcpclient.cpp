#include "tcpclient.h"
#include <QFileInfo>
#include <QDebug>

// TCP客户端构造函数：初始化套接字并连接信号槽
TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    // 连接状态信号槽
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    // 错误信号槽
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &TcpClient::onError);
}

// 连接到TCP服务器
void TcpClient::connectToServer(const QString &ip, int port)
{
    // 若已连接则先断开
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
    // 连接指定IP和端口
    m_socket->connectToHost(ip, port);
}

// 向服务器发送文件
void TcpClient::sendFile(const QString &filePath)
{
    // 检查是否已连接
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit logMessage("未连接到TCP服务器，无法发送文件");
        return;
    }

    QFile file(filePath);
    // 尝试打开文件（只读模式）
    if (!file.open(QIODevice::ReadOnly)) {
        emit logMessage("文件打开失败：" + file.errorString());
        return;
    }

    // 【TCP大文件传输协议】
    // 1. 先发送文件头：文件名|文件大小（自定义格式，便于服务器解析）
    QFileInfo fileInfo(file);
    QString fileHeader = QString("%1|%2").arg(fileInfo.fileName()).arg(file.size());
    m_socket->write(fileHeader.toUtf8() + "\n");  // 换行符作为分隔符

    // 2. 分块发送文件数据（4KB/块，避免内存溢出）
    const qint64 blockSize = 4096;
    qint64 bytesSent = 0;
    while (!file.atEnd()) {
        QByteArray block = file.read(blockSize);  // 读取一块数据
        bytesSent += m_socket->write(block);      // 发送数据
        m_socket->waitForBytesWritten(100);       // 等待数据发送完成
    }

    file.close();  // 关闭文件
    emit logMessage(QString("文件发送完成：%1，已发送%2字节").arg(fileInfo.fileName()).arg(bytesSent));
}

// 连接成功回调
void TcpClient::onConnected()
{
    emit logMessage("TCP服务器连接成功");
}

// 断开连接回调
void TcpClient::onDisconnected()
{
    emit logMessage("TCP服务器断开连接");
}

// 错误发生回调
void TcpClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);  // 标记未使用的参数
    emit logMessage("TCP错误：" + m_socket->errorString());
}
