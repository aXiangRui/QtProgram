#include "clientwidget.h"
#include "ui_clientwidget.h"
#include <QMessageBox>
#include <QDateTime>
#include <QHostAddress>
#include<QDebug>
#include <QApplication>  // 新增：用于设置应用程序退出规则

// ------------------------ 登录界面头文件（根据实际路径调整）,可以增加相应的界面/功能时增加其头文件 -----------------------------
// #include<../login_registr/login_register.h>



// --------------------------------------------------------------------------------------------------------------------

ClientWidget::ClientWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ClientWidget), m_isConnected(false)
{
//    // 隐藏窗口
//    this->hide();
//    // 强制设置窗口为不可见（双重保险）
//    this->setVisible(false);

    ui->setupUi(this);
    setWindowTitle("客户端");
    ui->sendBtn->setEnabled(false);       // 未连接时禁用发送


    // 初始化TCP套接字
    m_tcpSocket = new QTcpSocket(this);
    // 连接TCP套接字关键信号
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &ClientWidget::onReadyRead);
    connect(m_tcpSocket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &ClientWidget::onSocketError);
    connect(m_tcpSocket, &QTcpSocket::connected, this, &ClientWidget::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &ClientWidget::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &ClientWidget::onStateChanged);

    // 初始化网络检测
    m_netManager = new QNetworkConfigurationManager(this);

    // ========== 新增：初始化定时刷新定时器 ==========
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(REFRESH_INTERVAL);  // 设置定时周期
    connect(m_refreshTimer, &QTimer::timeout, this, &ClientWidget::onTimerRefresh);
    m_refreshTimer->start();  // 启动定时器

    // 客户端启动后执行一次自动连接
    appendMsg("客户端启动，开始自动连接服务器...");
    connectToServer();

    // ========== 设置应用程序退出规则（仅当最后一个窗口关闭时退出） ==========
    // 默认Qt应用程序在主窗口关闭时退出，若需要登录界面关闭后再退出，添加此行
    qApp->setQuitOnLastWindowClosed(true);
}

ClientWidget::~ClientWidget()
{
    // 断开TCP连接
    if (m_tcpSocket->isOpen()) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->waitForDisconnected(1000);
    }
    // 停止并释放定时器
    m_refreshTimer->stop();
    delete m_refreshTimer;
    // 释放UI
    delete ui;
}

// 检测网络是否可用
bool ClientWidget::checkNetworkAvailable()
{
    return m_netManager->isOnline();
}

// 通用连接函数（自动/手动连接均调用）
void ClientWidget::connectToServer()
{


    // 检测网络可用性
    if (!checkNetworkAvailable()) {
        appendMsg("连接失败，当前无可用网络", true);
        return;
    }

    // 避免重复连接（正在连接中则返回）
    if (m_tcpSocket->state() == QAbstractSocket::ConnectingState) {
        appendMsg("正在连接中，请勿重复操作", true);
        return;
    }

    // 执行连接
    m_tcpSocket->connectToHost(SERVER_IP, SERVER_PORT);
    appendMsg(QString("尝试连接 %1:%2...").arg(SERVER_IP).arg(SERVER_PORT));

    // 3秒连接超时处理
    if (!m_tcpSocket->waitForConnected(3000)) {
        QString errMsg = QString("连接失败：%1").arg(m_tcpSocket->errorString());
        appendMsg(errMsg, true);
        m_tcpSocket->abort(); // 终止无效连接
    }
}

// 刷新/连接按钮点击事件（客户端窗口内，已隐藏，以后可更改为应用界面内刷新）
void ClientWidget::on_connectBtn_clicked()
{
    if (!m_isConnected) {
        // 未连接：执行手动刷新连接
        appendMsg("【手动刷新】开始重新连接服务器...");
        connectToServer();
    } else {
        // 已连接：先断开再重连
        appendMsg("【手动刷新】先断开当前连接...");
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->waitForDisconnected(1000)) {
            appendMsg("【手动刷新】已断开连接，开始重新连接...");
            connectToServer();
        } else {
            appendMsg("【手动刷新】断开超时，强制终止后重连...", true);
            m_tcpSocket->abort();
            connectToServer();
        }
    }
}

// 发送消息按钮点击事件（客户端窗口已隐藏，暂无用，以后看情况可删）
void ClientWidget::on_sendBtn_clicked()
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        appendMsg("发送失败：未连接到服务器", true);
        return;
    }

    QString msg = ui->msgLineEdit->text().trimmed();
    if (msg.isEmpty()) {
        QMessageBox::warning(this, "警告", "消息不能为空");
        return;
    }

    // 拼接客户端ID和消息（格式：ID:消息）
    m_clientId = ui->idEdit->text().trimmed();
    QString sendMsg = QString("%1:%2").arg(m_clientId).arg(msg);

    // 发送消息
    m_tcpSocket->write(sendMsg.toUtf8());
    appendMsg(QString("发送：%1").arg(sendMsg));
    ui->msgLineEdit->clear();
}

// 接收服务器数据
void ClientWidget::onReadyRead()
{
    QByteArray data = m_tcpSocket->readAll();
    appendMsg(QString("接收：%1").arg(QString::fromUtf8(data)));
}

// TCP套接字错误处理
void ClientWidget::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    appendMsg(QString("网络错误：%1").arg(m_tcpSocket->errorString()), true);
}

// 连接成功处理
void ClientWidget::onConnected()
{
    m_isConnected = true;
    ui->connectBtn->setText("刷新");
    ui->sendBtn->setEnabled(true);

    // 连接成功后停止定时刷新（避免无效重连）
    m_refreshTimer->stop();
//    appendMsg(QString("【定时刷新】已连接，暂停%1秒定时检查").arg(REFRESH_INTERVAL / 1000));
    qDebug()<<"连接成功，定时刷新功能暂停";

    // --------------- 跳转到登录界面（可在此处添加各种页面的切换）---------------------
    // login_register *w = new login_register();
    // w->show();
    // this->hide();
    // --------------------------------------------------------------------------
    appendMsg("已成功连接到服务器");
    // this->hide();
}

// 断开连接处理
void ClientWidget::onDisconnected()
{
    m_isConnected = false;
    ui->connectBtn->setText("刷新");
    ui->sendBtn->setEnabled(false);

    // 断开后重启定时刷新
    m_refreshTimer->start();
//    appendMsg(QString("【定时刷新】已断开，重启%1秒定时检查").arg(REFRESH_INTERVAL / 1000), true);
    qDebug()<<"【定时刷新】已断开，重启定时检查";
    appendMsg("与服务器断开连接", true);
}

// TCP状态变化处理
void ClientWidget::onStateChanged(QAbstractSocket::SocketState state)
{
    QString stateStr;
    switch (state) {
        case QAbstractSocket::UnconnectedState: stateStr = "未连接"; break;
        case QAbstractSocket::ConnectingState:  stateStr = "连接中"; break;
        case QAbstractSocket::ConnectedState:   stateStr = "已连接"; break;
        case QAbstractSocket::ClosingState:     stateStr = "断开中"; break;
        default: stateStr = "未知状态";
    }
    appendMsg(QString("状态：%1").arg(stateStr));
}

// 新增：定时刷新连接的槽函数(以后可增加在切换界面时刷新界面)
void ClientWidget::onTimerRefresh()
{
    if (!m_isConnected) {
        appendMsg(QString("【定时刷新】%1秒未连接，自动执行重连...").arg(REFRESH_INTERVAL / 1000));
        connectToServer();
    }
}

// 消息显示辅助函数（带时间戳和错误标红，客户端可见，但客户端已隐藏）
void ClientWidget::appendMsg(const QString &msg, bool isError)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    if (isError) {
        ui->recvTextEdit->append(QString("[%1] <font color='red'>%2</font>").arg(time).arg(msg));
    } else {
        ui->recvTextEdit->append(QString("[%1] %2").arg(time).arg(msg));
    }
}
