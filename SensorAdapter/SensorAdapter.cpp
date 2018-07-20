#include "SensorAdapter.h"
#include <iostream>
#include <QVector>

SensorAdapter::SensorAdapter(const std::vector<std::pair<QString, int>>& sensorsDescription,
    QObject *parent):
QObject(parent)
{
    _sensorsProcessControllers.reserve(sensorsDescription.size());
    for (size_t i = 0; i < sensorsDescription.size(); ++i)
    {
        _sensorsProcessControllers.emplace_back(i, sensorsDescription[i].first, sensorsDescription[i].second);
        connect(&_sensorsProcessControllers[i], &SensorController::newData, this, &SensorAdapter::slotToGetNewParametrs);
        _sensorsProcessControllers[i].startProcess();
    }
}

bool SensorAdapter::isOpen(size_t id)
{
    if(id >= 0 && id < _sensorsProcessControllers.size())
        return _sensorsProcessControllers[id].isOpen();
    return false;
}

void SensorAdapter::sendCurPosition(int id, QVector<double> params)
{
    connect(this, &SensorAdapter::signalSendPosition, &_sensorsProcessControllers[id], &SensorController::writeParemetrs);
    emit signalSendPosition(params);
    disconnect(this, &SensorAdapter::signalSendPosition, &_sensorsProcessControllers[id], &SensorController::writeParemetrs);
    //todo what is disconnect
}

void SensorAdapter::slotToGetNewParametrs(int id, QVector<double> data)
{
    data.push_front(id);
    emit signalGenerateCommand("s", data);
}
