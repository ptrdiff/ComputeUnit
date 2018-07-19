#include "Executor.h"

#include <sstream>

#include <QCoreApplication>

Executor::Executor(std::string robotServerIP, const int robotServerPort,
                   std::string controlCenterIP, const int controlCenterPort, QObject *parent) try :
    QObject(parent),
    _wasFirstPoint(false),
    _lastSendPoint(),
    _controlCenterConnector(std::move(controlCenterIP), controlCenterPort),
    _robotConnector(std::move(robotServerIP), robotServerPort),
    _commandTable({
                      {"m", {&Executor::sendRobotMoveCommand, 7}},
                      {"a", {&Executor::sendControlCenterRobotPosition, 6}},
                      {"e", {&Executor::shutDownComputeUnit, 0}}
                  })
{
  qInfo() << QString("Create with parameters: Robot(IP: %1, Port: %2), Control Center(IP: %3, Port: %4).").arg
      (
          QString::fromStdString(robotServerIP),
          QString::number(robotServerPort),
          QString::fromStdString(controlCenterIP),
          QString::number(controlCenterPort)
      );
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


void Executor::slotToApplyCommand(const QString &id, const QVector<double> &params)
{
  QString dataString;
  for(auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start applying command. Id: %1, Parameters: %2").arg(id,dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_commandTable.count(id.toStdString()) == 0)
  {
    qCritical() << QString("Unknown command '%1' ").arg(id);
  } else
  {
    const auto curFunction = _commandTable[id.toStdString()];
    if (curFunction.second > params.size())
    {
      qCritical() << QString("Too less arguments for '%1' command (need minimum '%2', has '%3').").arg
          (
              id,
              QString::number(curFunction.second),
              QString::number(params.size())
          );
    } else
    {
      if (curFunction.second < params.size())
      {
        qWarning() << QString("Too much arguments for '%1' command (need minimum '%2', has '%3').").arg
            (
                id,
                QString::number(curFunction.second),
                QString::number(params.size())
            );
      }
      (this->*curFunction.first)(params);
    }
  }
  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Completed applying command: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::sendRobotMoveCommand(QVector<double> params)
{
  QString dataString;
  for(auto &i : params)
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
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Completed sending command to Robot: %1 ms").arg(durationChrono / 1000.0);

}

void Executor::sendControlCenterRobotPosition(QVector<double> params)
{
  QString dataString;
  for(auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start sending command to Control Center. Parameters: %1").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  emit signalWriteToControlCenter(params);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Completed sending command to Control Center: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::shutDownComputeUnit(QVector<double> params)
{
  QString dataString;
  for(auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start shutting down Compute Unit. Parameters: %1").arg(dataString);
  for(auto &i : params)
  {
    qInfo()<< QString("%1").arg(i);
  }
  const auto startChrono = std::chrono::steady_clock::now();

  emit signalWriteToRobot(QVector<double>{_lastSendPoint[0], _lastSendPoint[1], _lastSendPoint[2],
                                          _lastSendPoint[3], _lastSendPoint[4], _lastSendPoint[5], DEFAULT_SPEED, 1.0});
  QCoreApplication::exit(0);

  auto endChrono = std::chrono::steady_clock::now();
  auto durationChrono = std::chrono::duration_cast<std::chrono::microseconds>(endChrono-startChrono).count();
  qDebug() << QString("Completed shutting down Compute Unit: %1 ms").arg(durationChrono / 1000.0);
}
