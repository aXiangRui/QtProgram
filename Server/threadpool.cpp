#include "threadpool.h"

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
    m_pool = QThreadPool::globalInstance();
    m_pool->setMaxThreadCount(10);  // 设置最大线程数，可根据需求调整
}

ThreadPool& ThreadPool::getInstance()
{
    static ThreadPool instance;
    return instance;
}

void ThreadPool::addTask(Task *task)
{
    m_pool->start(task);
}
