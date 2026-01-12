#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <functional>
#include <memory>

// Task wrapper for async execution
class AsyncTask : public QObject {
    Q_OBJECT

public:
    using WorkFunction = std::function<QVariant()>;
    using ResultCallback = std::function<void(const QVariant&)>;
    using ErrorCallback = std::function<void(const QString&)>;

    AsyncTask(WorkFunction work, ResultCallback onSuccess, ErrorCallback onError, QObject* parent = nullptr);

    void execute();

signals:
    void completed(const QVariant& result);
    void failed(const QString& error);

private:
    WorkFunction m_work;
    ResultCallback m_onSuccess;
    ErrorCallback m_onError;
};

// Worker thread for executing async tasks
class AsyncWorker : public QThread {
    Q_OBJECT

public:
    explicit AsyncWorker(QObject* parent = nullptr);
    ~AsyncWorker() override;

    // Queue a task for execution
    void enqueue(std::shared_ptr<AsyncTask> task);

    // Stop the worker thread
    void stop();

signals:
    void taskStarted();
    void taskCompleted();
    void taskFailed(const QString& error);

protected:
    void run() override;

private:
    QQueue<std::shared_ptr<AsyncTask>> m_taskQueue;
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_running = true;
};

// Singleton worker pool manager
class AsyncWorkerPool : public QObject {
    Q_OBJECT

public:
    static AsyncWorkerPool& instance();

    // Execute work on a background thread, callbacks on main thread
    void execute(
        AsyncTask::WorkFunction work,
        AsyncTask::ResultCallback onSuccess = nullptr,
        AsyncTask::ErrorCallback onError = nullptr
    );

    // Shutdown all workers
    void shutdown();

private:
    explicit AsyncWorkerPool(QObject* parent = nullptr);
    ~AsyncWorkerPool() override;

    static constexpr int WORKER_COUNT = 2;
    QList<AsyncWorker*> m_workers;
    int m_nextWorker = 0;
    QMutex m_mutex;
};
