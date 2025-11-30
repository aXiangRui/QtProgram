#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

// 数据库连接工具类（占位，后续实现）
class DatabaseHelper
{
public:
    static QSqlDatabase getConnection()
    {
        // 【数据库连接位置】
        // 后续需实现：
        // 1. 初始化数据库驱动（QSQLITE/MYSQL/POSTGRESQL）
        // 2. 设置数据库路径/地址、用户名、密码
        // 3. 打开数据库并返回连接
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("./server.db");
        if (!db.open()) {
            qDebug() << "数据库连接失败：" << db.lastError().text();
        }
        return db;
    }
};

#endif // DATABASEHELPER_H
