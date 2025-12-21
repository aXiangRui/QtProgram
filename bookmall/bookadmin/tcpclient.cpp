#include "tcpclient.h"
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <QDataStream>
#include <QElapsedTimer>
#include <QDateTime>
#include <QEventLoop>
#include <QCoreApplication>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent), socket(nullptr), responseReceived(false)
{
    socket = new QTcpSocket(this);
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);

    connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpClient::onError);
    connect(timeoutTimer, &QTimer::timeout, this, [this]() {
        QMutexLocker locker(&responseMutex);
        if (!responseReceived) {
            responseReceived = true;
            responseCondition.wakeAll();
        }
    });
    
    recvBuffer.clear();
}

TcpClient::~TcpClient()
{
    disconnectFromServer();
}

bool TcpClient::connectToServer(const QString &host, quint16 port)
{
    // 验证参数
    if (host.isEmpty()) {
        qDebug() << "错误：主机地址为空";
        return false;
    }
    if (port == 0) {
        qDebug() << "错误：端口号为0";
        return false;
    }
    
    if (socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "已经连接到服务器，无需重复连接";
        return true;
    }

    qDebug() << "正在连接到服务器:" << host << ":" << port;
    socket->connectToHost(host, port);
    if (socket->waitForConnected(5000)) {
        qDebug() << "成功连接到服务器" << host << ":" << port;
        return true;
    } else {
        qDebug() << "连接失败:" << socket->errorString() << "(" << host << ":" << port << ")";
        return false;
    }
}

void TcpClient::disconnectFromServer()
{
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(3000);
    }
}

bool TcpClient::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

QJsonObject TcpClient::sendRequest(const QJsonObject &request, int timeout)
{
    if (!isConnected()) {
        QJsonObject errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"] = "未连接到服务器";
        qDebug() << "发送请求失败：未连接到服务器";
        return errorResponse;
    }

    // 使用请求ID来匹配响应（简单实现，使用时间戳）
    qint64 requestId = QDateTime::currentMSecsSinceEpoch();
    
    QMutexLocker locker(&responseMutex);
    responseReceived = false;
    pendingResponse = QJsonObject();
    recvBuffer.clear();  // 清空接收缓冲区

    // 发送请求 - 使用长度前缀协议：4字节大端长度 + JSON
    QJsonDocument doc(request);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    
    QByteArray frame;
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << (quint32)payload.size();
    frame.append(payload);
    
    qDebug() << "发送请求到服务器，JSON大小:" << payload.size() << "字节，总帧大小:" << frame.size() << "字节";
    qDebug() << "请求内容:" << QString::fromUtf8(payload);
    
    qint64 bytesWritten = socket->write(frame);
    socket->flush();
    
    if (bytesWritten != frame.size()) {
        qDebug() << "警告：数据未完全发送，已发送:" << bytesWritten << "总大小:" << frame.size();
    } else {
        qDebug() << "数据已成功发送";
    }

    // 等待响应 - 关键：必须处理事件循环，否则onReadyRead无法被调用
    qDebug() << "等待服务器响应，超时时间:" << timeout << "ms";
    
    // 释放锁，让事件循环能够运行
    locker.unlock();
    
    timeoutTimer->start(timeout);
    
    // 使用QEventLoop确保事件循环运行，这样onReadyRead才能被调用
    QEventLoop eventLoop;
    QTimer timeoutTimer2;
    timeoutTimer2.setSingleShot(true);
    connect(&timeoutTimer2, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timeoutTimer2.start(timeout);
    
    // 使用QElapsedTimer精确计时
    QElapsedTimer timer;
    timer.start();
    
    // 循环等待，处理事件循环
    bool received = false;
    while (timeoutTimer2.isActive() && timer.elapsed() < timeout) {
        // 检查是否收到响应
        {
            QMutexLocker checkLocker(&responseMutex);
            if (responseReceived) {
                received = true;
                qDebug() << "收到服务器响应（耗时:" << timer.elapsed() << "ms）";
                break;
            }
        }
        
        // 处理事件循环，让onReadyRead能够被调用（关键！）
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        
        // 如果超时，退出
        if (timer.elapsed() >= timeout && !responseReceived) {
            if (timer.elapsed() < timeout + 100) {  // 只在第一次超时时打印
                qDebug() << "已超过超时时间(" << timeout << "ms)，继续等待延迟响应...";
            }
        }
    }
    
    timeoutTimer->stop();
    timeoutTimer2.stop();
    
    // 最后再处理一次事件，确保刚到达的响应能被处理
    if (!received) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QMutexLocker checkLocker(&responseMutex);
        if (responseReceived) {
            received = true;
            qDebug() << "收到延迟的服务器响应（在最后处理事件后，总耗时:" << timer.elapsed() << "ms）";
        }
    }
    
    // 重新获取锁
    locker.relock();
    
    if (responseReceived && received) {
        qDebug() << "成功收到服务器响应";
        QJsonObject result = pendingResponse;
        // 重置状态，准备下次请求
        responseReceived = false;
        pendingResponse = QJsonObject();
        return result;
    } else {
        qDebug() << "请求超时，未收到服务器响应（等待了" << timer.elapsed() << "ms）";
        QJsonObject errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"] = "请求超时（服务器可能未响应）";
        return errorResponse;
    }
}

void TcpClient::onConnected()
{
    qDebug() << "已连接到服务器";
    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "与服务器断开连接";
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    QByteArray data = socket->readAll();
    recvBuffer.append(data);
    qDebug() << "收到服务器数据，大小:" << data.size() << "字节，缓冲区总大小:" << recvBuffer.size() << "字节";
    
    // 解析长度前缀协议：4字节大端长度 + JSON payload
    while (recvBuffer.size() >= 4) {
        // 读取长度前缀
        QDataStream ds(recvBuffer.left(4));
        ds.setByteOrder(QDataStream::BigEndian);
        quint32 payloadLen = 0;
        ds >> payloadLen;
        
        // 防御：检查payload长度是否合理（最大10MB）
        if (payloadLen > 10 * 1024 * 1024) {
            qDebug() << "错误：payload长度过大:" << payloadLen << "，关闭连接";
            socket->close();
            return;
        }
        
        // 检查是否收到完整帧
        if (recvBuffer.size() < 4 + (int)payloadLen) {
            // 数据不完整，等待更多数据
            qDebug() << "数据不完整，等待更多数据。需要:" << (4 + payloadLen) << "字节，当前:" << recvBuffer.size() << "字节";
            break;
        }
        
        // 提取JSON payload
        QByteArray payload = recvBuffer.mid(4, payloadLen);
        recvBuffer = recvBuffer.mid(4 + payloadLen);  // 移除已处理的数据
        
        qDebug() << "解析到完整帧，JSON大小:" << payload.size() << "字节";
        qDebug() << "JSON内容:" << QString::fromUtf8(payload);
        
        // 解析JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(payload, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QMutexLocker locker(&responseMutex);
            pendingResponse = doc.object();
            responseReceived = true;
            qDebug() << "JSON解析成功，响应内容:" << QJsonDocument(pendingResponse).toJson(QJsonDocument::Compact);
            responseCondition.wakeAll();  // 唤醒等待的线程
        } else {
            qDebug() << "接收到的数据格式错误:" << error.errorString();
            qDebug() << "错误位置:" << error.offset;
        }
    }
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    QString errorString = socket->errorString();
    qDebug() << "Socket错误:" << errorString;
    emit errorOccurred(errorString);
}

