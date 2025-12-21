#include "serverwindow.h"
#include "ui_serverwindow.h"
#include "data.h"
#include <QTime>
#include <QTimer>

// 服务器窗口构造函数：初始化UI和服务器实例
ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("TCP服务器");
    // 初始化服务器实例
    m_tcpServer = new TcpServer(this);

    connect(m_tcpServer, &TcpServer::dataReceived, this, [this](const QString& ip, quint16 port, const QString& data) {
           appendLog(QString("收到来自 [%1:%2] 的数据：%3").arg(ip).arg(port).arg(data));
           // 在这里处理登录请求等业务逻辑
       });
       connect(m_tcpServer, &TcpServer::logGenerated, this, &ServerWindow::appendLog);
    
    // 窗口显示后，异步初始化示例图书数据（避免阻塞UI）
    QTimer::singleShot(500, this, [this]() {
        if (Database::getInstance().isConnected()) {
            appendLog("正在初始化示例图书数据...");
            bool success = Database::getInstance().initSampleBooks();
            if (success) {
                appendLog("示例图书数据初始化完成");
            } else {
                appendLog("示例图书数据初始化失败或已存在数据");
            }
        }
    });
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


// 处理客户端发送的数据
void ServerWindow::onClientReadyRead()
{
    // 获取发送数据的客户端Socket
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    // 读取数据
    QByteArray data = clientSocket->readAll();
    QString clientIp = clientSocket->peerAddress().toString();
    quint16 clientPort = clientSocket->peerPort();

    // 显示日志
    appendLog(QString("收到来自 [%1:%2] 的数据：%3")
              .arg(clientIp)
              .arg(clientPort)
              .arg(QString::fromUtf8(data)));

    // 可选：回复客户端
    clientSocket->write("服务器已接收数据");
}
