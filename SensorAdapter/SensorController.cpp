#include "SensorController.h"

#include <QDebug>

#include <sstream>

SensorController::SensorController(int id, SensorConfig config, QObject* parent) :
  QObject(parent),
  _sensorProcess(std::make_unique<QProcess>()),
  _programName(config._sensorProgramName),
  _directoryForProcess(config._sensorFolderName),
  _numberOfElementsToRead(config._inputBlockSize),
  _numberOfElementsToSend(config._outputBlockSize),
  _id(id),
  _isOpen(false),
  _needToRestart(true)
{
  qInfo() << QString("Create SensorController. "
      "Id: %1. ProgName: %2. ReadBlockSize: %3. SendBlockSize: %4. Directory: %5.").
  arg(QString::number(_id), _programName, QString::number(_numberOfElementsToRead),
    QString::number(_numberOfElementsToSend), _directoryForProcess);
  const auto startChrono = std::chrono::steady_clock::now();

  _sensorProcess->setProgram(_programName);

  if (!_directoryForProcess.isEmpty())
  {
    _sensorProcess->setWorkingDirectory(_directoryForProcess);
  }

  connect(_sensorProcess.get(), &QProcess::errorOccurred, this, &SensorController::newError);

  connect(_sensorProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    this, &SensorController::processFinished);

  connect(_sensorProcess.get(), &QProcess::started, this,
    &SensorController::processHaveStarted);

  connect(_sensorProcess.get(), &QProcess::readyReadStandardError, this,
    &SensorController::newErrorMessage);

  connect(_sensorProcess.get(), &QProcess::readyReadStandardOutput, this,
    &SensorController::newMessage);

  connect(this, &SensorController::signalToWriteParams, this, &SensorController::writeParemetrs);

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed SensorController creation: %1 ms").arg(durationChrono / 1000.0);
}
//todo rewrite with less signals-slots

SensorController::SensorController(SensorController&& other) noexcept
  :
  _sensorProcess(std::move(other._sensorProcess)),
  _programName(std::move(other._programName)),
  _directoryForProcess(std::move(other._directoryForProcess)),
  _numberOfElementsToRead(other._numberOfElementsToRead),
  _numberOfElementsToSend(other._numberOfElementsToSend),
  _id(other._id),
  _isOpen(other._isOpen)
{
}

SensorController& SensorController::operator=(SensorController&& other) noexcept
{
  SensorController tmp = std::move(other);
  std::swap(tmp, *this);
  return *this;
}

SensorController::~SensorController()
{
  qInfo() << QString("Destruct sensor controller with sensor %1").arg(_programName);
  const auto startChrono = std::chrono::steady_clock::now();

  _needToRestart = false;
  if (_sensorProcess->state() == QProcess::ProcessState::Running)
  {
    _sensorProcess->kill();
    _sensorProcess->waitForFinished();
  }
  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << 
    QString("Completed Destructing sensor controller: %1 ms").arg(durationChrono / 1000.0);
}

bool SensorController::isOpen()
{
  return _isOpen;
}

void SensorController::writeParemetrs(QVector<double> params)
{
  QString dataString;
  for (auto &i : params)
  {
    dataString.push_back(QString("%1 ").arg(i));
  }
  qInfo() << QString("Start writing to sensor. Parameters: %2").arg(dataString);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_numberOfElementsToSend != -1 && params.size() < _numberOfElementsToSend)
  {
    qCritical() << QString("Not enough elements for sending to process(need %1, has %2)").arg(
      _numberOfElementsToSend, params.size());
  }
  else
  {

    if (_numberOfElementsToSend != -1 && params.size() > _numberOfElementsToSend)
    {
      qWarning() << QString("too many elements for sending to robot(need %1, has %2)").arg(
        _numberOfElementsToSend, params.size());
    }

    std::stringstream s;
    for (auto& elem : params)
    {
      s << elem << ' ';

    }
    _sensorProcess->write(s.str().c_str());
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Completed writing to sensor: %1 ms").arg(durationChrono / 1000.0);
}
//todo rewrite it with textstream

void SensorController::startProcess()
{
  qInfo() << QString("Try starting sensor process %1.").arg(_programName);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_sensorProcess->state() == QProcess::ProcessState::NotRunning)
  {
    qDebug() << QString("process %1 is going to start").arg(_programName);
    _sensorProcess->start();
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << 
    QString("Completed trying starting sensor process: %1 ms").arg(durationChrono / 1000.0);
}

void SensorController::killProcess()
{
  qInfo() << QString("Try terminating sensor process %1.").arg(_programName);
  const auto startChrono = std::chrono::steady_clock::now();

  if (_sensorProcess->state() == QProcess::ProcessState::Running)
  {
    _sensorProcess->kill();
    qDebug() << QString("kill process %1").arg(_programName);
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() <<
    QString("Completed trying terminating sensor process: %1 ms").arg(durationChrono / 1000.0);
}

void SensorController::newError(QProcess::ProcessError error)
{
  switch (error)
  {
  case QProcess::ProcessError::FailedToStart:
  {
    qCritical() << QString("failed ot start process %1").arg(_programName);
    break;
  }
  case QProcess::ProcessError::Crashed:
  {
    if (_needToRestart) 
    {
      startProcess();
    }
    break;
  }
  case QProcess::ProcessError::Timedout:
  {
    qCritical() << QString("process %1 don't answer on command").arg(_programName);
    killProcess();
    startProcess();
    break;
  }
  default:
  {
    qCritical() << QString("unknown error %2 with process %1").arg(_programName, error);
    killProcess();
    break;
  }
  }
}

void SensorController::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  _isOpen = false;

  if (exitStatus == QProcess::ExitStatus::NormalExit)
  {
    if (exitCode == 0)
    {
      qInfo() << QString(" process %1 finished with code 0").arg(_programName);
    }
    else
    {
      qWarning() << QString(" process %1 finished with code %2").arg(_programName, exitCode);
    }
  }
  else
  {
    if (!_programName.isEmpty())
    {
      qCritical() << QString(" process %1 crashed").arg(_programName);
    }
    else
    {
      qCritical() << QString(" process crashed");
    }
  }
}

void SensorController::processHaveStarted()
{
  _isOpen = true;
  qInfo() << QString("process %1 have started").arg(_programName);
}

void SensorController::newErrorMessage()
{
  const auto errorData = _sensorProcess->readAllStandardError();
  qWarning() << QString("error from process %1: %2").arg(_programName, 
    static_cast<QString>(errorData));
}

void SensorController::newMessage()
{
  qInfo() << QString("Start reading from sensor %1.").arg(_programName);
  const auto startChrono = std::chrono::steady_clock::now();

  const auto tmpString = _sensorProcess->readAllStandardOutput();
  QTextStream stream(tmpString);

  QVector<double> resultData;
  resultData.reserve(_numberOfElementsToRead);

  while (!stream.atEnd()) 
  {
    for (int i = 0; i < _numberOfElementsToRead; ++i)
    {
      stream.skipWhiteSpace();
      if (stream.atEnd())
      {
        qCritical() << QString("Not enough parametrs from sensor process %1(need %2, has %3)").
          arg(_programName, _numberOfElementsToRead, i);
        return;
      }

      double coords;
      stream >> coords;

      resultData.push_back(coords);
    }
    emit newData(_id, resultData);
    stream.skipWhiteSpace();
  }

  const auto endChrono = std::chrono::steady_clock::now();
  const auto durationChrono =
    std::chrono::duration_cast<std::chrono::microseconds>(endChrono - startChrono).count();
  qDebug() << QString("Complete reading from sensor: %1 ms").arg(durationChrono / 1000.0);
}
//todo add more correct parser(may not work when has two or more packs of parametrs)

//todo adding more looging