#include "Executor.h"

#include <chrono>

#include <QDebug>
#include <QCoreApplication>

Executor::Executor(RCAConnector& controlCenterConnector, RobotConnector& robotConnector,
  SensorAdapter &sensorAdapter, timur::CVS& cvs, MathModule& mathModule, QObject *parent) try :
  QObject(parent),
  _wasFirstPoint(false),
  _lastSendPoint({ 0,0,0,0,-90,0 }),
  _controlCenterConnector(controlCenterConnector),
  _robotConnector(robotConnector),
  _sensorAdapter(sensorAdapter),
  _cvs(cvs),
  _mathModule(mathModule),
  _commandTable({
                 {ExectorCommand::SHUT_DOWN,        {&Executor::shutDownComputeUnit,           -1}},
                 {ExectorCommand::SEND_TO_ROBOT,    {&Executor::sendRobotMoveCommand,           7}},
                 {ExectorCommand::SEND_TO_RCA,      {&Executor::sendControlCenterRobotPosition, 6}},
                 {ExectorCommand::SEND_TO_SENSOR,   {&Executor::askSensor,                     -1}},
                 {ExectorCommand::RECV_FROM_SENSOR, {&Executor::newSensorData,                 -1}}
    })
{
  qInfo() << QString("Create Executor.");
  const auto startChrono = std::chrono::steady_clock::now();

  QObject::connect(this, &Executor::signalToConnect, &_controlCenterConnector,
    &RCAConnector::slotToConnect, Qt::ConnectionType::QueuedConnection);
  QObject::connect(this, &Executor::signalToConnect, &_robotConnector,
    &RobotConnector::slotToConnect, Qt::ConnectionType::QueuedConnection);

  QObject::connect(&_controlCenterConnector, &RCAConnector::signalSocketError, this,
    &Executor::slotToSocketError);
  QObject::connect(&_robotConnector, &RobotConnector::signalSocketError, this,
    &Executor::slotToSocketError);

  QObject::connect(&_controlCenterConnector, &RCAConnector::signalNextCommand, this,
    &Executor::slotToApplyCommand);
  QObject::connect(&_robotConnector, &RobotConnector::signalNextCommand, this,
    &Executor::slotToApplyCommand);

  QObject::connect(&_sensorAdapter, &SensorAdapter::signalGenerateCommand, this,
    &Executor::slotToApplyCommand);

  QObject::connect(this, &Executor::signalWriteToControlCenter, &_controlCenterConnector,
    &RCAConnector::slotWriteToServer);
  QObject::connect(this, &Executor::signalWriteToRobot, &_robotConnector,
    &RobotConnector::slotWriteToServer);

  emit signalToConnect();

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed the Executor creation: %1 ms").arg(durationChrono / 1000.0);
}
catch (std::exception &exp)
{
  qCritical() << exp.what();
}

void Executor::slotToApplyCommand(ExectorCommand command, QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start applying command. Id: %1, Parameters: %2").arg(
    QString::number(static_cast<int>(command)), dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_commandTable.count(command) == 0)
  {
    qCritical() << QString("Unknown command '%1' ").arg(QString::number(static_cast<int>(command)));
  }
  else
  {
    const auto curFunction = _commandTable[command];
    if (curFunction.second != -1 && curFunction.second > params.size())
    {
      qCritical() <<
        QString("Too less arguments for '%1' command (need minimum '%2', has '%3').").arg
        (
          QString::number(static_cast<int>(command)),
          QString::number(curFunction.second),
          QString::number(params.size())
        );
    }
    else
    {
      if (curFunction.second != -1 && curFunction.second < params.size())
      {
        qWarning() <<
          QString("Too much arguments for '%1' command (need minimum '%2', has '%3').").arg
          (
            QString::number(static_cast<int>(command)),
            QString::number(curFunction.second),
            QString::number(params.size())
          );
        params.resize(curFunction.second);
      }
      (this->*curFunction.first)(params);
    }
  }
  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed applying command: %1 ms").arg(durationChrono / 1000.0);
}
//TODO Add anouther log descroption.

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

  params = _mathModule.sendToRobotTransformation(params);

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

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
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

  params = _mathModule.sendToRCATransformation(params);

  emit signalWriteToControlCenter(std::move(params));

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() <<
    QString("Completed sending command to Control Center: %1 ms").arg(durationChrono / 1000.0);
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

  if (_robotConnector.isConnected())
  {
    QVector<double> message = _lastSendPoint;

    message.push_back(DEFAULT_SPEED);
    message.push_back(1);

    emit signalWriteToRobot(message);
  }

  if (params.empty())
    QCoreApplication::exit(0);
  else
    QCoreApplication::exit(params[0]);

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed shutting down Compute Unit: %1 ms").arg(durationChrono / 1000.0);
}

void Executor::slotToSocketError()
{
  shutDownComputeUnit({ -1 });
}

void Executor::newSensorData(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start send new sensor data. Parameters: %1").arg(dataString);
  const auto start = std::chrono::steady_clock::now();

  emit signalWriteToControlCenter(params);

  qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::steady_clock::now() - start).count() / 1000.;
}//todo change this function when rcaConnector would be able to process data from sensors

void Executor::askSensor(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start make request for sensors. Parameters: %1").arg(dataString);
  const auto start = std::chrono::steady_clock::now();

  for (auto elem : params)
  {
    if (_sensorAdapter.isOpen(elem))
    {
      _sensorAdapter.sendCurPosition(elem, _mathModule.sendToSensorTransformation(_lastSendPoint));
    }
  }

  qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::steady_clock::now() - start).count() / 1000.;
}

void Executor::getComputerVisionSystemData(QVector<double> params)
{
    QString dataString;
    for (auto &i : params)
    {
        dataString.push_back(QString("%1 ").arg(i));
    }
    qInfo() << QString("Start using CVS. Parameters: %1").arg(dataString);
    const auto start = std::chrono::steady_clock::now();

    const auto objectCameraPosition = _cvs.getMarkerPose();

    std::array<double, 6> lastSendPoint;

    for(int i=0;i<6;++i)
    {
        lastSendPoint[i] = _lastSendPoint[i];
    }

    try {
        const auto objectPositions = _mathModule.sendAfterSensorTransformation(lastSendPoint, objectCameraPosition);

        std::array<double, 7> object;

        foreach(object, objectPositions)
        {
            QVector<double> command(7);
            for (int i = 0; i < 7; ++i)
            {
                command[i] = object[i];
                emit sendControlCenterRobotPosition(command);
            }
        }
    }
    catch(std::exception& exp)
    {
        qCritical() << QString("Can't access camera");
    }

    qDebug() << "finish: " << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count() / 1000.;
}

//todo update system to use all functions from fanuc server
//todo add timer, which process askSensor every 10 seconds.
//todo add logging to new functions