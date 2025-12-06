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



// 客户端主窗口：整合TCP、HTTP、WebSocket功能的UI界面

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

    void on_tcpConnectBtn_clicked();       // 连接TCP服务器
    void on_selectFileBtn_clicked();       // 选择文件
    void on_sendFileBtn_clicked();         // 发送文件
    // HTTP相关
    void on_getBooksBtn_clicked();         // 图书浏览请求
    void on_getUserBtn_clicked();          // 用户管理请求
    void on_getCartBtn_clicked();          // 购物车请求
    void on_getOrderBtn_clicked();         // 订单请求
    void on_getCollectBtn_clicked();       // 收藏请求
    // WebSocket相关
    void on_wsConnectBtn_clicked();        // 连接WebSocket服务器
    void on_sendWsMsgBtn_clicked();        // 发送WebSocket消息
    // 日志和消息显示
    void appendLog(const QString& log);    // 添加日志
    void appendWsMsg(const QString& msg);  // 添加WebSocket消息


private:

//   // 更新HTTP服务器地址（当IP或端口变化时）
//    void updateHttpServerUrl();

    Ui::ClientWindow *ui;  // UI界面对象
    TcpClient* m_tcpClient;  // TCP客户端实例
    HttpClient* m_httpClient;  // HTTP客户端实例
    WebSocketClient* m_wsClient;  // WebSocket客户端实例

    QString m_selectedFilePath;  // 选中的文件路径
};

#endif // CLIENTWINDOW_H
