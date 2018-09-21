#include "SensorAdapter.h"

#include <tuple>

#include <QVector>
#include <QDebug>

SensorAdapter::SensorAdapter(const std::vector<std::tuple<QString, int, int, QString>>&
  sensorsDescription, QObject *parent) :
  QObject(parent)
{
  QString stringParams = "";
  for (auto& elem : sensorsDescription)
  {
    stringParams.push_back(QString("{ %1, %2, %3, %4 }").arg(std::get<0>(elem), 
      QString("%1").arg(std::get<1>(elem)), QString("%1").arg(std::get<2>(elem)),
      std::get<3>(elem)));
  }
  qInfo() << QString("Create SensorAdapter with params: [%1]").arg(stringParams);
  const auto startChrono = std::chrono::steady_clock::now();
  
  _sensorsProcessControllers.reserve(sensorsDescription.size());
  for (size_t i = 0; i < sensorsDescription.size(); ++i)
  {
    _sensorsProcessControllers.emplace_back(i, std::get<0>(sensorsDescription[i]),
      std::get<1>(sensorsDescription[i]), std::get<2>(sensorsDescription[i]),
      std::get<3>(sensorsDescription[i]));
    connect(&_sensorsProcessControllers[i], &SensorController::newData, this,
      &SensorAdapter::slotToGetNewParametrs);
    _sensorsProcessControllers[i].startProcess();
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed SensorAdapter creation: %1 ms").arg(durationChrono / 1000.0);
}

bool SensorAdapter::isOpen(size_t id)
{
  qInfo() << QString("Check sensor with params: [%1]").arg(id);
  const auto startChrono = std::chrono::steady_clock::now();

  bool result = false;

  if (id >= 0 && id < _sensorsProcessControllers.size())
    result = _sensorsProcessControllers[id].isOpen();

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed checking sensor: %1 ms").arg(durationChrono / 1000.0);
  
  return result;
}

void SensorAdapter::sendCurPosition(int id, QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("send data to sensor with params. Id: %1. Params: %2.").arg(
    QString("%1").arg(id), dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  emit _sensorsProcessControllers[id].signalToWriteParams(params);

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed sending data to sensor: %1 ms").arg(durationChrono / 1000.0);
}
//todo what is disconnect

void SensorAdapter::slotToGetNewParametrs(int id, QVector<double> data)
{
  QString dataString;
  for (auto &i : data)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("recieve data from sensor with params. Id: %1. Params: %2.").arg(
    QString("%1").arg(id), dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  data.push_front(id);
  emit signalGenerateCommand(ExectorCommand::RECV_FROM_SENSOR, data);

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed recieving data from sensor: %1 ms").arg(durationChrono / 1000.0);
}
