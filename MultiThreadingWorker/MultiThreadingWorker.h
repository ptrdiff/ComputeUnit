#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include <functional>

class MultiThreadingWorker : public QObject
{
    Q_OBJECT

public:

    MultiThreadingWorker(QObject *parent = nullptr);

signals:

    void signalWorkReady();

public slots:

    void slotToDoSomething(std::function<void()> func);
};

#endif // WOKER_H