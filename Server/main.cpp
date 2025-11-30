#include "serverwidget.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 设置主线程不退出，即使窗口关闭（可选）
    a.setQuitOnLastWindowClosed(true);

    ServerWidget w;
    w.show();

    // 启动主线程事件循环，捕获所有异常
    try {
        return a.exec();
    } catch (...) {
        qCritical() << "主线程发生未知异常，服务器继续运行";
        return a.exec(); // 异常后重启事件循环
    }
}
