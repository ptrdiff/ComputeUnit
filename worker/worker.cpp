#include "worker.h"

Worker::Worker(QObject *parent):
QObject(parent)
{}

void Worker::slotToDoSomething(std::function<void()> func)
{
    func();
    emit signalWorkReady();
}
