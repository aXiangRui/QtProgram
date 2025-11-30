#include "clienthandler.h"
#include <QHostAddress>
#include <QDir>
#include <QThread>
#include <QMetaObject>
#include <QDebug>
#include <QXmlStreamReader>
#include <QFileDevice>

QMutex xmlMutex;

ClientHandler::ClientHandler(QTcpSocket *socket, QObject *parent)
    : QObject(parent), QRunnable(), m_socket(socket),
      m_isRunning(true), m_isSocketConnected(true)
{
    setAutoDelete(true);
    m_socket->setParent(nullptr);
    // 跨线程连接定时检查信号（连接到当前线程的事件循环）
    connect(this, &ClientHandler::checkSocketState, this, &ClientHandler::onCheckSocketState, Qt::DirectConnection);
}

ClientHandler::~ClientHandler()
{
    qDebug() << "ClientHandler析构，清理资源";
    // 加锁修改状态
    QMutexLocker locker(&m_stateMutex);
    m_isRunning = false;
    m_isSocketConnected = false;
    if (m_socket) {
        QTimer::singleShot(100, this, &ClientHandler::deleteSocketLater);
    }
}

void ClientHandler::deleteSocketLater()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
        qDebug() << "Socket已安全删除";
    }
}

void ClientHandler::run()
{
    if (!m_socket) {
        emit errorOccurred("无效的Socket对象");
        QMutexLocker locker(&m_stateMutex);
        m_isRunning = false;
        return;
    }

    // 绑定Socket信号槽（异步，非阻塞）
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead, Qt::DirectConnection);
    connect(m_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &ClientHandler::onSocketError, Qt::DirectConnection);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected, Qt::DirectConnection);

    // 发送连接成功日志
    QString clientIp = m_socket->peerAddress().toString();
    quint16 clientPort = m_socket->peerPort();
    emit logMessage(QString("客户端 [%1:%2] 连接成功").arg(clientIp).arg(clientPort));

    // 启动定时检查Socket状态（替代无限阻塞，每500ms检查一次）
    while (true) {
        // 加锁读取状态
        QMutexLocker locker(&m_stateMutex);
        if (!m_isRunning || !m_isSocketConnected) {
            break;
        }
        locker.unlock(); // 解锁后再休眠，避免阻塞其他线程

        QMetaObject::invokeMethod(this, "checkSocketState", Qt::DirectConnection);
        QThread::msleep(500); // 非阻塞休眠，让出CPU资源
    }

    qDebug() << "客户端处理任务正常退出";
}

// 接收信息
void ClientHandler::onReadyRead()
{
    QMutexLocker locker(&m_stateMutex);
    if (!m_socket || !m_isSocketConnected) return;
    locker.unlock();

    QByteArray data = m_socket->readAll();
    QString message = QString::fromUtf8(data);
//    QString clientIp = m_socket->peerAddress().toString();

    emit logMessage(QString(" %3").arg(message));
//    saveMessageToXml(clientIp, message);
    m_socket->write(QString("服务器已接收：%1").arg(message).toUtf8());

}

// 错误处理
void ClientHandler::onSocketError(QAbstractSocket::SocketError error)
{
    // 核心：过滤客户端主动断开的错误（RemoteHostClosedError）
    if (error == QAbstractSocket::RemoteHostClosedError) {
        qDebug() << "客户端主动断开连接，忽略该错误";
        m_isSocketConnected = false; // 仅标记状态，不发送错误日志
        return;
    }

    // 其他真正的异常错误，才记录并发送错误日志
    if (!m_socket) return;
    QString errMsg = QString("客户端Socket错误：%1").arg(m_socket->errorString());
    emit errorOccurred(errMsg);
    m_isSocketConnected = false; // 标记为断开，退出循环
}

void ClientHandler::onDisconnected()
{
    if (!m_socket) return;
    QString clientIp = m_socket->peerAddress().toString();
    quint16 clientPort = m_socket->peerPort();
    emit logMessage(QString("客户端 [%1:%2] 断开连接").arg(clientIp).arg(clientPort));
    emit clientDisconnected();

    // 加锁修改状态
    QMutexLocker locker(&m_stateMutex);
    m_isSocketConnected = false; // 标记为断开，退出循环
    locker.unlock();

    m_socket->close();
}

void ClientHandler::onCheckSocketState()
{
    // 检查Socket的实际状态，更新标记
    if (m_socket && m_socket->state() != QAbstractSocket::ConnectedState) {
        QMutexLocker locker(&m_stateMutex);
        m_isSocketConnected = false;
    }
}

//void ClientHandler::saveMessageToXml(const QString &clientIp, const QString &message)
//{
//    QMutexLocker locker(&xmlMutex); // 保持加锁，防止多线程同时写入
//    QFile file("messages.xml");
//    QDomDocument doc; // 使用QDomDocument操作XML，保证结构合法

//    // 步骤1：读取现有XML文件（若存在）
//    if (file.exists()) {
//        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//            emit errorOccurred(QString("XML文件打开失败：%1").arg(file.errorString()));
//            return;
//        }

//        // 解析XML文档，若解析失败则重建
//        QString errorMsg;
//        int errorLine, errorCol;
//        if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
//            emit errorOccurred(QString("XML文件解析错误：%1（行%2，列%3）").arg(errorMsg).arg(errorLine).arg(errorCol));
//            doc.clear(); // 清空错误文档，重建
//        }
//        file.close();
//    }

//    // 步骤2：初始化XML文档（若为空）
//    if (doc.isNull() || doc.documentElement().isNull()) {
//        QDomElement root = doc.createElement("Messages"); // 创建根节点
//        doc.appendChild(root);
//    }

//    // 步骤3：创建新的消息节点
//    QDomElement messageNode = doc.createElement("Message");
//    // 设置属性：客户端IP（转义特殊字符）
//    messageNode.setAttribute("clientIp", doc.createTextNode(clientIp).data());
//    // 设置属性：时间（转义特殊字符）
//    messageNode.setAttribute("time", doc.createTextNode(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).data());
//    // 创建内容节点：消息内容（强制转义特殊字符）
//    QDomElement contentNode = doc.createElement("Content");
//    QDomText contentText = doc.createTextNode(message); // createTextNode会自动转义特殊字符
//    contentNode.appendChild(contentText);
//    messageNode.appendChild(contentNode);

//    // 步骤4：将新节点添加到根节点
//    doc.documentElement().appendChild(messageNode);

//    // 步骤5：写入XML文件（覆盖写入，保证结构完整）
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
//        emit errorOccurred(QString("XML文件写入失败：%1").arg(file.errorString()));
//        return;
//    }

//    // 格式化XML输出（缩进4个空格，便于阅读）
//    QTextStream stream(&file);
//    doc.save(stream, 4); // 4个空格缩进
//    file.flush(); // 强制刷新缓冲区
//    file.close();
//}
