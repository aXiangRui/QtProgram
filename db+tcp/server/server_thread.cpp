#include "server_thread.h"

// ================= MyTcpServer 实现 =================

MyTcpServer::MyTcpServer(QObject *parent) : QTcpServer(parent)
{
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "新连接进入，Socket描述符:" << socketDescriptor << "，正在创建线程...";

    // 创建一个新线程来处理这个连接
    // 当线程结束时自动删除对象
    ClientThread *thread = new ClientThread(socketDescriptor, this);
    connect(thread, &ClientThread::finished, thread, &ClientThread::deleteLater);
    thread->start();
}

// ================= ClientThread 实现 =================

// 构造函数在主线程执行，不要在这里创建Socket或DB对象
ClientThread::ClientThread(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor) {
}

void ClientThread::run()
{
    // --- 以下代码都在子线程中执行 ---

    // 1. 创建 Socket
    m_socket = new QTcpSocket();
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_socket->error());
        return;
    }

    // 2. 连接 Socket 信号
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientThread::onReadyRead, Qt::DirectConnection);
    // 断开连接时，退出线程事件循环
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientThread::onDisconnected, Qt::DirectConnection);

    // 3. 建立独立的数据库连接 (核心部分)
    // 关键：给连接起一个唯一的名字！
    // 这里使用 "DB_Conn_" + socket描述符 作为连接名
    m_connectionName = QString("DB_Conn_%1").arg(m_socketDescriptor);

    {
        // 建立连接
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", m_connectionName);
        db.setHostName("127.0.0.1");
        db.setDatabaseName("test_db");
        db.setUserName("root");
        db.setPassword("123456");

        if (!db.open()) {
            qDebug() << "线程" << m_socketDescriptor << "数据库连接失败:" << db.lastError().text();
            m_socket->write("Server Error: DB Connection Failed\n");
            m_socket->disconnectFromHost();
            return;
        }
        qDebug() << "线程" << m_socketDescriptor << "数据库连接成功，连接名:" << m_connectionName;

        m_tableName = QString("table_client_%1").arg(m_socketDescriptor);
        QSqlQuery query(db);
        QString createSql = QString(
                    "CREATE TABLE IF NOT EXISTS %1 ("
                    "id INT PRIMARY KEY, "
                    "name VARCHAR(50), "
                    "age INT)"
        ).arg(m_tableName);

        if (!query.exec(createSql)) {
            qDebug() << "建表失败:" << query.lastError().text();
            m_socket->write("Server Error: Create Table Failed\n");
        } else {
            qDebug() << "客户端" << m_socketDescriptor << "专属表已就绪:" << m_tableName;
        }
    }
    // 注意：db 对象的作用域结束了，但连接(connection)依然存在于 Qt 内部管理列表中

    // 4. 开启线程事件循环（保持线程存活，等待网络信号）
    exec();

    // --- 线程结束后的清理工作 ---

    // 关闭 Socket
    if(m_socket->state() != QAbstractSocket::UnconnectedState){
        m_socket->disconnectFromHost();
    }
    delete m_socket; // 删除 Socket 对象

    // 移除数据库连接 (必须先关闭，且不能有 QSqlDatabase 对象引用它)
    QSqlDatabase::removeDatabase(m_connectionName);
    qDebug() << "线程" << m_socketDescriptor << "结束，资源已清理。";
}

void ClientThread::onReadyRead()
{
    // 获取当前线程的数据库连接
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    // 读取并追加到缓存
    m_buffer.append(m_socket->readAll());

    // 循环处理每一行指令
    while (m_buffer.contains('\n')) {
        int index = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(index);
        m_buffer.remove(0, index + 1); // 移除已处理的数据

        QString request = QString::fromUtf8(line).trimmed();
        if (request.isEmpty()) continue;

        qDebug() << "客户端" << m_socketDescriptor << "请求:" << request;

        QStringList parts = request.split("|");
        QString cmd = parts.value(0);
        QSqlQuery query(db);
        QString reply;

        if (cmd == "ADD" && parts.size() == 4) {
            // 格式: ADD|ID|Name|Age
            QString sql = QString("INSERT INTO %1 (id, name, age) VALUES (:id, :name, :age)").arg(m_tableName);

            query.prepare(sql);
            query.bindValue(":id", parts[1].toInt());
            query.bindValue(":name", parts[2]);
            query.bindValue(":age", parts[3].toInt());

            if (query.exec()) reply = "OK: Added to your private table\r\n";
            else reply = "ERROR: " + query.lastError().text() + "\r\n";
        }
        else if (cmd == "QUERY") {
            // 格式: QUERY
            QString sql = QString("SELECT * FROM %1").arg(m_tableName);

            if (query.exec(sql)) {
                reply = QString("Data from table [%1]:\r\n").arg(m_tableName);
                while (query.next()) {
                    reply += QString("ID:%1 Name:%2 Age:%3\r\n")
                            .arg(query.value(0).toString())
                            .arg(query.value(1).toString())
                            .arg(query.value(2).toString());
                }
            } else {
                reply = "ERROR: Query Failed\r\n";
            }
        }
        else if (cmd == "DELETE" && parts.size() == 2) {
            // 格式: DELETE|ID
            QString sql = QString("DELETE FROM %1 WHERE id = :id").arg(m_tableName);
            query.prepare(sql);
            query.bindValue(":id", parts[1].toInt());

            if (query.exec()) reply = "OK: Deleted\r\n";
            else reply = "ERROR: " + query.lastError().text() + "\r\n";
        }
        else if (cmd == "UPDATE" && parts.size() == 4) {
            // 格式: UPDATE|ID|Name|Age
            QString sql = QString("UPDATE %1 SET name = :name, age = :age WHERE id = :id").arg(m_tableName);
            query.prepare(sql);
            query.bindValue(":id", parts[1].toInt());
            query.bindValue(":name", parts[2]);
            query.bindValue(":age", parts[3].toInt());

            if (query.exec()) reply = "OK: Updated\r\n";
            else reply = "ERROR: " + query.lastError().text() + "\r\n";
        }
        else {
            reply = "UNKNOWN COMMAND\r\n";
        }

        m_socket->write(reply.toUtf8());
    }
}
void ClientThread::onDisconnected()
{
    qDebug() << "客户端" << m_socketDescriptor << "断开连接";

    // 如果希望客户端断开后，删除他的表：
        /*
        {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            QSqlQuery query(db);
            query.exec(QString("DROP TABLE %1").arg(m_tableName));
            qDebug() << "已删除临时表:" << m_tableName;
        }
        */

    quit(); // 退出 exec() 事件循环，从而执行 run() 后面的清理代码
}
