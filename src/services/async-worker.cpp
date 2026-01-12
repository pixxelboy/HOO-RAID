#include "async-worker.h"
#include <obs-module.h>
#include <QCoreApplication>

// AsyncTask implementation
AsyncTask::AsyncTask(WorkFunction work, ResultCallback onSuccess, ErrorCallback onError, QObject* parent)
    : QObject(parent)
    , m_work(std::move(work))
    , m_onSuccess(std::move(onSuccess))
    , m_onError(std::move(onError))
{
}

void AsyncTask::execute()
{
    try {
        QVariant result = m_work();
        emit completed(result);
        if (m_onSuccess) {
            // Invoke callback on main thread
            QMetaObject::invokeMethod(qApp, [this, result]() {
                m_onSuccess(result);
            }, Qt::QueuedConnection);
        }
    } catch (const std::exception& e) {
        QString error = QString::fromUtf8(e.what());
        emit failed(error);
        if (m_onError) {
            QMetaObject::invokeMethod(qApp, [this, error]() {
                m_onError(error);
            }, Qt::QueuedConnection);
        }
    }
}

// AsyncWorker implementation
AsyncWorker::AsyncWorker(QObject* parent)
    : QThread(parent)
{
}

AsyncWorker::~AsyncWorker()
{
    stop();
    wait();
}

void AsyncWorker::enqueue(std::shared_ptr<AsyncTask> task)
{
    QMutexLocker locker(&m_mutex);
    m_taskQueue.enqueue(task);
    m_condition.wakeOne();
}

void AsyncWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
    m_condition.wakeAll();
}

void AsyncWorker::run()
{
    while (true) {
        std::shared_ptr<AsyncTask> task;

        {
            QMutexLocker locker(&m_mutex);
            while (m_running && m_taskQueue.isEmpty()) {
                m_condition.wait(&m_mutex);
            }

            if (!m_running && m_taskQueue.isEmpty()) {
                break;
            }

            task = m_taskQueue.dequeue();
        }

        if (task) {
            emit taskStarted();
            task->execute();
            emit taskCompleted();
        }
    }
}

// AsyncWorkerPool implementation
AsyncWorkerPool& AsyncWorkerPool::instance()
{
    static AsyncWorkerPool instance;
    return instance;
}

AsyncWorkerPool::AsyncWorkerPool(QObject* parent)
    : QObject(parent)
{
    for (int i = 0; i < WORKER_COUNT; ++i) {
        AsyncWorker* worker = new AsyncWorker(this);
        worker->start();
        m_workers.append(worker);
    }
    blog(LOG_DEBUG, "[HOO-Raid] Async worker pool initialized with %d workers", WORKER_COUNT);
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    shutdown();
}

void AsyncWorkerPool::execute(
    AsyncTask::WorkFunction work,
    AsyncTask::ResultCallback onSuccess,
    AsyncTask::ErrorCallback onError)
{
    auto task = std::make_shared<AsyncTask>(
        std::move(work),
        std::move(onSuccess),
        std::move(onError)
    );

    QMutexLocker locker(&m_mutex);
    AsyncWorker* worker = m_workers[m_nextWorker];
    m_nextWorker = (m_nextWorker + 1) % m_workers.size();
    worker->enqueue(task);
}

void AsyncWorkerPool::shutdown()
{
    blog(LOG_DEBUG, "[HOO-Raid] Shutting down async worker pool");
    for (AsyncWorker* worker : m_workers) {
        worker->stop();
    }
    for (AsyncWorker* worker : m_workers) {
        worker->wait();
    }
    qDeleteAll(m_workers);
    m_workers.clear();
}
