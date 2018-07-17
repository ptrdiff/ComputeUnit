#include "MultiThreadingWorker.h"

MultiThreadingWorker::MultiThreadingWorker(QObject *parent):
QObject(parent)
{}

void MultiThreadingWorker::slotToDoSomething(const std::function<void()>& func)
{
    func();
    emit signalWorkReady();
}
