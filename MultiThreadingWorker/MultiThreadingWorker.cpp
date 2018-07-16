#include "MultiThreadingWorker.h"

MultiThreadingWorker::MultiThreadingWorker(QObject *parent):
QObject(parent)
{}

void MultiThreadingWorker::slotToDoSomething(std::function<void()> func)
{
    func();
    emit signalWorkReady();
}
