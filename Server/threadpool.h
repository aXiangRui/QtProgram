#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QThreadPool>
#include <QRunnable>
#include <QObject>

// 通用任务基类
class Task : public QRunnable
{
public:
    explicit Task() = default;
    ~Task() override = default;
};

// 线程池单例类
class ThreadPool : public QObject
{
    Q_OBJECT
public:
    static ThreadPool& getInstance();
    void addTask(Task* task);  // 添加任务到线程池

private:
    explicit ThreadPool(QObject *parent = nullptr);
    QThreadPool* m_pool;
};

#endif // THREADPOOL_H
