#include "SensorAdapter.h"
#include <iostream>
#include <QVector>

SensorAdapter::SensorAdapter(size_t numberof_sensors, std::vector<std::pair<QString, int>> sensorsDescription,
    QObject *parent = nullptr):
QObject(parent)
{
    _sensorsProcessControllers.reserve(numberof_sensors);
    for (size_t i = 0; i < numberof_sensors; ++i)
    {
        _sensorsProcessControllers.emplace_back(SensorController(i, sensorsDescription[i].first, sensorsDescription[i].second));
        connect(&_sensorsProcessControllers[i], &SensorController::newData, this, &SensorAdapter::slotToGetNewParametrs);
    }
}

bool SensorAdapter::isOpen(int id)
{
    return _sensorsProcessControllers[id].isOpen();
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
