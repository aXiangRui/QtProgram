#include "clientwindow.h"
#include "ui_clientwindow.h"

// 客户端窗口构造函数：初始化UI和客户端实例
ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("多协议客户端 v1.0");

    // 初始化客户端
    m_tcpClient = new TcpClient(this);
    m_httpClient = new HttpClient(this);
    m_wsClient = new WebSocketClient(this);

//    // 初始化HTTP服务器地址（从UI读取IP和端口）
//    updateHttpServerUrl();

//    // 连接信号槽：IP或HTTP端口变化时更新地址
//    connect(ui->serverIpEdit, &QLineEdit::textChanged, this, &ClientWindow::updateHttpServerUrl);
//    connect(ui->httpPortEdit, &QLineEdit::textChanged, this, &ClientWindow::updateHttpServerUrl);

    // 连接日志信号（显示到UI）
    connect(m_tcpClient, &TcpClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_httpClient, &HttpClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_httpClient, &HttpClient::responseReceived, this, [=](const QString& data) {
        ui->httpResponseEdit->setText(data);  // 显示HTTP响应
    });
    connect(m_wsClient, &WebSocketClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_wsClient, &WebSocketClient::messageReceived, this, &ClientWindow::appendWsMsg);
}

// 析构函数：释放UI资源
ClientWindow::~ClientWindow()
{
    delete ui;
}

// 添加日志到日志窗口（带时间戳）
void ClientWindow::appendLog(const QString &log)
{
    ui->logEdit->append(QString("[%1] %2").arg(QTime::currentTime().toString()).arg(log));
}

// 添加WebSocket消息到消息窗口
void ClientWindow::appendWsMsg(const QString &msg)
{
    ui->wsMsgEdit->append(QString("服务器：%1").arg(msg));
}

// // 更新HTTP服务器地址
//void ClientWindow::updateHttpServerUrl()
//{
//    QString ip = ui->serverIpEdit->text();
//    int httpPort = ui->httpPortEdit->text().toInt();
//    // 检查端口有效性（1-65535）
//    if (httpPort <= 0 || httpPort > 65535) {
//        appendLog("HTTP端口无效（1-65535）");
//        return;
//    }
//    m_httpClient->setBaseUrl(ip, httpPort);  // 更新HTTP客户端地址
//    appendLog(QString("HTTP服务器地址已更新：http://%1:%2").arg(ip).arg(httpPort));
//}

// TCP相关槽函数
void ClientWindow::on_tcpConnectBtn_clicked()
{
    QString ip = ui->serverIpEdit->text();
    int port = ui->tcpPortEdit->text().toInt();
    m_tcpClient->connectToServer(ip, port);
}

// 选择文件按钮点击事件
void ClientWindow::on_selectFileBtn_clicked()
{
    // 打开文件选择对话框
    m_selectedFilePath = QFileDialog::getOpenFileName(this, "选择文件", "./", "所有文件 (*.*)");
    if (!m_selectedFilePath.isEmpty()) {
        ui->filePathEdit->setText(m_selectedFilePath);  // 显示选中的文件路径
        appendLog("选中文件：" + m_selectedFilePath);
    }
}

// 发送文件按钮点击事件
void ClientWindow::on_sendFileBtn_clicked()
{
    if (!m_selectedFilePath.isEmpty()) {
        m_tcpClient->sendFile(m_selectedFilePath);  // 发送选中的文件
    } else {
        appendLog("请先选择要发送的文件");
    }
}

// HTTP相关槽函数
// 图书浏览请求按钮点击事件
void ClientWindow::on_getBooksBtn_clicked()
{
    m_httpClient->getBooks();
}

// 用户管理请求按钮点击事件
void ClientWindow::on_getUserBtn_clicked()
{
    m_httpClient->getUserInfo();
}

// 购物车请求按钮点击事件
void ClientWindow::on_getCartBtn_clicked()
{
    m_httpClient->getCart();
}

// 订单请求按钮点击事件
void ClientWindow::on_getOrderBtn_clicked()
{
    m_httpClient->getOrders();
}

// 收藏请求按钮点击事件
void ClientWindow::on_getCollectBtn_clicked()
{
    m_httpClient->getCollects();
}

// WebSocket相关槽函数
void ClientWindow::on_wsConnectBtn_clicked()
{
    QString ip = ui->serverIpEdit->text();
    int port = ui->wsPortEdit->text().toInt();
    m_wsClient->connectToServer(ip, port);
}

// 发送WebSocket消息按钮点击事件
void ClientWindow::on_sendWsMsgBtn_clicked()
{
    QString msg = ui->wsMsgInput->text();
    if (!msg.isEmpty()) {
        m_wsClient->sendMessage(msg);  // 发送消息
        ui->wsMsgEdit->append(QString("我：%1").arg(msg));  // 显示自己发送的消息
        ui->wsMsgInput->clear();  // 清空输入框
    }
}
