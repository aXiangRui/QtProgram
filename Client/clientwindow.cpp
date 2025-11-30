#include "clientwindow.h"
#include "ui_clientwindow.h"

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

    // 连接信号槽
    connect(m_tcpClient, &TcpClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_httpClient, &HttpClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_httpClient, &HttpClient::responseReceived, this, [=](const QString& data) {
        ui->httpResponseEdit->setText(data);
    });
    connect(m_wsClient, &WebSocketClient::logMessage, this, &ClientWindow::appendLog);
    connect(m_wsClient, &WebSocketClient::messageReceived, this, &ClientWindow::appendWsMsg);
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

void ClientWindow::appendLog(const QString &log)
{
    ui->logEdit->append(QString("[%1] %2").arg(QTime::currentTime().toString()).arg(log));
}

void ClientWindow::appendWsMsg(const QString &msg)
{
    ui->wsMsgEdit->append(QString("服务器：%1").arg(msg));
}

// TCP相关槽函数
void ClientWindow::on_tcpConnectBtn_clicked()
{
    QString ip = ui->serverIpEdit->text();
    int port = ui->tcpPortEdit->text().toInt();
    m_tcpClient->connectToServer(ip, port);
}

void ClientWindow::on_selectFileBtn_clicked()
{
    m_selectedFilePath = QFileDialog::getOpenFileName(this, "选择文件", "./", "所有文件 (*.*)");
    if (!m_selectedFilePath.isEmpty()) {
        ui->filePathEdit->setText(m_selectedFilePath);
        appendLog("选中文件：" + m_selectedFilePath);
    }
}

void ClientWindow::on_sendFileBtn_clicked()
{
    if (!m_selectedFilePath.isEmpty()) {
        m_tcpClient->sendFile(m_selectedFilePath);
    } else {
        appendLog("请先选择要发送的文件");
    }
}

// HTTP相关槽函数
void ClientWindow::on_getBooksBtn_clicked()
{
    m_httpClient->getBooks();
}

void ClientWindow::on_getUserBtn_clicked()
{
    m_httpClient->getUserInfo();
}

void ClientWindow::on_getCartBtn_clicked()
{
    m_httpClient->getCart();
}

void ClientWindow::on_getOrderBtn_clicked()
{
    m_httpClient->getOrders();
}

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

void ClientWindow::on_sendWsMsgBtn_clicked()
{
    QString msg = ui->wsMsgInput->text();
    if (!msg.isEmpty()) {
        m_wsClient->sendMessage(msg);
        ui->wsMsgEdit->append(QString("我：%1").arg(msg));
        ui->wsMsgInput->clear();
    }
}
