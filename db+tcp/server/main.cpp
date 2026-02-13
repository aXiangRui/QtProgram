#include <QCoreApplication>
#include "server_thread.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MyTcpServer server;
    if (!server.listen(QHostAddress::Any, 12345)) {
        qDebug() << "服务器启动失败";
        return 1;
    }

    qDebug() << "服务器正在监听端口 12345...";

    return a.exec();
}
