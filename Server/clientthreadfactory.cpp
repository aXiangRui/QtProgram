#include "clientthreadfactory.h"

// 单例实例初始化
ClientThreadFactory& ClientThreadFactory::getInstance()
{
    static ClientThreadFactory instance;
    return instance;
}

// 私有构造函数：无额外初始化逻辑
ClientThreadFactory::ClientThreadFactory(QObject *parent) : QObject(parent)
{
}

// 核心方法：根据协议类型创建对应线程任务
Task* ClientThreadFactory::createThread(ProtocolType type, qintptr socketDescriptor, QWebSocket *webSocket)
{
    switch (type) {
    case ProtocolType::TCP:
        // TCP协议：需要套接字描述符，创建TcpFileTask
        if (socketDescriptor != 0) {
            qDebug() << "线程工厂：创建TCP文件传输线程";
            return new TcpFileTask(socketDescriptor);
        } else {
            qWarning() << "线程工厂：创建TCP线程失败，缺少套接字描述符";
            return nullptr;
        }

    case ProtocolType::HTTP:
        // HTTP协议：需要套接字描述符，创建HttpTask
        if (socketDescriptor != 0) {
            qDebug() << "线程工厂：创建HTTP业务线程";
            return new HttpTask(socketDescriptor);
        } else {
            qWarning() << "线程工厂：创建HTTP线程失败，缺少套接字描述符";
            return nullptr;
        }

    case ProtocolType::WebSocket:
        // WebSocket协议：需要WebSocket连接对象，创建WebSocketTask
        if (webSocket != nullptr) {
            qDebug() << "线程工厂：创建WebSocket实时通信线程";
            return new WebSocketTask(webSocket);
        } else {
            qWarning() << "线程工厂：创建WebSocket线程失败，缺少WebSocket连接对象";
            return nullptr;
        }

    default:
        qWarning() << "线程工厂：不支持的协议类型";
        return nullptr;
    }
}
