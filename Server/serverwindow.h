#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include "tcpserver.h"
#include "httpserver.h"
#include "websocketserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void on_startTcpBtn_clicked();
    void on_startHttpBtn_clicked();
    void on_startWsBtn_clicked();
    void appendLog(const QString& log);  // 日志输出

private:
    Ui::ServerWindow *ui;
    TcpServer* m_tcpServer;
    HttpServer* m_httpServer;
    WebSocketServer* m_wsServer;
    bool m_tcpRunning = false;
    bool m_httpRunning = false;
    bool m_wsRunning = false;
    const int TCP_PORT = 8888;
    const int HTTP_PORT = 8080;
    const int WS_PORT = 9090;
};

#endif // SERVERWINDOW_H
