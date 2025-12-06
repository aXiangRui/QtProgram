#include <QApplication>
#include <QDebug>
#include "include/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序样式
    a.setStyle("Fusion");
    
    // 创建并显示主窗口
    MainWindow w;
    w.show();
    
    return a.exec();
}