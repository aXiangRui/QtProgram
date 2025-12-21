#include "bookmerchant.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序样式
    a.setStyle("Fusion");

    BookMerchant w;
    w.show();

    return a.exec();
}

