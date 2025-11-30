#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QNetworkConfigurationManager>
#include <QTimer>  // 定时器头文件

// 服务器IP和端口（根据实际部署修改）
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
// 定时刷新周期（毫秒），5秒=5000ms
#define REFRESH_INTERVAL 5000

namespace Ui {
class ClientWidget;
}

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget() override;

private slots:
    // 原有槽函数
    void on_connectBtn_clicked();
    void on_sendBtn_clicked();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onConnected();
    void onDisconnected();
    void onStateChanged(QAbstractSocket::SocketState state);

    // 新增：定时刷新连接的槽函数
    void onTimerRefresh();

private:
    Ui::ClientWidget *ui;
    QTcpSocket *m_tcpSocket;          // TCP套接字
    QNetworkConfigurationManager *m_netManager; // 网络检测
    QTimer *m_refreshTimer;           // 定时刷新定时器
    bool m_isConnected;               // 连接状态标记
    QString m_clientId;               // 客户端ID

    // 工具函数
    bool checkNetworkAvailable();     // 检测网络是否可用
    void connectToServer();           // 通用连接函数
    void appendMsg(const QString &msg, bool isError = false); // 消息显示
};

#endif // CLIENTWIDGET_H
