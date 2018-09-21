#ifndef SENSOR_ADAPTER_H
#define SENSOR_ADAPTER_H

#include <memory>
#include <vector>
#include <tuple>

#include <QObject>
#include <QString>

#include "SensorController.h"
#include "Executor/ExecutorCommandList.h"

/**
 * \brief Adapter for adding sensors.
 */
class SensorAdapter : public QObject
{
    Q_OBJECT
public:

  /**
   * \brief                         Constructor of SensorAdapter
   * \param[in] sensorsDescription  Vector with description for intitialising sensors.
   * \param[in] parent              QObject paraent.
   */
  explicit SensorAdapter(
    const std::vector<std::tuple<QString, int, int, QString>>& sensorsDescription,
    QObject *parent = nullptr);

  /**
   * \brief         Function for shecking if sensor is active.
   * \param[in] id  Id of sensor for checking.
   * \return        True if sensor is active, false otherwise.
   */
  bool isOpen(size_t id);

  /**
   * \brief             Function for sending current robot position to sensor.
   * \param[in] id      Id of sensor for sending.
   * \param[in] params  Postion of robot.
   */
  void sendCurPosition(int id, QVector<double> params);

protected:
    
  /**
   * \brief Vector with controllers of process controller.
   */
  std::vector<SensorController> _sensorsProcessControllers;

signals:

  /**
   * \brief             Signal for sending position to robot.
   * \param[in] params  First number is id of sensor for sending data, other is data for sending.
   */
  void signalSendPosition(QVector<double> params);

  /**
   * \brief              Signal for sending data from sensor to executor.
   * \param[in] command  Id of command.
   * \param[in] params   Vector with data.
   */
  void signalGenerateCommand(ExectorCommand command, QVector<double> params);
    
protected slots:

  /**
   * \brief           Slot for sending data to executor.
   * \param[in] id    Id of sensor.
   * \param[in] data  Data from sensor.
   */
  void slotToGetNewParametrs(int id, QVector<double> data);
};

//todo rewite initialise with using config

#endif // SENSOR_ADAPTER_H
