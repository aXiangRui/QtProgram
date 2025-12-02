#include "serverwindow.h"
#include "ui_serverwindow.h"

// 服务器窗口构造函数：初始化UI和服务器实例
ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("多协议服务器 v1.0");

    // 初始化服务器实例
    m_tcpServer = new TcpServer(this);
    m_httpServer = new HttpServer(this);
    m_wsServer = new WebSocketServer("WebSocketServer", QWebSocketServer::NonSecureMode, this);
}

// 析构函数：释放UI资源
ServerWindow::~ServerWindow()
{
    delete ui;
}

// 向日志窗口添加内容（带时间戳）
void ServerWindow::appendLog(const QString &log)
{
    ui->logEdit->append(QString("[%1] %2").arg(QTime::currentTime().toString()).arg(log));
}

// TCP服务启动/停止按钮点击事件
void ServerWindow::on_startTcpBtn_clicked()
{
    if (!m_tcpRunning) {
        // 启动TCP服务：监听所有网卡的TCP_PORT端口
        if (m_tcpServer->listen(QHostAddress::Any, TCP_PORT)) {
            m_tcpRunning = true;
            ui->tcpStatusLabel->setText("TCP服务：运行中（端口8888）");
            ui->startTcpBtn->setText("停止TCP");
            appendLog("TCP文件传输服务启动成功");
        } else {
            appendLog("TCP服务启动失败：" + m_tcpServer->errorString());
        }
    } else {
        m_tcpServer->close();
        m_tcpRunning = false;
        ui->tcpStatusLabel->setText("TCP服务：已停止");
        ui->startTcpBtn->setText("启动TCP");
        appendLog("TCP文件传输服务停止");
    }
}

// HTTP服务启动/停止按钮点击事件
void ServerWindow::on_startHttpBtn_clicked()
{
    if (!m_httpRunning) {
        if (m_httpServer->listen(QHostAddress::Any, HTTP_PORT)) {
            m_httpRunning = true;
            ui->httpStatusLabel->setText("HTTP服务：运行中（端口8080）");
            ui->startHttpBtn->setText("停止HTTP");
            appendLog("HTTP业务服务启动成功");
        } else {
            appendLog("HTTP服务启动失败：" + m_httpServer->errorString());
        }
    } else {
        // 停止HTTP服务
        m_httpServer->close();
        m_httpRunning = false;
        ui->httpStatusLabel->setText("HTTP服务：已停止");
        ui->startHttpBtn->setText("启动HTTP");
        appendLog("HTTP业务服务停止");
    }
}

// WebSocket服务启动/停止按钮点击事件
void ServerWindow::on_startWsBtn_clicked()
{
    if (!m_wsRunning) {
        // 启动WebSocket服务：监听所有网卡的WS_PORT端口
        if (m_wsServer->listen(QHostAddress::Any, WS_PORT)) {
            m_wsRunning = true;
            ui->wsStatusLabel->setText("WebSocket服务：运行中（端口9090）");
            ui->startWsBtn->setText("停止WebSocket");
            appendLog("WebSocket实时通信服务启动成功");
        } else {
            appendLog("WebSocket服务启动失败：" + m_wsServer->errorString());
        }
    } else {
        // 停止WebSocket服务
        m_wsServer->close();
        m_wsRunning = false;
        ui->wsStatusLabel->setText("WebSocket服务：已停止");
        ui->startWsBtn->setText("启动WebSocket");
        appendLog("WebSocket实时通信服务停止");
    }
}
