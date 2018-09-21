#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include <memory>

/**
 * \brief Controller for work with process.
 */
class SensorController : public QObject
{
  Q_OBJECT

public:

  /**
   * \brief                             Constructor with creating sensor process.
   * \param[in] id                      Id of current sensor.
   * \param[in] sensorProgramName       Program name for launching.
   * \param[in] numberOfElementsToRead  Size of one input block.
   * \param[in] numberOfElementsToSend  Size of one ouput block.
   * \param[in] directoryForProcess     Directory for process.
   * \param[in] parent                  Qobject parent.
   */
  SensorController(int id, QString sensorProgramName, int numberOfElementsToRead,
    int numberOfElementsToSend = -1, QString directoryForProcess = "",
    QObject * parent = nullptr);

  /**
   * \brief           Deleted copy constructor.
   * \param[in] other Anouther instance.
   */
  SensorController(SensorController& other) = delete;

  /**
   * \brief            Deleted copy operator.
   * \param[in] other  Anouther instance.
   */
  SensorController& operator= (SensorController& other) = delete;

  /**
   * \brief            Move contructor.
   * \param[in] other  Anouther instance.
   */
  SensorController(SensorController&& other) noexcept;

  /**
   * \brief           Move operator.
   * \param[in] other Anouther instance.
   */
  SensorController& operator= (SensorController&& other) noexcept;

  /**
   * \brief Class destructor.
   */
  ~SensorController();

  /**
   * \brief  Function for checking if process is running.
   * \return True if sensor process is running, false otherwise.
   */
  bool isOpen();

  /**
   * \brief Function for starting current process.
   */
  void startProcess();

  /**
   * \brief Function for teminating current process.
   */
  void killProcess();

signals:

  /**
   * \brief             Signal for sending new data from sensor to executor.
   * \param[in] id      Id of current server.
   * \param[in] params  Data.
   */
  void newData(int id, QVector<double> params);

  /**
   * \brief             Signal for send data to sensor.
   * \param[in] params  Data for sending.
   */
  void signalToWriteParams(QVector<double> params);

public slots:

  /**
   * \brief             Slot for sending data to sensor.
   * \param[in] params  Data for sending.
   */
  void writeParemetrs(QVector<double> params);

private:

  /**
   * \brief Pointer to Qt class for wotking with proceses.
   */
  std::unique_ptr<QProcess> _sensorProcess;

  /**
   * \brief Name of program.
   */
  QString _programName;

  /**
   * \brief Directory name for process.
   */
  QString _directoryForProcess;

  /**
   * \brief Number of elements for receiving in one block.
   */
  int _numberOfElementsToRead;

  /**
   * \brief Size of sending block.
   */
  int _numberOfElementsToSend;

  /**
   * \brief Id of current sensor.
   */
  int _id;

  /**
   * \brief Flag if sensor process is running.
   */
  bool _isOpen;

  /**
   * \brief Flag is crashed process need to be restarted.
   */
  bool _needToRestart;

signals:

  /**
   * \brief           Signal for intialising Object in anouther thread.
   * \param[in] func  Lyambda with code for initialasing.
   */
  void signalToComputeInAnoutherThread(std::function<void()> func);

private slots:

  /**
   * \brief           Slot for processing error from sensor process.
   * \param[in] error Process code error.
   */
  void newError(QProcess::ProcessError error);

  /**
   * \brief                 Function for processing process exit.
   * \param[in] exitCode    Exit code of process.
   * \param[in] exitStatus  Status of exit.
   */
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

  /**
   * \brief Slot for processing starting of process.
   */
  void processHaveStarted();

  /**
   * \brief Slot for processing new error from stderr.
   */
  void newErrorMessage();

  /**
   * \brief Slot for processing new message from stdout.
   */
  void newMessage();

};
#endif // SENSOR_CONTROLLER_H