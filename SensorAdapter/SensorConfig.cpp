#include "SensorConfig.h"

SensorConfig::SensorConfig(QString sensorProgramName, QString sensorFolderName, int inputBlockSize,
  int outputBlockSize):
  _sensorProgramName(std::move(sensorProgramName)),
  _sensorFolderName(std::move(sensorFolderName)),
  _inputBlockSize(inputBlockSize),
  _outputBlockSize(outputBlockSize)
{
}

QString SensorConfig::toQString() const
{
  return
    QString("{ program name: %1, directory name: %2, input block size: %3, outputblock size: %4").
    arg(_sensorProgramName, _sensorFolderName, QString("%1").arg(_inputBlockSize),
      QString("%1").arg(_outputBlockSize));
}
