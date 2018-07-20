#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include "../MultiThreadingWorker/MultiThreadingWorker.h"

class SensorController : public QObject
{
    Q_OBJECT

public:

    SensorController(QString sensorProgramName, int numberOfElementsToRead,
        int numberOfElementsToSend = -1, QString directoryForProcess = "",
        QObject * parent = nullptr);

    ~SensorController();

signals:

    void newData(QVector<double>);

public slots:

    void writeParemetrs(QVector<double>);

private:

    std::unique_ptr<QProcess> _sensorProcess;

    MultiThreadingWorker _multiThreadingWorker;

    QThread _thread;

    QString _programName;

    QString _directoryForProcess;

    int _numberOfElementsToRead;

    int _numberOfElementsToSend;

    void startProcess();

    void killProcess();

signals:

    void signalToComputeInAnoutherThread(std::function<void()> func);

private slots:

    void newError(QProcess::ProcessError error);

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void processHaveStarted();

    void newErrorMessage();

    void newMessage();

};
#endif // SENSOR_CONTROLLER_H