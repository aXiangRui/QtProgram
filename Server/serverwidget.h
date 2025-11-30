#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QThreadPool>
#include <QAtomicInt>
#include <QMutex>
#include <QList>
#include <QPointer>
#include <QApplication>
#include <QMessageBox>

namespace Ui {
class ServerWidget;
}

class ClientHandler;

class ServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerWidget(QWidget *parent = nullptr);
    ~ServerWidget() override;

private slots:
    void on_startBtn_clicked();
    void onNewConnection();
    void updateClientCount();
    void appendLog(const QString &msg);
    void appendError(const QString &err);

private:
    Ui::ServerWidget *ui;
    QTcpServer *m_tcpServer;
    QAtomicInt m_clientCount;
    QMutex m_socketMutex;
    QList<QPointer<QTcpSocket>> m_clientSockets;
    void stopServer();
};

// 全局异常捕获函数
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif // SERVERWIDGET_H
