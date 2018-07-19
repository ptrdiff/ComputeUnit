#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include "../MultiThreadingWorker/MultiThreadingWorker.h"

class SensorController : QObject
{
    Q_OBJECT

public:

    SensorController(QString sensorProgramName,QObject * parent = nullptr);

    ~SensorController();

private:

    std::unique_ptr<QProcess> _sensorProcess;

    MultiThreadingWorker _multiThreadingWorker;

    QThread _thread;

    QString _programName;

    QString _directoryForProcess;

    void initialiseProcess();

    void deinitialiseProcess();

signals:

    void signalToComputeInAnoutherThread(std::function<void()> func);
};
#endif // SENSOR_CONTROLLER_H