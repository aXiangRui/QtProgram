#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDateTime>
#include <QMutex>
#include <QTimer>
#include <QDomDocument>   // XML文档操作
#include <QDomElement>    // XML元素
#include <QDomText>       // XML文本节点
#include <QTextStream>    // 文本流

// 全局互斥锁：保证XML文件写入线程安全
extern QMutex xmlMutex;

class ClientHandler : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ClientHandler(QTcpSocket *socket, QObject *parent = nullptr);
    ~ClientHandler() override;

protected:
    void run() override;

signals:
    void clientDisconnected();
    void logMessage(const QString &msg);
    void errorOccurred(const QString &err);
    void checkSocketState(); // 定时检查Socket状态的信号

private slots:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onDisconnected();
    void deleteSocketLater();
    void onCheckSocketState(); // 检查Socket状态的槽函数

private:
    QTcpSocket *m_socket;
    bool m_isRunning; // 普通bool变量
    bool m_isSocketConnected; // 普通bool变量
    QMutex m_stateMutex; // 保护bool变量的互斥锁
//    void saveMessageToXml(const QString &clientIp, const QString &message);
};

#endif // CLIENTHANDLER_H
