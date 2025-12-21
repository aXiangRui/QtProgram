#include "serverwindow.h"
#include "data.h"  // MySQL数据库支持
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "服务器启动 - 数据库模式";
    qDebug() << "========================================";
    
    // 连接到远程数据库
    // 数据库服务器: 49.232.145.193:3306 (MySQL默认端口)
    // TCP服务器端口: 8888 (客户端连接端口)
    if (!Database::getInstance().initConnection(
            "49.232.145.193",  // 数据库服务器IP
            3306,              // MySQL端口
            "test_db",         // 数据库名
            "root01",          // 用户名
            "123456")           // 密码
        ) {
        QMessageBox::warning(nullptr, "数据库提示", 
            "无法连接到MySQL数据库！\n\n"
            "服务器: 49.232.145.193:3306\n"
            "数据库: test_db\n"
            "用户: root01\n\n"
            "可能原因:\n"
            "1. MySQL驱动未安装 (libmysql.dll)\n"
            "2. 数据库服务器未运行\n"
            "3. 网络连接问题\n"
            "4. 用户名密码错误\n\n"
            "程序将使用内存模式运行。");
        qDebug() << "❌ 数据库连接失败，切换到内存模式";
    } else {
        qDebug() << "✅ 数据库连接成功！";
        qDebug() << "✅ 数据库服务器: 49.232.145.193:3306";
        qDebug() << "✅ 数据库名称: test_db";
        qDebug() << "✅ 请求日志功能已启用";
    }
    
    qDebug() << "========================================";
    
    ServerWindow w;
    w.show();
    return a.exec();
}
