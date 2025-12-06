#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QThreadPool>
#include <QRunnable>
#include <QObject>

// 通用任务基类，所有线程池任务需继承此类并实现run()方法
class Task : public QRunnable
{
public:
    explicit Task() = default;  // 默认构造函数
    ~Task() override = default; // 析构函数，override确保重写父类方法
};

// 线程池单例类，管理所有客户端请求的线程资源
class ThreadPool : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例（全局唯一）
    static ThreadPool& getInstance();
    // 向线程池添加任务（线程池自动调度执行）
    void addTask(Task* task);

private:
    // 私有构造函数（单例模式禁止外部实例化）
    explicit ThreadPool(QObject *parent = nullptr);
    QThreadPool* m_pool;  // Qt内置线程池对象
};

#endif // THREADPOOL_H
