#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include "tcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

// 服务器主窗口：显示服务状态和运行日志
class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    // 启动/停止TCP服务
    void on_startTcpBtn_clicked();
    // 向日志窗口添加内容
    void appendLog(const QString& log);

    void onClientReadyRead();

private:
    Ui::ServerWindow *ui;  // UI界面对象
    TcpServer* m_tcpServer;  // TCP服务器实例
    bool m_tcpRunning = false;  // TCP服务运行状态
    // 对应端口
    const int TCP_PORT = 8888;
};

#endif // SERVERWINDOW_H
