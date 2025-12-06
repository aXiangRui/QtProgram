#include "serverwidget.h"
#include "ui_serverwidget.h"
#include "clienthandler.h"
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QThread>

// 注册全局异常捕获
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    // 将异常信息写入日志，避免程序崩溃
    QString log = QString("[%1] %2: %3").arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                     .arg(type == QtFatalMsg ? "Fatal" : "Error")
                     .arg(msg);
    QFile file("server_log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        file.write(log.toUtf8() + "\n");
        file.close();
    }
    // 致命错误时弹出提示，不退出程序
    if (type == QtFatalMsg) {
        QMessageBox::critical(nullptr, "服务器错误", log);
    }
}

ServerWidget::ServerWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ServerWidget), m_clientCount(0)
{
    // 安装全局异常捕获器
    qInstallMessageHandler(myMessageOutput);

    ui->setupUi(this);
    setWindowTitle("服务器");
    ui->portLineEdit->setText("8888");
    ui->clientCountLbl->setText("当前连接数：0");

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ServerWidget::onNewConnection);

    // 配置线程池，防止任务超时死锁
    QThreadPool::globalInstance()->setMaxThreadCount(200);  // 最大线程数
    QThreadPool::globalInstance()->setExpiryTimeout(30000); // 30秒超时清理
}

ServerWidget::~ServerWidget()
{
    qDebug() << "服务器主窗口析构";
    QTimer::singleShot(0, this, &ServerWidget::stopServer);
    delete ui;
}

void ServerWidget::stopServer()
{
    if (m_tcpServer->isListening()) {
        QMutexLocker locker(&m_socketMutex);
        QList<QPointer<QTcpSocket>> tempSockets = m_clientSockets;
        foreach (QPointer<QTcpSocket> socket, tempSockets) {
            if (socket && socket->state() == QAbstractSocket::ConnectedState) {
                socket->write(QString("服务器即将关闭，连接断开").toUtf8());
                socket->flush();
                socket->disconnectFromHost();
                socket->waitForDisconnected(500);
            }
        }
        m_clientSockets.clear();

        m_tcpServer->close();
        ui->startBtn->setText("启动服务器");
        appendLog("服务器已停止");
        m_clientCount.store(0);
        ui->clientCountLbl->setText("当前连接数：0");
    }
}

void ServerWidget::on_startBtn_clicked()
{
    if (m_tcpServer->isListening()) {
        stopServer();
    } else {
        quint16 port = ui->portLineEdit->text().toUShort();
        if (port == 0) {
            QMessageBox::warning(this, "错误", "请输入有效的端口号（1-65535）");
            return;
        }

        if (m_tcpServer->listen(QHostAddress::Any, port)) {
            ui->startBtn->setText("停止服务器");
            appendLog(QString("服务器启动成功，监听端口：%1").arg(port));
        } else {
            QMessageBox::critical(this, "启动失败",
                                 QString("无法监听端口 %1：%2").arg(port).arg(m_tcpServer->errorString()));
        }
    }
}

void ServerWidget::onNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *newSocket = m_tcpServer->nextPendingConnection();
        if (!newSocket) {
            appendError("获取新连接失败");
            continue;
        }

        QMutexLocker locker(&m_socketMutex);
        m_clientSockets.append(QPointer<QTcpSocket>(newSocket));

        ClientHandler *handler = new ClientHandler(newSocket);
        connect(handler, &ClientHandler::clientDisconnected, this, &ServerWidget::updateClientCount, Qt::QueuedConnection);
        connect(handler, &ClientHandler::logMessage, this, &ServerWidget::appendLog, Qt::QueuedConnection);
        connect(handler, &ClientHandler::errorOccurred, this, &ServerWidget::appendError, Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(handler);

        m_clientCount.fetchAndAddOrdered(1);
        ui->clientCountLbl->setText(QString("当前连接数：%1").arg(m_clientCount.load()));
    }
}

// 连接数刷新
void ServerWidget::updateClientCount()
{
    int current = m_clientCount.fetchAndAddOrdered(-1);
    if (current <= 0) {
        m_clientCount.store(0);
    }
    ui->clientCountLbl->setText(QString("当前连接数：%1").arg(m_clientCount.load()));

    QMutexLocker locker(&m_socketMutex);
    for (int i = m_clientSockets.size() - 1; i >= 0; --i) {
        QPointer<QTcpSocket> socket = m_clientSockets.at(i);
        if (!socket || socket->state() == QAbstractSocket::UnconnectedState) {
            m_clientSockets.removeAt(i);
        }
    }
}

void ServerWidget::appendLog(const QString &msg)
{
    ui->logTextEdit->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(msg));
}

void ServerWidget::appendError(const QString &err)
{
    ui->logTextEdit->append(QString("[%1] <font color='red'>错误：%2</font>").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(err));
}
