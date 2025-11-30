#include "clientwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 仅启动客户端（可运行多个实例）
    ClientWidget client;
    client.show();
    client.hide();// 隐藏原客户端

    return a.exec();
}
