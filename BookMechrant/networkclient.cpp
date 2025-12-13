#include "NetworkClient.h"
#include <QHostAddress>
#include <QTimer>
#include <QDataStream>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), serverPort(0), connectedState(false)
{
    socket = new QTcpSocket(this);

    // 连接信号
    connect(socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));

    // 设置超时
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
}

NetworkClient::~NetworkClient()
{
    disconnectFromServer();
}

bool NetworkClient::connectToServer(const QString &host, quint16 port)
{
    if (isConnected()) {
        disconnectFromServer();
    }

    serverHost = host;
    serverPort = port;
    lastError.clear();

    socket->connectToHost(host, port);

    // 等待连接建立
    if (socket->waitForConnected(3000)) {
        connectedState = true;
        return true;
    } else {
        lastError = socket->errorString();
        emit errorOccurred(lastError);
        return false;
    }
}

void NetworkClient::disconnectFromServer()
{
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
    }
    connectedState = false;
}

bool NetworkClient::isConnected() const
{
    return connectedState && socket->state() == QAbstractSocket::ConnectedState;
}

bool NetworkClient::login(const QString &username, const QString &password)
{
    if (!isConnected()) {
        lastError = "未连接到服务器";
        return false;
    }

    // 构建登录请求
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_8);

    stream << (quint8)0x01; // 协议版本
    stream << (quint16)0x0001; // 命令: 登录
    stream << username;
    stream << password;

    // 发送登录请求
    if (sendRequest(data)) {
        // 等待响应
        if (socket->waitForReadyRead(3000)) {
            // 响应在 onSocketReadyRead 中处理
            return true;
        } else {
            lastError = "登录超时";
            emit loginFailed(lastError);
            return false;
        }
    }

    return false;
}

bool NetworkClient::sendRequest(const QString &action, const QByteArray &data)
{
    if (!isConnected()) {
        lastError = "未连接到服务器";
        return false;
    }

    // 构建协议数据包
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_8);

    // 协议格式: [4字节数据长度][数据]
    quint32 dataSize = data.size();
    stream << dataSize;
    packet.append(data);

    // 发送数据
    qint64 written = socket->write(packet);
    bool success = written == packet.size() && socket->waitForBytesWritten(3000);

    if (!success) {
        lastError = QString("发送数据失败: %1").arg(socket->errorString());
        emit errorOccurred(lastError);
    }

    return success;
}

bool NetworkClient::sendRequest(const QByteArray &data)
{
    return sendRequest("", data);
}

QString NetworkClient::getLastError() const
{
    return lastError;
}

void NetworkClient::onSocketConnected()
{
    connectedState = true;
    lastError.clear();
    emit connected();
}

void NetworkClient::onSocketDisconnected()
{
    connectedState = false;
    emit disconnected();
}

void NetworkClient::onSocketReadyRead()
{
    while (socket->bytesAvailable() > 0) {
        // 检查是否有完整的数据包
        if (socket->bytesAvailable() < sizeof(quint32)) {
            return; // 等待更多数据
        }

        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_8);

        // 读取数据长度
        quint32 dataSize;
        in >> dataSize;

        // 检查数据是否全部到达
        if (socket->bytesAvailable() < dataSize) {
            // 重置流位置，等待完整数据
            socket->ungetChar('\0'); // 简单处理，实际应该更精确
            return;
        }

        // 读取完整数据
        QByteArray data = socket->read(dataSize);
        emit dataReceived(data);

        // 解析响应
        processResponse(data);
    }
}

void NetworkClient::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    lastError = socket->errorString();
    connectedState = false;

    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            lastError = "连接被拒绝";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            lastError = "服务器关闭连接";
            break;
        case QAbstractSocket::HostNotFoundError:
            lastError = "找不到服务器";
            break;
        case QAbstractSocket::SocketTimeoutError:
            lastError = "连接超时";
            break;
        case QAbstractSocket::NetworkError:
            lastError = "网络错误";
            break;
        default:
            lastError = QString("网络错误: %1").arg(socket->errorString());
            break;
    }

    emit errorOccurred(lastError);

    // 如果是连接错误，自动重连
    if (socketError == QAbstractSocket::ConnectionRefusedError ||
        socketError == QAbstractSocket::RemoteHostClosedError) {
        QTimer::singleShot(5000, this, SLOT(reconnect()));
    }
}

void NetworkClient::processResponse(const QByteArray &data)
{
    // 简单响应处理
    QString response = QString::fromUtf8(data);

    if (response.contains("login_success")) {
        emit loginSuccess();
    } else if (response.contains("login_failed")) {
        QString error = response.mid(response.indexOf(":") + 1);
        emit loginFailed(error.trimmed());
    } else if (response.contains("error")) {
        QString error = response.mid(response.indexOf(":") + 1);
        emit errorOccurred(error.trimmed());
    }
}

void NetworkClient::reconnect()
{
    if (!isConnected() && !serverHost.isEmpty() && serverPort > 0) {
        connectToServer(serverHost, serverPort);
    }
}
