#include "Executor.h"

#include <sstream>

#include <QCoreApplication>


Executor::Executor(RCAConnector& controlCenterConnector, RobotConnector& robotConnector, QObject *parent) try :
QObject(parent),
_wasFirstPoint(false),
_lastSendPoint({0,0,0,0,-90,0}),
_controlCenterConnector(controlCenterConnector),
_robotConnector(robotConnector),
_sensorAdapter({ {"SensorAdapter/tmp/echo.exe",6} }),
_commandTable({ 
    { "m", {&Executor::sendRobotMoveCommand, 7} },
    { "a", {&Executor::sendControlCenterRobotPosition, 6} },
    { "e", {&Executor::shutDownComputeUnit, 0} },
    { "s", { &Executor::NewSensorData, -1 }},
    { "f", { &Executor::aksSensor, -1}}
})
{
  qInfo() << QString("Create with Executor.");
  const auto startChrono = std::chrono::steady_clock::now();

  QObject::connect(&_controlCenterConnector, &RCAConnector::signalNextCommand, this,
                   &Executor::slotToApplyCommand);
  QObject::connect(&_robotConnector, &RobotConnector::signalNextComand, this,
                   &Executor::slotToApplyCommand);

  QObject::connect(this, &Executor::signalWriteToControlCenter, &_controlCenterConnector,
                   &RCAConnector::slotWriteToServer);
  QObject::connect(this, &Executor::signalWriteToRobot, &_robotConnector,
                   &RobotConnector::slotWriteToServer);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed the creation: %1 ms").arg(durationChrono / 1000.0);
}
catch (std::exception &exp)
{
  qCritical() << exp.what();
}
// TODO Fix bug: 
//"Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*"


void Executor::slotToApplyCommand(const QString &id, QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start applying command. Id: %1, Parameters: %2").arg(id, dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_commandTable.count(id.toStdString()) == 0)
  {
    qCritical() << QString("Unknown command '%1' ").arg(id);
  } else
  {
    const auto curFunction = _commandTable[id.toStdString()];
    if (curFunction.second != -1 && curFunction.second > params.size())
    {
      qCritical() << QString("Too less arguments for '%1' command (need minimum '%2', has '%3').").arg
          (
              id,
              QString::number(curFunction.second),
              QString::number(params.size())
          );
    }
    else
    {
      if (curFunction.second != -1 && curFunction.second < params.size())
      {
        qWarning() << QString("Too much arguments for '%1' command (need minimum '%2', has '%3').").arg
            (
                id,
                QString::number(curFunction.second),
                QString::number(params.size())
            );
        params.resize(curFunction.second);
      }
      (this->*curFunction.first)(params);
    }
  }
  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed applying command: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::sendRobotMoveCommand(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start sending command to Robot. Parameters: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  double speed = DEFAULT_SPEED;

  if (_wasFirstPoint)
  {
    speed = 0.;

    for (size_t i = 0; i < 6; ++i)
    {
      speed += abs(_lastSendPoint[i] - params[i]);
    }

    speed = std::min(speed / TIME_FOR_RESPONSE, MAX_SPEED);
  }

  _wasFirstPoint = true;
  for (size_t i = 0; i < 6; ++i)
  {
    _lastSendPoint[i] = params[i];
  }

  params.push_back(speed);
  std::swap(params[6], params[7]);

  emit signalWriteToRobot(params);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed sending command to Robot: %1 ms").arg(durationChrono / 1000.0);

}

void Executor::sendControlCenterRobotPosition(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start sending command to Control Center. Parameters: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  emit signalWriteToControlCenter(params);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed sending command to Control Center: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::shutDownComputeUnit(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start shutting down Compute Unit. Parameters: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  QVector<double> message = _lastSendPoint;

  message.push_back(DEFAULT_SPEED);
  message.push_back(1);

  emit signalWriteToRobot(message);

  QCoreApplication::exit(0);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed shutting down Compute Unit: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::NewSensorData(QVector<double> params)
{
    QString dataString;
    for (auto &i : params)
    {
        dataString.push_back(QString("%1 ").arg(i));
    }
    qInfo() << QString("Start sned new sensor data. Parameters: %1").arg(dataString);
    const auto start = std::chrono::steady_clock::now();

    emit signalWriteToControlCenter(params);

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::aksSensor(QVector<double> params)
{
    QString dataString;
    for (auto &i : params)
    {
        dataString.push_back(QString("%1 ").arg(i));
    }
    qInfo() << QString("Start make request for sensors. Parameters: %1").arg(dataString);
    const auto start = std::chrono::steady_clock::now();

    for(auto elem : params)
    {
        if (_sensorAdapter.isOpen(elem))
        {
            _sensorAdapter.sendCurPosition(elem, _lastSendPoint);
        }
    }

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;

}


//todo add logging to new functions