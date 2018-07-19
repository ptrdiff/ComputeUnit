#ifndef WORKER_H
#define WORKER_H

#include <functional>

#include <QObject>

/**
 * \brief Class for executing function like void() in anouther QThread.
 */
class MultiThreadingWorker : public QObject
{
    Q_OBJECT

public:

    /**
     * \brief               Default Qt constructor.
     * \param[in] parent    Link to parent of this object.
     */
    MultiThreadingWorker(QObject *parent = nullptr);

signals:

    /**
     * \brief Signal, which emits after executing lyambda.
     */
    void signalWorkReady();

public slots:

    /**
     * \brief       Slot for executing passed lyambda.
     * \param func  Lyambda for executing.
     */
    void slotToDoSomething(const std::function<void()>& func);
};

#endif // WOKER_H