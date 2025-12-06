#include "threadpool.h"

// 线程池构造函数：初始化线程池并设置最大线程数
ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
    m_pool = QThreadPool::globalInstance();  // 获取Qt全局线程池实例
    m_pool->setMaxThreadCount(10);  // 设置最大线程数（可根据服务器性能调整）
}

// 单例实例获取：静态局部变量确保唯一实例
ThreadPool& ThreadPool::getInstance()
{
    static ThreadPool instance;
    return instance;
}

// 添加任务到线程池：线程池会自动分配线程执行任务的run()方法
void ThreadPool::addTask(Task *task)
{
    m_pool->start(task);  // 提交任务到线程池队列
}
