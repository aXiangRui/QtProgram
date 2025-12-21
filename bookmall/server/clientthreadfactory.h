#ifndef CLIENTTHREADFACTORY_H
#define CLIENTTHREADFACTORY_H

#include <QObject>
#include <QtGlobal>
#include "tcpserver.h"

// 协议类型枚举：标识当前创建的线程对应的协议
enum class ProtocolType {
    TCP        // TCP大文件传输
};

/**
 * @brief 客户端线程工厂类：创建TCP协议的线程任务
 * @note 职责：根据协议类型和连接参数，创建对应的线程任务（TcpFileTask）
 *       解耦线程创建逻辑，便于后续扩展新协议或修改线程创建规则
 */
class ClientThreadFactory : public QObject
{
    Q_OBJECT
public:
    // 单例模式（全局唯一工厂实例）
    static ClientThreadFactory& getInstance();

    /**
     * @brief 创建线程任务
     * @param type 协议类型（TCP）
     * @param socketDescriptor TCP连接的套接字描述符
     * @return 对应的线程任务（Task子类实例），失败返回nullptr
     */
    Task* createThread(ProtocolType type, qintptr socketDescriptor = 0);

private:
    // 私有构造函数（单例模式禁止外部实例化）
    explicit ClientThreadFactory(QObject *parent = nullptr);
};

#endif // CLIENTTHREADFACTORY_H
