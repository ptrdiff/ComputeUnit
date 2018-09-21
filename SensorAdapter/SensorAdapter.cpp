#include "SensorAdapter.h"
#include <iostream>
#include <tuple>
#include <QVector>

SensorAdapter::SensorAdapter(const std::vector<std::tuple<QString, int, int, QString>>& 
    sensorsDescription, QObject *parent):
QObject(parent)
{
    _sensorsProcessControllers.reserve(sensorsDescription.size());
    for (size_t i = 0; i < sensorsDescription.size(); ++i)
    {
        _sensorsProcessControllers.emplace_back(i, std::get<0>(sensorsDescription[i]), 
            std::get<1>(sensorsDescription[i]), std::get<2>(sensorsDescription[i]), 
            std::get<3>(sensorsDescription[i]));
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
    emit signalGenerateCommand(ExectorCommand::RECV_FROM_SENSOR, data);
}
