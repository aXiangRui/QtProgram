#include "tcpclient.h"
#include <QFileInfo>
#include <QDebug>

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &TcpClient::onError);
}

void TcpClient::connectToServer(const QString &ip, int port)
{
    m_socket->connectToHost(ip, port);
}

void TcpClient::sendFile(const QString &filePath)
{
    // 替换isConnected()为state()判断
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit logMessage("未连接到TCP服务器，无法发送文件");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit logMessage("文件打开失败：" + file.errorString());
        return;
    }

    // 【TCP大文件传输】发送文件头（文件名+大小）+ 文件数据
    QFileInfo fileInfo(file);
    QString fileHeader = QString("%1|%2").arg(fileInfo.fileName()).arg(file.size());
    m_socket->write(fileHeader.toUtf8() + "\n");

    // 分块发送文件数据（避免内存溢出）
    const qint64 blockSize = 4096;
    qint64 bytesSent = 0;
    while (!file.atEnd()) {
        QByteArray block = file.read(blockSize);
        bytesSent += m_socket->write(block);
        m_socket->waitForBytesWritten(100);
    }

    file.close();
    emit logMessage(QString("文件发送完成：%1，已发送%2字节").arg(fileInfo.fileName()).arg(bytesSent));
}

void TcpClient::onConnected()
{
    emit logMessage("TCP服务器连接成功");
}

void TcpClient::onDisconnected()
{
    emit logMessage("TCP服务器断开连接");
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);  // 标记参数未使用，消除警告
    emit logMessage("TCP错误：" + m_socket->errorString());
}
