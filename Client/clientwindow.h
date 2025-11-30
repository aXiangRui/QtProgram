#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "tcpclient.h"
#include "httpclient.h"
#include "websocketclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWindow; }
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    // TCP相关
    void on_tcpConnectBtn_clicked();
    void on_selectFileBtn_clicked();
    void on_sendFileBtn_clicked();
    // HTTP相关
    void on_getBooksBtn_clicked();
    void on_getUserBtn_clicked();
    void on_getCartBtn_clicked();
    void on_getOrderBtn_clicked();
    void on_getCollectBtn_clicked();
    // WebSocket相关
    void on_wsConnectBtn_clicked();
    void on_sendWsMsgBtn_clicked();
    // 日志和消息
    void appendLog(const QString& log);
    void appendWsMsg(const QString& msg);

private:
    Ui::ClientWindow *ui;
    TcpClient* m_tcpClient;
    HttpClient* m_httpClient;
    WebSocketClient* m_wsClient;
    QString m_selectedFilePath;  // 选中的文件路径
};

#endif // CLIENTWINDOW_H
