#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <QString>

/**
 * \brief Struct with parametrs for sensor.
 */
struct SensorConfig
{
  /**
   * \brief Name of sensor program.
   */
  QString _sensorProgramName;

  /**
   * \brief Path to directory where sensor process should be started.
   */
  QString _sensorFolderName;

  /**
   * \brief Size of input data block.
   */
  int _inputBlockSize;

  /**
   * \brief Size of output data block.
   */
  int _outputBlockSize;
  
  /**
   * \brief                         Constructor of sensor config.
   * \param[in] sensorProgramName   Name of sensor program
   * \param[in] sensorFolderName    Path to directory where sensor process should be started.
   * \param[in] inputBlockSize      Size of input data block.
   * \param[in] outputBlockSize     Size of output data block.
   */
  SensorConfig(QString sensorProgramName, QString sensorFolderName, int inputBlockSize,
    int outputBlockSize);

  /**
   * \brief   Function for creating QString with description of current sensor.
   * \return  QString with description.
   */
  QString toQString()const ;
};

#endif // SENSOR_CONFIG_H