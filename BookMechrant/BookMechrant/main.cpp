#include <QApplication>
#include "Seller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("图书商家管理系统");
    app.setOrganizationName("BookSeller");
    app.setApplicationVersion("1.0.0");

    Seller window;
    window.setWindowTitle("图书商家管理系统 v1.0");
    window.resize(1200, 700);  // 直接设置尺寸
    window.show();

    return app.exec();
}
